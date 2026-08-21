#include "Lyrics.h"

#include <algorithm>
#include <codecvt>
#include <cwctype>
#include <fstream>
#include <iomanip>
#include <locale>
#include <sstream>

#include <windows.h>
#include <winhttp.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Data.Json.h>

namespace {

struct Timestamp {
  std::int64_t milliseconds = 0;
  std::size_t end = 0;
};

std::wstring Utf8ToWide(std::string_view text) {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  return converter.from_bytes(text.data(), text.data() + text.size());
}

std::string WideToUtf8(std::wstring_view text) {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  return converter.to_bytes(text.data(), text.data() + text.size());
}

std::vector<std::wstring> SplitLines(const std::wstring& text) {
  std::vector<std::wstring> lines;
  std::size_t start = 0;
  while (start <= text.size()) {
    const std::size_t end = text.find(L'\n', start);
    std::wstring line = text.substr(start, end == std::wstring::npos ? end : end - start);
    if (!line.empty() && line.back() == L'\r') line.pop_back();
    lines.push_back(std::move(line));
    if (end == std::wstring::npos) break;
    start = end + 1;
  }
  return lines;
}

bool ParseUnsigned(std::wstring_view text, std::size_t& index, std::int64_t& value) {
  if (index >= text.size() || !std::iswdigit(text[index])) return false;
  value = 0;
  while (index < text.size() && std::iswdigit(text[index])) {
    value = value * 10 + (text[index] - L'0');
    ++index;
  }
  return true;
}

std::optional<Timestamp> ParseTimestamp(std::wstring_view text, std::size_t start) {
  if (start >= text.size() || text[start] != L'[') return std::nullopt;

  std::size_t index = start + 1;
  std::int64_t minutes = 0;
  std::int64_t seconds = 0;
  if (!ParseUnsigned(text, index, minutes) || index >= text.size() || text[index] != L':') return std::nullopt;
  ++index;
  if (!ParseUnsigned(text, index, seconds) || seconds >= 60) return std::nullopt;

  std::int64_t fractionMs = 0;
  if (index < text.size() && (text[index] == L'.' || text[index] == L':')) {
    ++index;
    const std::size_t fractionStart = index;
    std::int64_t fraction = 0;
    if (!ParseUnsigned(text, index, fraction) || index - fractionStart > 3) return std::nullopt;
    switch (index - fractionStart) {
      case 1: fractionMs = fraction * 100; break;
      case 2: fractionMs = fraction * 10; break;
      case 3: fractionMs = fraction; break;
      default: return std::nullopt;
    }
  }

  if (index >= text.size() || text[index] != L']') return std::nullopt;
  return Timestamp{(minutes * 60 + seconds) * 1000 + fractionMs, index + 1};
}

std::wstring Trim(std::wstring_view text) {
  const auto first = text.find_first_not_of(L" \t");
  if (first == std::wstring_view::npos) return {};
  const auto last = text.find_last_not_of(L" \t");
  return std::wstring(text.substr(first, last - first + 1));
}

std::optional<std::int64_t> ParseMilliseconds(std::wstring_view value) {
  try {
    std::size_t consumed = 0;
    const auto parsed = std::stoll(std::wstring(value), &consumed);
    if (consumed != value.size()) return std::nullopt;
    return parsed;
  } catch (...) {
    return std::nullopt;
  }
}

std::optional<std::int64_t> ParseLengthMilliseconds(std::wstring_view value) {
  std::vector<std::int64_t> parts;
  std::size_t start = 0;
  while (start <= value.size()) {
    const auto end = value.find(L':', start);
    auto part = ParseMilliseconds(value.substr(start, end == std::wstring_view::npos ? end : end - start));
    if (!part) return std::nullopt;
    parts.push_back(*part);
    if (end == std::wstring_view::npos) break;
    start = end + 1;
  }
  if (parts.size() == 2 && parts[1] < 60) return (parts[0] * 60 + parts[1]) * 1000;
  if (parts.size() == 3 && parts[1] < 60 && parts[2] < 60) return (parts[0] * 3600 + parts[1] * 60 + parts[2]) * 1000;
  return std::nullopt;
}

void SetMetadataValue(LyricsMetadata& metadata, std::wstring_view key, std::wstring_view value) {
  const auto trimmed = Trim(value);
  if (key == L"ar") metadata.artist = trimmed;
  else if (key == L"al") metadata.album = trimmed;
  else if (key == L"ti") metadata.title = trimmed;
  else if (key == L"length") metadata.lengthMs = ParseLengthMilliseconds(trimmed);
  else if (key == L"offset") metadata.offsetMs = ParseMilliseconds(trimmed);
}

std::wstring SafeFilePart(std::wstring_view value) {
  std::wstring result;
  for (wchar_t character : value) {
    if (std::iswalnum(character) || character == L' ' || character == L'-' || character == L'_' || character == L'.') {
      result.push_back(static_cast<wchar_t>(std::towlower(character)));
    } else {
      result.push_back(L'_');
    }
  }
  while (!result.empty() && (result.back() == L' ' || result.back() == L'.')) result.pop_back();
  return result.empty() ? L"unknown" : result;
}

std::string UrlEncode(std::wstring_view value) {
  const auto utf8 = WideToUtf8(value);
  const char hex[] = "0123456789ABCDEF";
  std::string encoded;
  for (unsigned char character : utf8) {
    if ((character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z') ||
        (character >= '0' && character <= '9') || character == '-' || character == '_' ||
        character == '.' || character == '~') {
      encoded.push_back(static_cast<char>(character));
    } else {
      encoded.push_back('%');
      encoded.push_back(hex[character >> 4]);
      encoded.push_back(hex[character & 0x0f]);
    }
  }
  return encoded;
}

struct HttpResponse {
  int status = 0;
  std::string body;
  std::wstring error;
};

HttpResponse GetLrclib(std::wstring_view apiUrl, std::wstring_view path, std::wstring_view userAgent) {
  HttpResponse response;
  const std::wstring apiUrlString(apiUrl.empty() ? kDefaultLyricsApiUrl : apiUrl);
  const std::wstring pathString(path);
  const std::wstring userAgentString(userAgent);
  wchar_t hostName[256] = {};
  wchar_t basePath[2048] = {};
  URL_COMPONENTS urlComponents = {};
  urlComponents.dwStructSize = sizeof(urlComponents);
  urlComponents.lpszHostName = hostName;
  urlComponents.dwHostNameLength = _countof(hostName);
  urlComponents.lpszUrlPath = basePath;
  urlComponents.dwUrlPathLength = _countof(basePath);
  if (!WinHttpCrackUrl(apiUrlString.c_str(), static_cast<DWORD>(apiUrlString.length()), 0, &urlComponents)) {
    response.error = L"Invalid lyrics API URL: " + apiUrlString;
    return response;
  }
  if (urlComponents.nScheme != INTERNET_SCHEME_HTTP && urlComponents.nScheme != INTERNET_SCHEME_HTTPS) {
    response.error = L"Lyrics API URL must use HTTP or HTTPS";
    return response;
  }

  std::wstring requestPath(basePath, urlComponents.dwUrlPathLength);
  while (requestPath.length() > 1 && requestPath.back() == L'/') requestPath.pop_back();
  if (requestPath.empty()) requestPath = L"/";
  if (!pathString.empty() && pathString.front() != L'/') requestPath += L'/';
  requestPath += pathString;

  HINTERNET session = WinHttpOpen(userAgentString.c_str(), WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, nullptr, nullptr, 0);
  if (!session) {
    response.error = L"WinHttpOpen failed: " + std::to_wstring(GetLastError());
    return response;
  }
  WinHttpSetTimeouts(session, 5000, 5000, 10000, 10000);

  HINTERNET connection = WinHttpConnect(session, hostName, urlComponents.nPort, 0);
  HINTERNET request = connection
                          ? WinHttpOpenRequest(connection, L"GET", requestPath.c_str(), nullptr, WINHTTP_NO_REFERER,
                                               WINHTTP_DEFAULT_ACCEPT_TYPES,
                                               urlComponents.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0)
                          : nullptr;
  if (!connection) response.error = L"WinHttpConnect failed: " + std::to_wstring(GetLastError());
  if (connection && !request) response.error = L"WinHttpOpenRequest failed: " + std::to_wstring(GetLastError());
  if (request && !WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, nullptr, 0, 0, 0)) {
    response.error = L"WinHttpSendRequest failed: " + std::to_wstring(GetLastError());
  } else if (request && !WinHttpReceiveResponse(request, nullptr)) {
    response.error = L"WinHttpReceiveResponse failed: " + std::to_wstring(GetLastError());
  } else if (request) {
    DWORD status = 0;
    DWORD statusSize = sizeof(status);
    WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                        WINHTTP_HEADER_NAME_BY_INDEX, &status, &statusSize, WINHTTP_NO_HEADER_INDEX);
    response.status = static_cast<int>(status);
    DWORD available = 0;
    while (WinHttpQueryDataAvailable(request, &available) && available > 0) {
      std::string chunk(available, '\0');
      DWORD read = 0;
      if (!WinHttpReadData(request, chunk.data(), available, &read)) {
        response.error = L"WinHttpReadData failed: " + std::to_wstring(GetLastError());
        break;
      }
      response.body.append(chunk.data(), read);
    }
  }

