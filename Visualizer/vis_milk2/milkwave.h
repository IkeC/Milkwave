
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

using namespace winrt;
using namespace Windows::Media::Control;
// using namespace Windows::Foundation;
// using namespace Windows::Media::Playback;
using namespace std::chrono_literals;

extern float milkwave_amp_left;
extern float milkwave_amp_right;

class Milkwave {
  // MediaPlayer mediaPlayer;

public:
  std::wstring currentArtist;
  std::wstring currentTitle;
  std::wstring currentAlbum;
  bool updated = false;
  std::chrono::steady_clock::time_point start_time;

  Milkwave();
  void Init();
  void LogException(const wchar_t* context, const std::exception& e);
  void PollMediaInfo();
};