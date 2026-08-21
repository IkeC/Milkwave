#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

inline constexpr wchar_t kDefaultLyricsApiUrl[] = L"https://lrclib.net/api";

enum class LyricsDocumentState {
  Empty,
  Loading,
  Loaded,
  Instrumental,
  NotFound,
  Invalid,
};

struct LyricsLine {
  std::int64_t startMs = 0;
  std::wstring text;
};

struct LyricsMetadata {
  std::wstring artist;
  std::wstring album;
  std::wstring title;
  std::optional<std::int64_t> lengthMs;
  std::optional<std::int64_t> offsetMs;
};

struct LyricsDocument {
  LyricsDocumentState state = LyricsDocumentState::Empty;
  std::vector<LyricsLine> lines;
  std::optional<LyricsMetadata> metadata;
  std::wstring plainText;
  std::wstring error;

  const LyricsLine* CurrentLine(std::int64_t positionMs) const;
};

LyricsDocument ParseLrcUtf8(std::string_view text);
LyricsDocument LoadLyricsFile(const std::filesystem::path& path);
bool SaveLyricsFileAtomic(const std::filesystem::path& path, std::string_view utf8Text);

struct LyricsTrackIdentity {
  std::wstring artist;
  std::wstring title;
  std::wstring album;
  std::int64_t durationMs = 0;
};

std::filesystem::path LyricsDirectory(const std::filesystem::path& installDirectory);
std::filesystem::path LyricsPathForTrack(const std::filesystem::path& installDirectory,
                                         const LyricsTrackIdentity& track);
std::optional<std::filesystem::path> FindLocalLyrics(const std::filesystem::path& installDirectory,
                                                     const LyricsTrackIdentity& track);

enum class LyricsSource {
  None,
  Local,
  Lrclib,
};

struct LyricsResolution {
  LyricsDocument document;
  LyricsSource source = LyricsSource::None;
  int httpStatus = 0;
  std::wstring error;
  std::filesystem::path cachePath;
  bool cacheSaved = false;
  bool cacheExists = false;
  unsigned long cacheSaveError = 0;
};

LyricsResolution FetchLyricsFromLrclib(const LyricsTrackIdentity& track,
               std::wstring_view userAgent = L"Milkwave/1.0",
               std::wstring_view apiUrl = kDefaultLyricsApiUrl);
LyricsResolution ResolveLyrics(const std::filesystem::path& installDirectory,
                               const LyricsTrackIdentity& track,
             std::wstring_view userAgent = L"Milkwave/1.0",
             std::wstring_view apiUrl = kDefaultLyricsApiUrl);