  if (request) WinHttpCloseHandle(request);
  if (connection) WinHttpCloseHandle(connection);
  WinHttpCloseHandle(session);
  return response;
}

winrt::hstring JsonKey(std::wstring_view name) {
  return winrt::hstring(name);
}

std::optional<std::wstring> JsonString(const winrt::Windows::Data::Json::JsonObject& object,
                                       std::wstring_view name) {
  try {
    const auto value = object.GetNamedString(JsonKey(name), L"");
    return value.empty() ? std::nullopt : std::optional<std::wstring>(value.c_str());
  } catch (const winrt::hresult_error&) {
    return std::nullopt;
  }
}

std::optional<bool> JsonBoolean(const winrt::Windows::Data::Json::JsonObject& object,
                                std::wstring_view name) {
  try {
    return object.GetNamedBoolean(JsonKey(name), false);
  } catch (const winrt::hresult_error&) {
    return std::nullopt;
  }
}

LyricsDocument ParseLrclibObject(const winrt::Windows::Data::Json::JsonObject& object) {
  if (JsonBoolean(object, L"instrumental") == true) {
    LyricsDocument document;
    document.state = LyricsDocumentState::Instrumental;
    return document;
  }
  const auto syncedLyrics = JsonString(object, L"syncedLyrics");
  if (syncedLyrics && !syncedLyrics->empty()) return ParseLrcUtf8(WideToUtf8(*syncedLyrics));

  LyricsDocument document;
  const auto plainLyrics = JsonString(object, L"plainLyrics");
  if (plainLyrics && !plainLyrics->empty()) {
    document.state = LyricsDocumentState::Loaded;
    document.plainText = *plainLyrics;
  }
  return document;
}

