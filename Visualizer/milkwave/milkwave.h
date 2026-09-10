#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <chrono>
#include <condition_variable>
#include <cstdint>

#include <windows.h>

#include <direct.h>
#include <string>
#include <dbghelp.h>

// Win RT
#include <winrt/Windows.Media.Control.h>
#include <winrt/Windows.Foundation.h>

#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Graphics.Imaging.h>
#include <filesystem>
#include <mutex>
#include <optional>
#include <thread>

#include "Lyrics.h"
#include "song_timeline.h"

using namespace winrt;
using namespace Windows::Media::Control;
using namespace std::chrono_literals;

extern float milkwave_amp_left;
extern float milkwave_amp_right;

class Milkwave {
 public:
  ~Milkwave();

  std::wstring currentArtist;
  std::wstring currentTitle;
  std::wstring currentAlbum;
  std::int64_t currentPositionMs = 0;
  std::int64_t currentDurationMs = 0;
  bool hasTimeline = false;
  bool timelineApproximate = false;
  bool isPlaying = false;

  std::chrono::steady_clock::time_point start_time;
  std::chrono::steady_clock::time_point timelineBaseTime;
  std::int64_t timelineBasePositionMs = 0;
  std::int64_t lastReportedPositionMs = 0;
  bool hasReportedPosition = false;
  bool hasPlaybackState = false;

  std::filesystem::path coverSpriteFilePath;

  bool updated = false;
  bool doPoll = false;
  bool doPollExplicit = false;
  bool isSongChange = false;
  bool doSaveCover = true;
  bool coverUpdated = false;
  int logLevel = 1;  // 0 = Off, 1 = Error, 2 = Info

  Milkwave();
  void Init(wchar_t* exePath);
  void SetLogDirectory(std::filesystem::path directory);
  void LogInfo(const wchar_t* info);
  void LogInfo(std::wstring info);
  void LogDebug(std::wstring info);
  void LogDebug(const wchar_t* info);
  void LogException(const wchar_t* context, const std::exception& e, bool showMessage);
  void UpdateCurrentPosition(std::chrono::steady_clock::time_point currentTime);
  void ResetTimeline();  // reset the internal timeline to 0 (restarts lyrics)
  void PollMediaInfo();
  void SetLyricsApiUrl(std::wstring apiUrl);
  void SetLyricsAutoLoad(bool enabled);
  void RequestLyricsNow();                                     // force a lyrics resolution for the current track
  bool LoadLyricsFromFile(const std::filesystem::path& path);  // load a specific lyrics file
  std::wstring CurrentLyricsFilePath() const;                  // current lyrics file, or empty
  std::wstring CurrentLyricText(std::int64_t offsetMs = 0) const;
  struct LyricsVisualState {
    std::wstring text;
    float opacity = 0.0f;
    std::int64_t startMs = -1;
  };
  LyricsVisualState CurrentLyricsVisualState(std::int64_t offsetMs, float fadeSeconds) const;
  std::wstring LyricsMonitorText(bool enabled, std::int64_t offsetMs = 0) const;
  bool SaveThumbnailToFile(const winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionMediaProperties& properties);

 private:
  bool EnsureMediaManager();
  void RequestLyricsResolution();
  void LyricsWorkerLoop();

  std::filesystem::path lyricsInstallDirectory;
  bool m_bLyricsAutoLoad = true;
  std::filesystem::path currentLyricsFile;  // file backing the loaded lyrics, if any
  mutable std::mutex lyricsMutex;
  std::condition_variable lyricsCondition;
  std::optional<LyricsTrackIdentity> pendingLyricsTrack;
  LyricsDocument lyricsDocument;
  std::uint64_t lyricsRequestGeneration = 0;
  std::wstring lyricsApiUrl = kDefaultLyricsApiUrl;
  std::filesystem::path logDirectory;
  mutable std::mutex logMutex;
  bool stopLyricsWorker = false;
  std::thread lyricsWorker;

  GlobalSystemMediaTransportControlsSessionManager smtcManager{nullptr};
  GlobalSystemMediaTransportControlsSession smtcSession{nullptr};
  std::chrono::steady_clock::time_point lastManagerAttempt;
  std::chrono::steady_clock::time_point lastSmtcPoll;
  std::chrono::steady_clock::time_point lastMetadataPoll;
  bool hasManagerAttempt = false;
  bool hasSmtcPoll = false;
  bool hasMetadataPoll = false;
  SongTimelineClock timelineClock;

  // Monotonic floor for the displayed lyric line: prevents brief "jump back"
  // flickers caused by SMTC timeline drift. Only large backward movements
  // (user seeks, track restart, Restart button) move the line backward.
  mutable std::int64_t m_lastShownLyricsPositionMs = 0;
  mutable bool m_hasShownLyricsPosition = false;
  std::int64_t EffectiveLyricsPosition(std::int64_t adjustedPositionMs) const;

  void WriteLog(const wchar_t* level, const std::wstring& message);
};

extern Milkwave milkwave;