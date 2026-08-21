#include "milkwave.h"
#include "Lyrics.h"
#include <locale>
#include <codecvt>

namespace {
constexpr std::int64_t ASSUMED_TIMELINE_START_OFFSET_MS = 500;
}

Milkwave::Milkwave() {}

Milkwave::~Milkwave() {
  {
    std::lock_guard<std::mutex> lock(lyricsMutex);
    stopLyricsWorker = true;
  }
  lyricsCondition.notify_one();
  if (lyricsWorker.joinable()) lyricsWorker.join();
}

void Milkwave::Init(wchar_t* exePath) {
  winrt::init_apartment();  // Initialize the WinRT runtime
  start_time = std::chrono::steady_clock::now();
  timelineBaseTime = start_time;

  // Init receives the configured base directory, not the executable filename.
  std::filesystem::path exeDir = std::filesystem::path(exePath);
  if (!exeDir.empty()) logDirectory = exeDir / L"logs";
  lyricsInstallDirectory = exeDir;

  // Construct the "resources/sprites/" directory path relative to the executable
  std::filesystem::path spritesDir = exeDir / "resources/sprites";
  std::filesystem::create_directories(spritesDir);
  std::filesystem::create_directories(LyricsDirectory(exeDir));
  lyricsWorker = std::thread(&Milkwave::LyricsWorkerLoop, this);

  // Construct the file path
  coverSpriteFilePath = spritesDir / "cover.png";
}

void Milkwave::SetLogDirectory(std::filesystem::path directory) {
  if (!directory.empty()) logDirectory = std::move(directory);
}

void Milkwave::SetLyricsApiUrl(std::wstring apiUrl) {
  if (apiUrl.empty()) apiUrl = kDefaultLyricsApiUrl;
  std::lock_guard<std::mutex> lock(lyricsMutex);
  lyricsApiUrl = std::move(apiUrl);
}

void Milkwave::UpdateCurrentPosition(std::chrono::steady_clock::time_point currentTime) {
  if (!hasTimeline || !isPlaying) return;

  auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - timelineBaseTime).count();
  currentPositionMs = timelineBasePositionMs + elapsedMs;
  if (currentPositionMs < 0) currentPositionMs = 0;
  if (currentDurationMs > 0 && currentPositionMs > currentDurationMs) currentPositionMs = currentDurationMs;
}

