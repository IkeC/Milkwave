#include "milkwave.h"
#include <locale>
#include <codecvt>

Milkwave::Milkwave() {}

void Milkwave::Init(wchar_t* exePath) {
  winrt::init_apartment(); // Initialize the WinRT runtime
  start_time = std::chrono::steady_clock::now();

  // Get the executable's directory
  std::filesystem::path exeDir = std::filesystem::path(exePath).parent_path();

  // Construct the "resources/sprites/" directory path relative to the executable
  std::filesystem::path spritesDir = exeDir / "resources/sprites";
  std::filesystem::create_directories(spritesDir);

  // Construct the file path
  coverSpriteFilePath = spritesDir / "cover.png";
}

void Milkwave::PollMediaInfo() {

  if (!doPoll && !doPollExplicit) return;

  try {
    // Get the current time
    auto current_time = std::chrono::steady_clock::now();

    // Calculate the elapsed time in seconds
    auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count();

    // Check if 2 seconds have passed or manual poll requested
    if (elapsed_seconds >= 2 || doPollExplicit) {

      auto smtcManager = winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();
      auto currentSession = smtcManager.GetCurrentSession();
      updated = false;
      if (currentSession) {
        auto properties = currentSession.TryGetMediaPropertiesAsync().get();
        if (properties) {
          if (doPollExplicit || properties.Artist().c_str() != currentArtist || properties.Title().c_str() != currentTitle || properties.AlbumTitle().c_str() != currentAlbum) {
            isSongChange = currentAlbum.length() || currentArtist.length() || currentTitle.length();
            currentArtist = properties.Artist().c_str();
            currentTitle = properties.Title().c_str();
            currentAlbum = properties.AlbumTitle().c_str();

            if ((doPollExplicit || doSaveCover) && properties.Thumbnail()) {
              SaveThumbnailToFile(properties);
            }

            updated = true;
          }
        }
      }
      else {
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

void Milkwave::SaveThumbnailToFile(const winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionMediaProperties& properties) {
  try {
    // Retrieve the thumbnail
    auto thumbnailRef = properties.Thumbnail();
    if (!thumbnailRef) {
      std::wcerr << L"No thumbnail available for the current media." << std::endl;
      return;
    }

    // Open the thumbnail stream
    auto thumbnailStream = thumbnailRef.OpenReadAsync().get();
    auto decoder = winrt::Windows::Graphics::Imaging::BitmapDecoder::CreateAsync(thumbnailStream).get();

    // Encode the image as a PNG and save it to the file
    auto fileStream = winrt::Windows::Storage::Streams::InMemoryRandomAccessStream();
    auto encoder = winrt::Windows::Graphics::Imaging::BitmapEncoder::CreateAsync(
      winrt::Windows::Graphics::Imaging::BitmapEncoder::PngEncoderId(), fileStream).get();

    encoder.SetSoftwareBitmap(decoder.GetSoftwareBitmapAsync().get());
    encoder.FlushAsync().get();

    // Write the encoded image to the file
    std::ofstream outputFile(coverSpriteFilePath, std::ios::binary);
    if (!outputFile.is_open()) {
      std::wcerr << L"Failed to open file for writing: " << coverSpriteFilePath << std::endl;
      return;
    }

    // Use DataReader to read the stream content
    auto size = fileStream.Size();
    fileStream.Seek(0);
    auto buffer = winrt::Windows::Storage::Streams::Buffer(static_cast<uint32_t>(size));
    fileStream.ReadAsync(buffer, static_cast<uint32_t>(size), winrt::Windows::Storage::Streams::InputStreamOptions::None).get();

    outputFile.write(reinterpret_cast<const char*>(buffer.data()), buffer.Length());
    outputFile.close();

    std::wcout << L"Thumbnail saved to: " << coverSpriteFilePath.wstring() << std::endl;
  } catch (const std::exception& e) {
    LogException(L"SaveThumbnailToFile", e, false);
  }
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
  }
  else {
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

    logFile << "Exception occurred: " << utf8info << "\n" << exceptionMessage << std::endl;

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
  }
  else {
    std::cerr << "Failed to open log file: " << logFilePath.str() << std::endl;
  }

  if (showMessage) {
    // Show a message box with the error details
    std::wstring message = L"An unexpected error occurred:\n\n";
    message += std::wstring(exceptionMessage.begin(), exceptionMessage.end());
    message += L"\n\nDetails have been written to the log directory.\n\nDouble-click 'Window' in the Remote to restart Visualizer.";

    MessageBoxW(NULL, message.c_str(), L"Milkwave Error", MB_OK | MB_ICONERROR);
  }
}