LyricsResolution ParseLrclibResponse(const HttpResponse& response) {
  LyricsResolution result;
  result.httpStatus = response.status;
  result.error = response.error;
  if (!response.error.empty()) return result;
  try {
    const auto object = winrt::Windows::Data::Json::JsonObject::Parse(winrt::to_hstring(response.body));
    result.document = ParseLrclibObject(object);
    result.source = result.document.state == LyricsDocumentState::Loaded ||
                            result.document.state == LyricsDocumentState::Instrumental
                        ? LyricsSource::Lrclib
                        : LyricsSource::None;
    if (result.source == LyricsSource::None && result.document.state == LyricsDocumentState::Empty) {
      result.document.state = LyricsDocumentState::NotFound;
    }
  } catch (const winrt::hresult_error& error) {
    result.document.state = LyricsDocumentState::Invalid;
    result.error = error.message().c_str();
  }
  return result;
}

LyricsResolution ParseLrclibSearchResponse(const HttpResponse& response) {
  LyricsResolution result;
  result.httpStatus = response.status;
  result.error = response.error;
  if (!response.error.empty()) return result;
  try {
    const auto values = winrt::Windows::Data::Json::JsonArray::Parse(winrt::to_hstring(response.body));
    for (std::uint32_t index = 0; index < values.Size(); ++index) {
      auto document = ParseLrclibObject(values.GetObjectAt(index));
      if (document.state == LyricsDocumentState::Loaded || document.state == LyricsDocumentState::Instrumental) {
        result.document = std::move(document);
        result.source = LyricsSource::Lrclib;
        return result;
      }
    }
    result.document.state = LyricsDocumentState::NotFound;
  } catch (const winrt::hresult_error& error) {
    result.document.state = LyricsDocumentState::Invalid;
    result.error = error.message().c_str();
  }
  return result;
}

}  // namespace

