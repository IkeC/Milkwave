#include "milkwave.h"

Milkwave::Milkwave() {
}

void Milkwave::Init() {
  winrt::init_apartment(); // Initialize the WinRT runtime
  start_time = std::chrono::steady_clock::now();
}

void Milkwave::PollMediaInfo() {

  if (!doPoll && !doPollExplicit) return;
  
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

    // Get the executable's directory
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    std::filesystem::path exeDir = std::filesystem::path(exePath).parent_path();

    // Construct the "resources/sprites/" directory path relative to the executable
    std::filesystem::path spritesDir = exeDir / "resources/sprites";
    std::filesystem::create_directories(spritesDir);

    // Construct the file path
    std::filesystem::path filePath = spritesDir / "cover.png";

    // Encode the image as a PNG and save it to the file
    auto fileStream = winrt::Windows::Storage::Streams::InMemoryRandomAccessStream();
    auto encoder = winrt::Windows::Graphics::Imaging::BitmapEncoder::CreateAsync(
      winrt::Windows::Graphics::Imaging::BitmapEncoder::PngEncoderId(), fileStream).get();

    encoder.SetSoftwareBitmap(decoder.GetSoftwareBitmapAsync().get());
    encoder.FlushAsync().get();

    // Write the encoded image to the file
    std::ofstream outputFile(filePath, std::ios::binary);
    if (!outputFile.is_open()) {
      std::wcerr << L"Failed to open file for writing: " << filePath << std::endl;
      return;
    }

    // Use DataReader to read the stream content
    auto size = fileStream.Size();
    fileStream.Seek(0);
    auto buffer = winrt::Windows::Storage::Streams::Buffer(static_cast<uint32_t>(size));
    fileStream.ReadAsync(buffer, static_cast<uint32_t>(size), winrt::Windows::Storage::Streams::InputStreamOptions::None).get();

    outputFile.write(reinterpret_cast<const char*>(buffer.data()), buffer.Length());
    outputFile.close();

    std::wcout << L"Thumbnail saved to: " << filePath.wstring() << std::endl;
  } catch (const std::exception& e) {
    LogException(L"SaveThumbnailToFile", e);
  }
}


void Milkwave::LogException(const wchar_t* context, const std::exception& e) {
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
  logFilePath << logDir << "\\error." << timestamp << ".visualizer.log";

  // Write the exception details to the log file
  std::ofstream logFile(logFilePath.str());
  if (logFile.is_open()) {
    logFile << "Exception occurred: " << context << "\n" << exceptionMessage << std::endl;

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

  // Show a message box with the error details
  std::wstring message = L"An unexpected error occurred:\n\n";
  message += std::wstring(exceptionMessage.begin(), exceptionMessage.end());
  message += L"\n\nDetails have been written to the log directory.\n\nDouble-click 'Window' in the Remote to restart Visualizer.";

  MessageBoxW(NULL, message.c_str(), L"Milkwave Error", MB_OK | MB_ICONERROR);
}