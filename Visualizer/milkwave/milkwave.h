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
  void PollMediaInfo();
  void SetLyricsApiUrl(std::wstring apiUrl);
  std::wstring CurrentLyricText(std::int64_t offsetMs = 0) const;
  struct LyricsVisualState {
    std::wstring text;
    float opacity = 0.0f;
  };
  LyricsVisualState CurrentLyricsVisualState(std::int64_t offsetMs, std::int64_t fadeDurationMs) const;
  std::wstring LyricsMonitorText(bool enabled, std::int64_t offsetMs = 0) const;
  bool SaveThumbnailToFile(const winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionMediaProperties& properties);

 private:
  void RequestLyricsResolution();
  void LyricsWorkerLoop();

  std::filesystem::path lyricsInstallDirectory;
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

  void WriteLog(const wchar_t* level, const std::wstring& message);
};

extern Milkwave milkwave;