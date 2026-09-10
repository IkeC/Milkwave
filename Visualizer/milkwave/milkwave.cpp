#include "milkwave.h"
#include "Lyrics.h"
#include <locale>
#include <codecvt>

namespace {

constexpr std::int64_t kTimelineBackwardJitterToleranceMs = 500;
constexpr std::int64_t ASSUMED_TIMELINE_START_OFFSET_MS = 500;
// How far backward the lyric position may move (SMTC timeline drift) before we
// treat it as a real seek and let the displayed line go back. Prevents the
// lyrics from briefly flickering back to a previous line.
constexpr std::int64_t kLyricsBackwardJitterToleranceMs = 1000;
}  // namespace

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
  timelineClock.Reset();

  // Init receives the configured base directory, not the executable filename.
  std::filesystem::path exeDir = std::filesystem::path(exePath);
  if (!exeDir.empty()) logDirectory = exeDir / L"log";
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

void Milkwave::SetLyricsAutoLoad(bool enabled) { m_bLyricsAutoLoad = enabled; }

void Milkwave::RequestLyricsNow() { RequestLyricsResolution(); }

bool Milkwave::LoadLyricsFromFile(const std::filesystem::path& path) {
  if (path.empty()) return false;
  auto document = ::LoadLyricsFile(path);
  if (document.state != LyricsDocumentState::Loaded) return false;

  std::lock_guard<std::mutex> lock(lyricsMutex);
  // Bump the generation so any in-flight auto-resolution for a stale track is
  // discarded and doesn't overwrite the manually loaded lyrics.
  ++lyricsRequestGeneration;
  pendingLyricsTrack.reset();
  lyricsDocument = std::move(document);
  currentLyricsFile = path;
  m_lastShownLyricsPositionMs = 0;
  m_hasShownLyricsPosition = false;
  LogInfo(L"Lyrics loaded from file: " + path.wstring());
  return true;
}

std::wstring Milkwave::CurrentLyricsFilePath() const {
  std::lock_guard<std::mutex> lock(lyricsMutex);
  return currentLyricsFile.wstring();
}

void Milkwave::SetLyricsApiUrl(std::wstring apiUrl) {
  if (apiUrl.empty()) apiUrl = kDefaultLyricsApiUrl;
  std::lock_guard<std::mutex> lock(lyricsMutex);
  lyricsApiUrl = std::move(apiUrl);
}

void Milkwave::UpdateCurrentPosition(std::chrono::steady_clock::time_point currentTime) {
  if (!hasTimeline || !isPlaying) return;

  if (timelineClock.IsInitialized()) {
    const auto steadyMs = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime.time_since_epoch()).count();
    currentPositionMs = timelineClock.Position(steadyMs);
    currentDurationMs = timelineClock.Duration();
    return;
  }

  auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - timelineBaseTime).count();
  currentPositionMs = timelineBasePositionMs + elapsedMs;
  if (currentPositionMs < 0) currentPositionMs = 0;
  if (currentDurationMs > 0 && currentPositionMs > currentDurationMs) currentPositionMs = currentDurationMs;
}

void Milkwave::ResetTimeline() {
  // Reset the internal timeline to 0 so the lyrics restart from the beginning.
  // This is mainly for players that don't properly report a timecode: the
  // position then counts up from zero using the internal clock. Players that
  // do report a timeline will re-sync on the next poll.
  currentPositionMs = 0;
  timelineBasePositionMs = 0;
  timelineBaseTime = std::chrono::steady_clock::now();
  timelineClock.Reset();
  lastReportedPositionMs = 0;
  hasReportedPosition = false;
  m_lastShownLyricsPositionMs = 0;
  m_hasShownLyricsPosition = false;
}

bool Milkwave::EnsureMediaManager() {
  if (smtcManager) return true;

  const auto now = std::chrono::steady_clock::now();
  if (hasManagerAttempt && now - lastManagerAttempt < 2s) return false;
  hasManagerAttempt = true;
  lastManagerAttempt = now;

  try {
    smtcManager = GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();
    return static_cast<bool>(smtcManager);
  } catch (const winrt::hresult_error& e) {
    LogInfo(L"EnsureMediaManager failed: " + std::wstring(e.message().c_str()));
    return false;
  }
}