const LyricsLine* LyricsDocument::CurrentLine(std::int64_t positionMs) const {
  if (state != LyricsDocumentState::Loaded || lines.empty()) return nullptr;
  const auto offsetMs = metadata && metadata->offsetMs ? *metadata->offsetMs : 0;
  const auto adjustedPositionMs = positionMs - offsetMs;
  const LyricsLine* current = nullptr;
  for (const auto& line : lines) {
    if (line.startMs > adjustedPositionMs) break;
    current = &line;
  }
  return current;
}

LyricsDocument ParseLrcUtf8(std::string_view text) {
  LyricsDocument document;
  if (text.empty()) return document;

  std::wstring wideText;
  try {
    wideText = Utf8ToWide(text);
  } catch (...) {
    document.state = LyricsDocumentState::Invalid;
    document.error = L"Input is not valid UTF-8";
    return document;
  }
  if (!wideText.empty() && wideText.front() == 0xfeff) wideText.erase(0, 1);

  LyricsMetadata metadata;
  bool hasMetadata = false;
  std::vector<LyricsLine> lines;
  std::wostringstream plainText;

  for (const auto& rawLine : SplitLines(wideText)) {
    const auto line = Trim(rawLine);
    std::vector<Timestamp> timestamps;
    std::size_t cursor = 0;
    while (cursor < line.size() && line[cursor] == L'[') {
      auto timestamp = ParseTimestamp(line, cursor);
      if (!timestamp) break;
      timestamps.push_back(*timestamp);
      cursor = timestamp->end;
    }

    if (!timestamps.empty()) {
      const auto lyricText = Trim(std::wstring_view(line).substr(cursor));
      for (const auto& timestamp : timestamps) lines.push_back({timestamp.milliseconds, lyricText});
      continue;
    }

    if (line.size() >= 4 && line.front() == L'[') {
      const auto colon = line.find(L':');
      const auto close = line.find(L']', colon == std::wstring::npos ? 0 : colon);
      if (colon > 1 && close != std::wstring::npos) {
        SetMetadataValue(metadata, std::wstring_view(line).substr(1, colon - 1),
                         std::wstring_view(line).substr(colon + 1, close - colon - 1));
        hasMetadata = true;
        continue;
      }
    }

    if (!line.empty()) plainText << line << L'\n';
  }

  std::stable_sort(lines.begin(), lines.end(), [](const LyricsLine& left, const LyricsLine& right) {
    return left.startMs < right.startMs;
  });
  document.lines = std::move(lines);
  document.plainText = plainText.str();
  if (!document.plainText.empty()) document.plainText.pop_back();
  if (hasMetadata) document.metadata = metadata;
  document.state = document.lines.empty() && document.plainText.empty() && !document.metadata
                       ? LyricsDocumentState::Empty
                       : LyricsDocumentState::Loaded;
  return document;
}

