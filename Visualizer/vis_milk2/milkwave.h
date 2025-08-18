#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>

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

  std::chrono::steady_clock::time_point start_time;
  
  std::filesystem::path coverSpriteFilePath;

  bool updated = false;
  bool doPoll = false;
  bool doPollExplicit = false;
  bool isSongChange = false;
  bool doSaveCover = true;
  int logLevel = 1; // 0 = Off, 1 = Error, 2 = Info

  Milkwave();
  void Init(wchar_t* exePath);
  void LogInfo(const wchar_t* info);
  void LogInfo(std::wstring info);
  void LogException(const wchar_t* context, const std::exception& e, bool showMessage);
  void PollMediaInfo();
  void SaveThumbnailToFile(const winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionMediaProperties& properties);
  
};