void Milkwave::PollMediaInfo() {
  if (!doPoll && !doPollExplicit) return;

  try {
    auto current_time = std::chrono::steady_clock::now();
    UpdateCurrentPosition(current_time);

    if (!doPollExplicit && hasSmtcPoll && current_time - lastSmtcPoll < 1s) return;
    if (!EnsureMediaManager()) return;

    hasSmtcPoll = true;
    lastSmtcPoll = current_time;
    auto currentSession = smtcManager.GetCurrentSession();
    updated = false;
    if (currentSession) {
      const bool sessionChanged = !smtcSession || currentSession != smtcSession;
      smtcSession = currentSession;
      auto timeline = currentSession.GetTimelineProperties();
      const auto timelineStartMs = timeline.StartTime().count() / 10000;
      const auto timelineEndMs = timeline.EndTime().count() / 10000;
      const auto timelinePositionMs = timeline.Position().count() / 10000 - timelineStartMs;
      const auto timelineDurationMs = timelineEndMs - timelineStartMs;
      const bool hasReportedTimeline = timelineDurationMs > 0 && timelinePositionMs >= 0;
      auto playbackInfo = currentSession.GetPlaybackInfo();
      const bool nextIsPlaying = playbackInfo.PlaybackStatus() == GlobalSystemMediaTransportControlsSessionPlaybackStatus::Playing;
      const auto playbackRate = playbackInfo.PlaybackRate();
      const double nextPlaybackRate = playbackRate ? playbackRate.Value() : 1.0;
      const auto nowSteadyMs = std::chrono::duration_cast<std::chrono::milliseconds>(current_time.time_since_epoch()).count();
      const auto nowUtcMs = winrt::clock::now().time_since_epoch().count() / 10000;

      if (hasReportedTimeline) {
        const auto updatedUtcMs = timeline.LastUpdatedTime().time_since_epoch().count() / 10000;
        timelineClock.Update(timelinePositionMs, timelineDurationMs, updatedUtcMs > 0 ? updatedUtcMs : nowUtcMs,
                             nowUtcMs, nowSteadyMs, nextIsPlaying, nextPlaybackRate);
        currentPositionMs = timelineClock.Position(nowSteadyMs);
        currentDurationMs = timelineClock.Duration();
        timelineBasePositionMs = currentPositionMs;
        timelineBaseTime = current_time;
        lastReportedPositionMs = timelinePositionMs;
        hasReportedPosition = true;
        hasTimeline = true;
        timelineApproximate = false;
      } else if (sessionChanged) {
        timelineClock.Reset();
        currentPositionMs = ASSUMED_TIMELINE_START_OFFSET_MS;
        currentDurationMs = 0;
        timelineBasePositionMs = currentPositionMs;
        timelineBaseTime = current_time;
        hasReportedPosition = false;
        hasTimeline = true;
        timelineApproximate = true;
      }

      isPlaying = nextIsPlaying;
      hasPlaybackState = true;
      if (!hasReportedTimeline && playbackInfo.PlaybackStatus() == GlobalSystemMediaTransportControlsSessionPlaybackStatus::Stopped) {
        ResetTimeline();
        hasTimeline = !currentArtist.empty() || !currentTitle.empty();
        timelineApproximate = hasTimeline;
      } else {
        UpdateCurrentPosition(current_time);
      }

      const bool shouldPollMetadata = doPollExplicit || sessionChanged || !hasMetadataPoll || current_time - lastMetadataPoll >= 1s;
      if (shouldPollMetadata) {
        hasMetadataPoll = true;
        lastMetadataPoll = current_time;
        auto properties = currentSession.TryGetMediaPropertiesAsync().get();
        if (properties) {
          const bool metadataChanged = properties.Artist().c_str() != currentArtist || properties.Title().c_str() != currentTitle || properties.AlbumTitle().c_str() != currentAlbum;
          const bool trackChanged = sessionChanged || metadataChanged;
          if (trackChanged) {
            isSongChange = currentAlbum.length() || currentArtist.length() || currentTitle.length();
            currentArtist = properties.Artist().c_str();
            currentTitle = properties.Title().c_str();
            currentAlbum = properties.AlbumTitle().c_str();
            if (hasReportedTimeline) {
              timelineClock.Reset();
              timelineClock.Update(timelinePositionMs, timelineDurationMs,
                                   timeline.LastUpdatedTime().time_since_epoch().count() / 10000,
                                   nowUtcMs, nowSteadyMs, nextIsPlaying, nextPlaybackRate);
              currentPositionMs = timelineClock.Position(nowSteadyMs);
              currentDurationMs = timelineClock.Duration();
              timelineBasePositionMs = currentPositionMs;
              timelineBaseTime = current_time;
            } else {
              timelineClock.Reset();
              currentPositionMs = ASSUMED_TIMELINE_START_OFFSET_MS;
              currentDurationMs = 0;
              timelineBasePositionMs = currentPositionMs;
              timelineBaseTime = current_time;
              hasReportedPosition = false;
              hasTimeline = true;
              timelineApproximate = true;
            }
            updated = true;
          }
          if ((doPollExplicit || trackChanged || doSaveCover) && properties.Thumbnail()) SaveThumbnailToFile(properties);
          if ((trackChanged || doPollExplicit) && m_bLyricsAutoLoad) RequestLyricsResolution();
          if (doPollExplicit && !trackChanged) doPollExplicit = false;
        }
      }
    } else {
      smtcSession = nullptr;
      hasMetadataPoll = false;
      timelineClock.Reset();
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

    start_time = current_time;
  } catch (const winrt::hresult_error& e) {
    smtcManager = nullptr;
    smtcSession = nullptr;
    hasMetadataPoll = false;
    timelineClock.Reset();
    LogInfo(L"PollMediaInfo failed: " + std::wstring(e.message().c_str()));
  } catch (const std::exception& e) {
    LogException(L"PollMediaInfo", e, false);
  }
}

std::int64_t Milkwave::EffectiveLyricsPosition(std::int64_t adjustedPositionMs) const {
  if (!m_hasShownLyricsPosition) {
    m_lastShownLyricsPositionMs = adjustedPositionMs;
    m_hasShownLyricsPosition = true;
    return adjustedPositionMs;
  }
  const auto backwardMs = m_lastShownLyricsPositionMs - adjustedPositionMs;
  if (backwardMs > kLyricsBackwardJitterToleranceMs) {
    // Genuine backward movement (user seek, track restart, or the Restart
    // button) — follow it and reset the floor.
    m_lastShownLyricsPositionMs = adjustedPositionMs;
    return adjustedPositionMs;
  }
  if (backwardMs > 0) {
    // Small backward jitter (timeline/SMTC drift) — hold the forward line so
    // the lyrics never briefly flicker back to a previous line.
    return m_lastShownLyricsPositionMs;
  }
  // Forward (or equal) movement.
  m_lastShownLyricsPositionMs = adjustedPositionMs;
  return adjustedPositionMs;
}

std::wstring Milkwave::CurrentLyricText(std::int64_t offsetMs) const {
  std::lock_guard<std::mutex> lock(lyricsMutex);
  const auto effectivePositionMs = EffectiveLyricsPosition(currentPositionMs + offsetMs);
  const auto* line = lyricsDocument.CurrentLine(effectivePositionMs);
  return line ? line->text : L"";
}

Milkwave::LyricsVisualState Milkwave::CurrentLyricsVisualState(std::int64_t offsetMs,
                                                               float fadeSeconds) const {
  std::lock_guard<std::mutex> lock(lyricsMutex);
  if (lyricsDocument.lines.empty()) return {};
  const auto adjustedPositionMs = currentPositionMs + offsetMs;
  const auto effectivePositionMs = EffectiveLyricsPosition(adjustedPositionMs);
  const auto* currentLine = lyricsDocument.CurrentLine(effectivePositionMs);
  if (!currentLine) return {};

  float opacity = 1.0f;
  const auto duration = std::max<std::int64_t>(0, static_cast<std::int64_t>(fadeSeconds * 1000.0f));
  if (duration > 0) {
    const auto sinceStart = effectivePositionMs - currentLine->startMs;
    opacity = (std::min)(opacity, static_cast<float>(sinceStart) / static_cast<float>(duration));
    for (const auto& line : lyricsDocument.lines) {
      if (line.startMs > currentLine->startMs) {
        const auto untilNext = line.startMs - effectivePositionMs;
        if (untilNext < duration)
          opacity = (std::min)(opacity, static_cast<float>(untilNext) / static_cast<float>(duration));
        break;
      }
    }
  }
  return {currentLine->text, std::clamp(opacity, 0.0f, 1.0f), currentLine->startMs};
}

std::wstring Milkwave::LyricsMonitorText(bool enabled, std::int64_t offsetMs) const {
  // Status only — the current lyric line is exposed via CurrentLyricText().
  (void)offsetMs;
  if (!enabled) return L"Lyrics off";
  std::lock_guard<std::mutex> lock(lyricsMutex);
  if (lyricsDocument.state == LyricsDocumentState::Loading) return L"Lyrics loading";
  if (lyricsDocument.state == LyricsDocumentState::Loaded) {
    if (lyricsDocument.lines.empty() && !lyricsDocument.plainText.empty()) return L"Lyrics missing timestamps";
    return L"Lyrics loaded";
  }
  return L"Lyrics unavailable";
}

void Milkwave::RequestLyricsResolution() {
  LyricsTrackIdentity track{currentArtist, currentTitle, currentAlbum, currentDurationMs};
  std::lock_guard<std::mutex> lock(lyricsMutex);
  ++lyricsRequestGeneration;
  pendingLyricsTrack.reset();
  lyricsDocument = LyricsDocument{};
  currentLyricsFile.clear();
  m_lastShownLyricsPositionMs = 0;
  m_hasShownLyricsPosition = false;
  if (track.artist.empty() && track.title.empty()) return;
  lyricsDocument.state = LyricsDocumentState::Loading;
  pendingLyricsTrack = std::move(track);
  lyricsCondition.notify_one();
  LogDebug(L"Lyrics loading: " + currentArtist + L" - " + currentTitle);
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
        case LyricsDocumentState::Loaded:
          stateText = L"loaded";
          break;
        case LyricsDocumentState::Instrumental:
          stateText = L"instrumental";
          break;
        case LyricsDocumentState::NotFound:
          stateText = L"not found";
          break;
        case LyricsDocumentState::Invalid:
          stateText = L"invalid";
          break;
        case LyricsDocumentState::Loading:
          stateText = L"loading";
          break;
        case LyricsDocumentState::Empty:
          stateText = L"empty";
          break;
      }
      const auto lineCount = resolution.document.lines.size();
      const auto plainTextLength = resolution.document.plainText.size();
      const auto cachePath = resolution.cachePath;
      const auto cacheSaved = resolution.cacheSaved;
      const auto cacheExists = resolution.cacheExists;
      const auto cacheSaveError = resolution.cacheSaveError;
      // Remember the backing file when one actually exists on disk (a locally
      // found file, or an LRCLIB result that was cached).
      if (resolution.source == LyricsSource::Local || resolution.cacheExists) {
        currentLyricsFile = resolution.cachePath;
      } else {
        currentLyricsFile.clear();
      }
      lyricsDocument = std::move(resolution.document);
      m_lastShownLyricsPositionMs = 0;
      m_hasShownLyricsPosition = false;
      const wchar_t* sourceText = resolution.source == LyricsSource::Local ? L"local" : resolution.source == LyricsSource::Lrclib ? L"lrclib"
                                                                                                                                  : L"none";
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
      LogInfo(std::move(message));
    } else {
      LogInfo(L"Lyrics result discarded for stale track: " + track.artist + L" - " + track.title);
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

void Milkwave::WriteLog(const wchar_t* level, const std::wstring& message) {
  try {
    std::lock_guard<std::mutex> lock(logMutex);
    std::filesystem::path directory = logDirectory;
    if (directory.empty()) directory = std::filesystem::current_path() / L"log";
    std::filesystem::create_directories(directory);

    const auto systemNow = std::chrono::system_clock::now();
    const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(systemNow.time_since_epoch()) % 1000;
    const std::time_t now = std::chrono::system_clock::to_time_t(systemNow);
    std::tm localTime{};
    localtime_s(&localTime, &now);
    wchar_t date[16] = {};
    wchar_t clockTime[32] = {};
    std::wcsftime(date, _countof(date), L"%Y-%m-%d", &localTime);
    wchar_t timeWithoutMilliseconds[16] = {};
    std::wcsftime(timeWithoutMilliseconds, _countof(timeWithoutMilliseconds), L"%H:%M:%S", &localTime);
    swprintf_s(clockTime, L"%s.%03lld", timeWithoutMilliseconds, static_cast<long long>(milliseconds.count()));

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
