#include "milkwave.h"


Milkwave::Milkwave() {
}

void Milkwave::Init() {
  winrt::init_apartment(); // Initialize the WinRT runtime
  start_time = std::chrono::steady_clock::now();
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

void Milkwave::PollMediaInfo() {

  if (!doPoll && !doPollOnce) return;
  
  // Get the current time
  auto current_time = std::chrono::steady_clock::now();

  // Calculate the elapsed time in seconds
  auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count();

  // Check if 2 seconds have passed
  if (elapsed_seconds >= 2) {

    auto smtcManager = winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();
    auto currentSession = smtcManager.GetCurrentSession();
    updated = false;
    if (currentSession) {
      auto properties = currentSession.TryGetMediaPropertiesAsync().get();
      if (properties) {
        if (doPollOnce  || properties.Artist().c_str() != currentArtist || properties.Title().c_str() != currentTitle || properties.AlbumTitle().c_str() != currentAlbum) {
          currentArtist = properties.Artist().c_str();
          currentTitle = properties.Title().c_str();
          currentAlbum = properties.AlbumTitle().c_str();
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

    if (doPollOnce) {
      doPollOnce = false;
    }
  }
}
