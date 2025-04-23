
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

  bool updated = false;
  bool doPoll = false;
  bool doPollExplicit = false;
  bool isSongChange = false;
  bool doSaveCover = true;

  Milkwave();
  void Init();
  void LogException(const wchar_t* context, const std::exception& e);
  void PollMediaInfo();
  void SaveThumbnailToFile(const winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionMediaProperties& properties);
};