LyricsDocument LoadLyricsFile(const std::filesystem::path& path) {
  std::ifstream input(path, std::ios::binary);
  if (!input) {
    LyricsDocument document;
    document.state = LyricsDocumentState::NotFound;
    return document;
  }
  const std::string text((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
  return ParseLrcUtf8(text);
}

bool SaveLyricsFileAtomic(const std::filesystem::path& path, std::string_view utf8Text) {
  try {
    std::filesystem::create_directories(path.parent_path());
    const auto temporaryPath = path.wstring() + L".tmp";
    {
      std::ofstream output(temporaryPath, std::ios::binary | std::ios::trunc);
      if (!output) return false;
      output.write(utf8Text.data(), static_cast<std::streamsize>(utf8Text.size()));
      output.flush();
      if (!output) return false;
    }
    return MoveFileExW(temporaryPath.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != FALSE;
  } catch (...) {
    return false;
  }
}

std::filesystem::path LyricsDirectory(const std::filesystem::path& installDirectory) {
  return installDirectory / L"resources" / L"lyrics";
}

std::filesystem::path LyricsPathForTrack(const std::filesystem::path& installDirectory,
                                         const LyricsTrackIdentity& track) {
  std::wstring fileName = SafeFilePart(track.artist) + L" - " + SafeFilePart(track.title);
  if (!track.album.empty()) fileName += L" - " + SafeFilePart(track.album);
  fileName += L" - " + std::to_wstring(track.durationMs) + L"ms.lrc";
  return LyricsDirectory(installDirectory) / fileName;
}

std::optional<std::filesystem::path> FindLocalLyrics(const std::filesystem::path& installDirectory,
                                                     const LyricsTrackIdentity& track) {
  const auto path = LyricsPathForTrack(installDirectory, track);
  return std::filesystem::is_regular_file(path) ? std::optional(path) : std::nullopt;
}

LyricsResolution FetchLyricsFromLrclib(const LyricsTrackIdentity& track,
                                       std::wstring_view userAgent,
                                       std::wstring_view apiUrl) {
  const auto durationSeconds = std::max<std::int64_t>(0, (track.durationMs + 500) / 1000);
  const std::wstring getPath = L"/get?track_name=" + Utf8ToWide(UrlEncode(track.title)) +
                               L"&artist_name=" + Utf8ToWide(UrlEncode(track.artist)) +
                               L"&album_name=" + Utf8ToWide(UrlEncode(track.album)) +
                               L"&duration=" + std::to_wstring(durationSeconds);
  auto response = GetLrclib(apiUrl, getPath, userAgent);
  if (response.status == 404 && response.error.empty()) {
    const std::wstring searchPath = L"/search?track_name=" + Utf8ToWide(UrlEncode(track.title)) +
                                    L"&artist_name=" + Utf8ToWide(UrlEncode(track.artist));
    return ParseLrclibSearchResponse(GetLrclib(apiUrl, searchPath, userAgent));
  }
  if (response.status < 200 || response.status >= 300) {
    LyricsResolution result;
    result.httpStatus = response.status;
    result.error = response.error.empty() ? L"LRCLIB request failed" : response.error;
    return result;
  }
  return ParseLrclibResponse(response);
}

LyricsResolution ResolveLyrics(const std::filesystem::path& installDirectory,
                               const LyricsTrackIdentity& track,
                               std::wstring_view userAgent,
                               std::wstring_view apiUrl) {
  LyricsResolution result;
  if (const auto localPath = FindLocalLyrics(installDirectory, track)) {
    result.document = LoadLyricsFile(*localPath);
    if (result.document.state == LyricsDocumentState::Loaded) {
      result.source = LyricsSource::Local;
      result.cachePath = *localPath;
      return result;
    }
  }

  result = FetchLyricsFromLrclib(track, userAgent, apiUrl);
  result.cachePath = LyricsPathForTrack(installDirectory, track);
  if (result.source == LyricsSource::Lrclib && result.document.state == LyricsDocumentState::Loaded &&
      !result.document.lines.empty()) {
    std::string serialized;
    for (const auto& line : result.document.lines) {
      const auto minutes = line.startMs / 60000;
      const auto seconds = (line.startMs / 1000) % 60;
      const auto centiseconds = (line.startMs % 1000) / 10;
      std::ostringstream timestamp;
      timestamp << '[' << std::setfill('0') << std::setw(2) << minutes << ':' << std::setw(2) << seconds << '.'
                << std::setw(2) << centiseconds << "] ";
      serialized += timestamp.str() + WideToUtf8(line.text) + '\n';
    }
    result.cacheSaved = SaveLyricsFileAtomic(result.cachePath, serialized);
    std::error_code existsError;
    result.cacheExists = std::filesystem::is_regular_file(result.cachePath, existsError);
    if (!result.cacheSaved) {
      result.cacheSaveError = GetLastError();
      if (result.error.empty()) result.error = L"Failed to save lyrics cache";
    }
  }
  return result;
}