void Milkwave::PollMediaInfo() {
  if (!doPoll && !doPollExplicit) return;

  try {
    // Get the current time
    auto current_time = std::chrono::steady_clock::now();
    UpdateCurrentPosition(current_time);

    // Calculate the elapsed time in seconds
    auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count();

    // Check if 1 second has passed or manual poll requested
    if (elapsed_seconds >= 1 || doPollExplicit) {
      auto smtcManager = winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();
      auto currentSession = smtcManager.GetCurrentSession();
      updated = false;
      if (currentSession) {
        auto timeline = currentSession.GetTimelineProperties();
        auto timelineDurationMs = std::chrono::duration_cast<std::chrono::milliseconds>(timeline.EndTime() - timeline.StartTime()).count();
        auto timelinePositionMs = std::chrono::duration_cast<std::chrono::milliseconds>(timeline.Position()).count();
        bool hasReportedTimeline = timelineDurationMs > 0 && timelinePositionMs >= 0;
        auto playbackStatus = currentSession.GetPlaybackInfo().PlaybackStatus();
        bool nextIsPlaying = playbackStatus == winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionPlaybackStatus::Playing;
        bool playbackStateChanged = !hasPlaybackState || nextIsPlaying != isPlaying;

        auto properties = currentSession.TryGetMediaPropertiesAsync().get();
        if (properties) {
          bool lyricsTrackChanged = properties.Artist().c_str() != currentArtist || properties.Title().c_str() != currentTitle || properties.AlbumTitle().c_str() != currentAlbum;
          bool trackChanged = doPollExplicit || properties.Artist().c_str() != currentArtist || properties.Title().c_str() != currentTitle || properties.AlbumTitle().c_str() != currentAlbum;
          if (trackChanged) {
            isSongChange = currentAlbum.length() || currentArtist.length() || currentTitle.length();
            currentArtist = properties.Artist().c_str();
            currentTitle = properties.Title().c_str();
            currentAlbum = properties.AlbumTitle().c_str();

            if ((doPollExplicit || doSaveCover) && properties.Thumbnail()) {
              SaveThumbnailToFile(properties);
            }

            updated = true;
          }

          if (trackChanged) {
            currentPositionMs = hasReportedTimeline ? std::max<std::int64_t>(0, timelinePositionMs) : ASSUMED_TIMELINE_START_OFFSET_MS;
            currentDurationMs = hasReportedTimeline ? std::max<std::int64_t>(0, timelineDurationMs) : 0;
            timelineBasePositionMs = currentPositionMs;
            timelineBaseTime = current_time;
            hasReportedPosition = hasReportedTimeline;
            lastReportedPositionMs = currentPositionMs;
            hasTimeline = hasReportedTimeline || !currentArtist.empty() || !currentTitle.empty();
            timelineApproximate = !hasReportedTimeline;
            if (lyricsTrackChanged) RequestLyricsResolution();
          } else if (hasReportedTimeline) {
            currentDurationMs = std::max<std::int64_t>(0, timelineDurationMs);
            if (!hasReportedPosition || timelinePositionMs != lastReportedPositionMs) {
              currentPositionMs = std::max<std::int64_t>(0, timelinePositionMs);
              timelineBasePositionMs = currentPositionMs;
              timelineBaseTime = current_time;
              lastReportedPositionMs = currentPositionMs;
              hasReportedPosition = true;
            }
            hasTimeline = true;
            timelineApproximate = false;
          }

          if (playbackStateChanged) {
            UpdateCurrentPosition(current_time);
            timelineBasePositionMs = currentPositionMs;
            timelineBaseTime = current_time;
          }

          if (!hasReportedTimeline && playbackStatus == winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionPlaybackStatus::Stopped) {
            currentPositionMs = 0;
            timelineBasePositionMs = 0;
            timelineBaseTime = current_time;
            lastReportedPositionMs = 0;
            hasReportedPosition = false;
            hasTimeline = !currentArtist.empty() || !currentTitle.empty();
            timelineApproximate = hasTimeline;
          }

          isPlaying = nextIsPlaying;
          hasPlaybackState = true;
          UpdateCurrentPosition(current_time);
        }
      } else {
        currentPositionMs = 0;
        currentDurationMs = 0;
        hasTimeline = false;
        timelineApproximate = false;
        isPlaying = false;
        hasPlaybackState = false;
        hasReportedPosition = false;
        {
          std::lock_guard<std::mutex> lock(lyricsMutex);
          ++lyricsRequestGeneration;
          pendingLyricsTrack.reset();
          lyricsDocument = LyricsDocument{};
        }
        timelineBasePositionMs = 0;
        timelineBaseTime = current_time;
        if (currentArtist.length() || currentTitle.length() || currentAlbum.length()) {
          currentArtist = L"";
          currentTitle = L"";
          currentAlbum = L"";
          updated = true;
        }
      }

      // Reset the start time to the current time
      start_time = current_time;
    }
  } catch (const std::exception& e) {
    LogException(L"PollMediaInfo", e, false);
  }
}

std::wstring Milkwave::CurrentLyricText(std::int64_t offsetMs) const {
  std::lock_guard<std::mutex> lock(lyricsMutex);
  const auto* line = lyricsDocument.CurrentLine(currentPositionMs + offsetMs);
  return line ? line->text : L"";
}

Milkwave::LyricsVisualState Milkwave::CurrentLyricsVisualState(std::int64_t offsetMs,
                                                               std::int64_t fadeDurationMs) const {
  std::lock_guard<std::mutex> lock(lyricsMutex);
  const auto adjustedPositionMs = currentPositionMs + offsetMs;
  const auto* currentLine = lyricsDocument.CurrentLine(adjustedPositionMs);
  if (!currentLine) {
    if (!lyricsDocument.plainText.empty()) return {lyricsDocument.plainText, 1.0f};
    return {};
  }

  float opacity = 1.0f;
  const auto duration = std::max<std::int64_t>(0, fadeDurationMs);
  if (duration > 0) {
    const auto sinceStart = adjustedPositionMs - currentLine->startMs;
    opacity = (std::min)(opacity, static_cast<float>(sinceStart) / static_cast<float>(duration));
    for (const auto& line : lyricsDocument.lines) {
      if (line.startMs > currentLine->startMs) {
        const auto untilNext = line.startMs - adjustedPositionMs;
        if (untilNext < duration)
          opacity = (std::min)(opacity, static_cast<float>(untilNext) / static_cast<float>(duration));
        break;
      }
    }
  }
  return {currentLine->text, std::clamp(opacity, 0.0f, 1.0f)};
}

std::wstring Milkwave::LyricsMonitorText(bool enabled, std::int64_t offsetMs) const {
  if (!enabled) return L"Lyrics off";
  std::lock_guard<std::mutex> lock(lyricsMutex);
  const auto* line = lyricsDocument.CurrentLine(currentPositionMs + offsetMs);
  if (line) return line->text;
  if (lyricsDocument.state == LyricsDocumentState::Loading) return L"Lyrics loading";
  if (lyricsDocument.state == LyricsDocumentState::Loaded) return L"Lyrics loaded";
  return L"Lyrics unavailable";
}

