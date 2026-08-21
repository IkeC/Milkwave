#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <chrono>
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

using namespace winrt;
using namespace Windows::Media::Control;
using namespace std::chrono_literals;

extern float milkwave_amp_left;
extern float milkwave_amp_right;

class Milkwave {
 public:
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
  void LogInfo(const wchar_t* info);
  void LogInfo(std::wstring info);
  void LogDebug(std::wstring info);
  void LogDebug(const wchar_t* info);
  void LogException(const wchar_t* context, const std::exception& e, bool showMessage);
  void UpdateCurrentPosition(std::chrono::steady_clock::time_point currentTime);
  void PollMediaInfo();
  bool SaveThumbnailToFile(const winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionMediaProperties& properties);
};

extern Milkwave milkwave;