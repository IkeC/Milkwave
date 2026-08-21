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

  // Get the executable's directory
  std::filesystem::path exeDir = std::filesystem::path(exePath).parent_path();
  lyricsInstallDirectory = exeDir;

  // Construct the "resources/sprites/" directory path relative to the executable
  std::filesystem::path spritesDir = exeDir / "resources/sprites";
  std::filesystem::create_directories(spritesDir);
  std::filesystem::create_directories(LyricsDirectory(exeDir));
  lyricsWorker = std::thread(&Milkwave::LyricsWorkerLoop, this);

  // Construct the file path
  coverSpriteFilePath = spritesDir / "cover.png";
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

std::wstring Milkwave::CurrentLyricText() const {
  std::lock_guard<std::mutex> lock(lyricsMutex);
  const auto* line = lyricsDocument.CurrentLine(currentPositionMs);
  return line ? line->text : L"";
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
}

void Milkwave::LyricsWorkerLoop() {
  winrt::init_apartment();
  for (;;) {
    LyricsTrackIdentity track;
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
    }

    auto resolution = ResolveLyrics(lyricsInstallDirectory, track);

    std::lock_guard<std::mutex> lock(lyricsMutex);
    if (requestGeneration == lyricsRequestGeneration) lyricsDocument = std::move(resolution.document);
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
  LogInfo(info.c_str());
}

void Milkwave::LogDebug(const wchar_t* info) {
  if (logLevel < 3) return;
  LogInfo(info);
}

void Milkwave::LogInfo(std::wstring info) {
  LogInfo(info.c_str());
}

void Milkwave::LogInfo(const wchar_t* info) {
  if (logLevel < 2) return;

  // Ensure the "log" directory exists
  const char* logDir = "log";
  if (_mkdir(logDir) != 0 && errno != EEXIST) {
    std::cerr << "Failed to create or access log directory: " << logDir << std::endl;
    return;
  }

  // Get the current timestamp
  std::time_t now = std::time(nullptr);
  std::tm localTime;
  localtime_s(&localTime, &now);

  char datestring[20];
  char timestring[20];

  std::strftime(datestring, sizeof(datestring), "%Y-%m-%d", &localTime);
  std::strftime(timestring, sizeof(timestring), "%H:%M:%S", &localTime);

  // Construct the log file path
  std::ostringstream logFilePath;
  logFilePath << logDir << "\\" << datestring << ".visualizer.info.log";

  // Open the log file in append mode
  std::ofstream logFile(logFilePath.str(), std::ios::app);
  if (logFile.is_open()) {
    // Convert wchar_t* to UTF-8 std::string
    std::wstring ws(info);
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    std::string utf8info = conv.to_bytes(ws);

    logFile << timestring << "> " << utf8info << std::endl;
    logFile.close();
  } else {
    std::cerr << "Failed to open log file: " << logFilePath.str() << std::endl;
  }
}

void Milkwave::LogException(const wchar_t* context, const std::exception& e, bool showMessage) {
  if (logLevel < 1) return;

  std::wstring ws(context);
  std::wstring info = L"caught exception: ";
  info += ws;
  LogInfo(info.c_str());

  std::string exceptionMessage = e.what();

  // Ensure the "log" directory exists
  const char* logDir = "log";
  if (_mkdir(logDir) != 0 && errno != EEXIST) {
    std::cerr << "Failed to create or access log directory: " << logDir << std::endl;
    return;
  }

  // Get the current timestamp
  std::time_t now = std::time(nullptr);
  std::tm localTime;
  localtime_s(&localTime, &now);

  char timestamp[20];
  std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d_%H-%M-%S", &localTime);

  // Construct the log file path
  std::ostringstream logFilePath;
  logFilePath << logDir << "\\" << timestamp << ".visualizer.error.log";

  // Write the exception details to the log file
  std::ofstream logFile(logFilePath.str());
  if (logFile.is_open()) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    std::string utf8info = conv.to_bytes(ws);

    logFile << "Exception occurred: " << utf8info << "\n"
            << exceptionMessage << std::endl;

    // Capture and log the stack trace
    logFile << "\nStack trace:\n";
    HANDLE process = GetCurrentProcess();
    SymInitialize(process, NULL, TRUE);

    void* stack[64];
    USHORT frames = CaptureStackBackTrace(0, 64, stack, NULL);

    SYMBOL_INFO* symbol = (SYMBOL_INFO*)malloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char));
    if (symbol == NULL) {
      logFile << "Failed to allocate memory for SYMBOL_INFO." << std::endl;
      SymCleanup(process);
      return;
    }
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    for (USHORT i = 0; i < frames; i++) {
      SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);
      logFile << frames - i - 1 << ": " << symbol->Name << " - 0x" << std::hex << symbol->Address << std::dec << "\n";
    }

    free(symbol);
    SymCleanup(process);

    logFile.close();
  } else {
    std::cerr << "Failed to open log file: " << logFilePath.str() << std::endl;
  }

  if (showMessage) {
    // Show a message box with the error details
    std::wstring message = L"An unexpected error occurred:\n\n";
    message += std::wstring(exceptionMessage.begin(), exceptionMessage.end());
    message += L"\n\nDetails have been written to the log directory. Please open an issue on GitHub if the problem persists.\n\nPress Ctrl+O in the Remote to restart Visualizer.";

    MessageBoxW(NULL, message.c_str(), L"Milkwave Error", MB_OK | MB_ICONERROR);
  }
}