void Milkwave::RequestLyricsResolution() {
  LyricsTrackIdentity track{currentArtist, currentTitle, currentAlbum, currentDurationMs};
  std::lock_guard<std::mutex> lock(lyricsMutex);
  ++lyricsRequestGeneration;
  pendingLyricsTrack.reset();
  lyricsDocument = LyricsDocument{};
  if (track.artist.empty() && track.title.empty()) return;
  lyricsDocument.state = LyricsDocumentState::Loading;
  pendingLyricsTrack = std::move(track);
  lyricsCondition.notify_one();
  LogEvent(L"Lyrics loading: " + currentArtist + L" - " + currentTitle);
}

void Milkwave::LyricsWorkerLoop() {
  winrt::init_apartment();
  for (;;) {
    LyricsTrackIdentity track;
    std::wstring apiUrl;
    std::uint64_t requestGeneration = 0;
    {
      std::unique_lock<std::mutex> lock(lyricsMutex);
      lyricsCondition.wait(lock, [this] { return stopLyricsWorker || pendingLyricsTrack.has_value(); });
      if (stopLyricsWorker) {
        winrt::uninit_apartment();
        return;
      }
      track = *pendingLyricsTrack;
      pendingLyricsTrack.reset();
      requestGeneration = lyricsRequestGeneration;
      apiUrl = lyricsApiUrl;
    }

    auto resolution = ResolveLyrics(lyricsInstallDirectory, track, L"Milkwave/1.0", apiUrl);

    std::lock_guard<std::mutex> lock(lyricsMutex);
    if (requestGeneration == lyricsRequestGeneration) {
      const wchar_t* stateText = L"unknown";
      switch (resolution.document.state) {
        case LyricsDocumentState::Loaded: stateText = L"loaded"; break;
        case LyricsDocumentState::Instrumental: stateText = L"instrumental"; break;
        case LyricsDocumentState::NotFound: stateText = L"not found"; break;
        case LyricsDocumentState::Invalid: stateText = L"invalid"; break;
        case LyricsDocumentState::Loading: stateText = L"loading"; break;
        case LyricsDocumentState::Empty: stateText = L"empty"; break;
      }
      const auto lineCount = resolution.document.lines.size();
      const auto plainTextLength = resolution.document.plainText.size();
      const auto cachePath = resolution.cachePath;
      const auto cacheSaved = resolution.cacheSaved;
      const auto cacheExists = resolution.cacheExists;
      const auto cacheSaveError = resolution.cacheSaveError;
      lyricsDocument = std::move(resolution.document);
      const wchar_t* sourceText = resolution.source == LyricsSource::Local ? L"local" :
                                  resolution.source == LyricsSource::Lrclib ? L"lrclib" : L"none";
      std::wstring message = L"Lyrics result: " + track.artist + L" - " + track.title + L" state=" + stateText +
                             L" source=" + sourceText + L" lines=" + std::to_wstring(lineCount) +
                             L" plainChars=" + std::to_wstring(plainTextLength) +
                             L" http=" + std::to_wstring(resolution.httpStatus);
      if (!cachePath.empty()) {
        message += L" cache=" + cachePath.wstring() + L" cacheSaved=" + (cacheSaved ? L"true" : L"false") +
                   L" cacheExists=" + (cacheExists ? L"true" : L"false");
      }
      if (cacheSaveError != 0) message += L" cacheSaveError=" + std::to_wstring(cacheSaveError);
      if (!resolution.error.empty()) message += L" error=" + resolution.error;
      LogEvent(std::move(message));
    } else {
      LogEvent(L"Lyrics result discarded for stale track: " + track.artist + L" - " + track.title);
    }
  }
}

bool Milkwave::SaveThumbnailToFile(const winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionMediaProperties& properties) {
  try {
    // Retrieve the thumbnail
    auto thumbnailRef = properties.Thumbnail();
    if (!thumbnailRef) {
      std::wcerr << L"No thumbnail available for the current media." << std::endl;
      return false;
    }

    // Open the thumbnail stream
    auto thumbnailStream = thumbnailRef.OpenReadAsync().get();
    auto decoder = winrt::Windows::Graphics::Imaging::BitmapDecoder::CreateAsync(thumbnailStream).get();

    // Encode the image as a PNG and save it to the file
    auto fileStream = winrt::Windows::Storage::Streams::InMemoryRandomAccessStream();
    auto encoder = winrt::Windows::Graphics::Imaging::BitmapEncoder::CreateAsync(
                       winrt::Windows::Graphics::Imaging::BitmapEncoder::PngEncoderId(), fileStream)
                       .get();

    encoder.SetSoftwareBitmap(decoder.GetSoftwareBitmapAsync().get());
    encoder.FlushAsync().get();

    // Write the encoded image to the file
    std::ofstream outputFile(coverSpriteFilePath, std::ios::binary);
    if (!outputFile.is_open()) {
      std::wcerr << L"Failed to open file for writing: " << coverSpriteFilePath << std::endl;
      return false;
    }

    // Use DataReader to read the stream content
    auto size = fileStream.Size();
    fileStream.Seek(0);
    auto buffer = winrt::Windows::Storage::Streams::Buffer(static_cast<uint32_t>(size));
    fileStream.ReadAsync(buffer, static_cast<uint32_t>(size), winrt::Windows::Storage::Streams::InputStreamOptions::None).get();

    outputFile.write(reinterpret_cast<const char*>(buffer.data()), buffer.Length());
    outputFile.close();

    std::wcout << L"Thumbnail saved to: " << coverSpriteFilePath.wstring() << std::endl;
    coverUpdated = true;
    return true;
  } catch (const std::exception& e) {
    LogException(L"SaveThumbnailToFile", e, false);
  }
  return false;
}

