#include "Lyrics.h"

#include <cassert>
#include <filesystem>
#include <string>

#include "song_timeline.h"

namespace {

void TestParseAndSelection() {
  const auto document = ParseLrcUtf8("[ar:bjork]\n[ti:Joga]\n[offset:500]\n[00:02.5][00:01]One\n[00:03]\xE2\x9C\x93");
  assert(document.state == LyricsDocumentState::Loaded);
  assert(document.lines.size() == 3);
  assert(document.lines[0].startMs == 1000);
  assert(document.lines[1].startMs == 2500);
  assert(document.lines[2].text == L"\x2713");
  assert(document.CurrentLine(1499) == nullptr);
  assert(document.CurrentLine(1500)->text == L"One");
  assert(document.CurrentLine(3500)->text == L"\x2713");
}

void TestDuplicateAndMalformedInput() {
  const auto document = ParseLrcUtf8("[00:01]first\n[00:01]second\n[bad]ignored\nplain");
  assert(document.state == LyricsDocumentState::Loaded);
  assert(document.CurrentLine(1000)->text == L"second");
  assert(document.plainText == L"plain");

  const auto invalid = ParseLrcUtf8("\xFF");
  assert(invalid.state == LyricsDocumentState::Invalid);
}

void TestSongTimelineClock() {
  SongTimelineClock clock;
  clock.Update(1000, 10000, 1000, 1000, 0, true, 1.0);
  assert(clock.Position(500) == 1500);

  clock.Update(1500, 10000, 1500, 1500, 500, false, 1.0);
  assert(clock.Position(1500) == 1500);
  assert(clock.Position(2500) == 1500);

  clock.Update(1500, 10000, 2500, 2500, 2500, true, 2.0);
  assert(clock.Position(3000) == 2500);
  assert(clock.Position(3500) == 3500);

  clock.Update(1200, 10000, 2000, 3000, 3500, true, 1.0);
  assert(clock.Position(3500) >= 1200);

  clock.Update(9000, 5000, 4000, 4000, 4000, true, 1.0);
  assert(clock.Position(4000) == 5000);
}

void TestLocalResolution() {
  const auto root = std::filesystem::temp_directory_path() / L"milkwave-lyrics-tests";
  std::filesystem::remove_all(root);
  const LyricsTrackIdentity track{L"Björk", L"Joga", L"Homogenic", 245000};
  const auto expected = LyricsPathForTrack(root, track);
  std::filesystem::create_directories(expected.parent_path());
  assert(SaveLyricsFileAtomic(expected, "[00:01]hello\n"));
  assert(LoadLyricsFile(expected).CurrentLine(1000)->text == L"hello");
  assert(LyricsDirectory(root) == root / L"resources" / L"lyrics");
  assert(FindLocalLyrics(root, track) == expected);
  std::filesystem::remove_all(root);
}

void TestLrclibFearOfTheDark() {
  const auto root = std::filesystem::temp_directory_path() / L"milkwave-lrclib-tests";
  std::filesystem::remove_all(root);
  const LyricsTrackIdentity track{L"Iron Maiden", L"Fear of the Dark", L"Fear of the Dark", 437000};
  const auto result = ResolveLyrics(root, track, L"MilkwaveLyricsTests/1.0");
  assert(result.httpStatus == 200);
  assert(result.source == LyricsSource::Lrclib);
  assert(result.document.state == LyricsDocumentState::Loaded);
  assert(!result.document.lines.empty());
  assert(result.document.lines.front().startMs == 52520);
  assert(result.document.lines.front().text == L"I am a man who walks alone");
  assert(FindLocalLyrics(root, track).has_value());

  const auto cached = ResolveLyrics(root, track, L"MilkwaveLyricsTests/1.0");
  assert(cached.source == LyricsSource::Local);
  std::filesystem::remove_all(root);
}

}  // namespace

int main(int argc, char** argv) {
  TestParseAndSelection();
  TestDuplicateAndMalformedInput();
  TestSongTimelineClock();
  TestLocalResolution();
  if (argc > 1 && std::string(argv[1]) == "--network") TestLrclibFearOfTheDark();
  return 0;
}