void Milkwave::LogDebug(std::wstring info) {
  if (logLevel < 3) return;
  WriteLog(L"DEBUG", info);
}

void Milkwave::LogDebug(const wchar_t* info) {
  if (logLevel < 3) return;
  WriteLog(L"DEBUG", info ? std::wstring(info) : L"");
}

void Milkwave::LogInfo(std::wstring info) {
  LogInfo(info.c_str());
}

void Milkwave::LogInfo(const wchar_t* info) {
  if (logLevel < 2) return;
  WriteLog(L"INFO", info ? std::wstring(info) : L"");
}

void Milkwave::LogEvent(std::wstring info) {
  if (logLevel < 1) return;
  WriteLog(L"EVENT", info);
}

void Milkwave::LogEvent(const wchar_t* info) {
  if (logLevel < 1) return;
  WriteLog(L"EVENT", info ? std::wstring(info) : L"");
}

void Milkwave::WriteLog(const wchar_t* level, const std::wstring& message) {
  try {
    std::lock_guard<std::mutex> lock(logMutex);
    std::filesystem::path directory = logDirectory;
    if (directory.empty()) directory = std::filesystem::current_path() / L"logs";
    std::filesystem::create_directories(directory);

    std::time_t now = std::time(nullptr);
    std::tm localTime{};
    localtime_s(&localTime, &now);
    wchar_t date[16] = {};
    wchar_t clockTime[16] = {};
    std::wcsftime(date, _countof(date), L"%Y-%m-%d", &localTime);
    std::wcsftime(clockTime, _countof(clockTime), L"%H:%M:%S", &localTime);

    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::ofstream logFile(directory / (std::wstring(date) + L".visualizer.log"), std::ios::app);
    if (!logFile.is_open()) return;
    logFile << converter.to_bytes(clockTime) << " - [" << converter.to_bytes(level ? level : L"LOG") << "] "
            << converter.to_bytes(message) << std::endl;
  } catch (...) {
  }
}

void Milkwave::LogException(const wchar_t* context, const std::exception& e, bool showMessage) {
  if (logLevel < 1) return;

  std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
  std::wstring exceptionMessage;
  try {
    exceptionMessage = converter.from_bytes(e.what());
  } catch (...) {
    exceptionMessage = L"<exception text conversion failed>";
  }

  std::wstring logMessage = L"caught exception: " + std::wstring(context ? context : L"unknown") + L": " + exceptionMessage;
  logMessage += L"\nStack trace:";
  HANDLE process = GetCurrentProcess();
  SymInitialize(process, NULL, TRUE);
  void* stack[64];
  USHORT frames = CaptureStackBackTrace(0, 64, stack, NULL);
  SYMBOL_INFO* symbol = (SYMBOL_INFO*)malloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char));
  if (symbol != NULL) {
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    for (USHORT i = 0; i < frames; i++) {
      if (!SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol)) continue;
      logMessage += L"\n" + std::to_wstring(frames - i - 1) + L": ";
      try {
        logMessage += converter.from_bytes(symbol->Name);
      } catch (...) {
        logMessage += L"<unknown>";
      }
      wchar_t address[32] = {};
      swprintf_s(address, L" - 0x%llX", static_cast<unsigned long long>(symbol->Address));
      logMessage += address;
    }
    free(symbol);
  }
  SymCleanup(process);
  WriteLog(L"ERROR", logMessage);

  if (showMessage) {
    std::wstring message = L"An unexpected error occurred:\n\n";
    message += exceptionMessage;
    message += L"\n\nDetails have been written to the log directory. Please open an issue on GitHub if the problem persists.\n\nPress Ctrl+O in the Remote to restart Visualizer.";
    MessageBoxW(NULL, message.c_str(), L"Milkwave Error", MB_OK | MB_ICONERROR);
  }
}
