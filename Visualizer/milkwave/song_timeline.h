#pragma once

#include <algorithm>
#include <cstdlib>
#include <cstdint>

class SongTimelineClock {
 public:
  void Reset() {
    initialized_ = false;
    reportedPositionMs_ = 0;
    durationMs_ = 0;
    updatedUtcMs_ = 0;
    anchorSteadyMs_ = 0;
    anchorPositionMs_ = 0;
    rate_ = 1.0;
    playing_ = false;
  }

  void Update(std::int64_t positionMs,
              std::int64_t durationMs,
              std::int64_t updatedUtcMs,
              std::int64_t nowUtcMs,
              std::int64_t nowSteadyMs,
              bool playing,
              double rate) {
    if (positionMs < 0 || durationMs <= 0 || updatedUtcMs < updatedUtcMs_) return;

    rate = (rate >= 0.0 && rate <= 16.0) ? rate : 1.0;
    const auto predictedPositionMs = Position(nowSteadyMs);
    auto correctedPositionMs = positionMs;
    if (playing && nowUtcMs >= updatedUtcMs) {
      correctedPositionMs += static_cast<std::int64_t>((nowUtcMs - updatedUtcMs) * rate);
    }
    correctedPositionMs = std::clamp(correctedPositionMs, std::int64_t{0}, durationMs);

    const bool durationChanged = initialized_ && durationMs != durationMs_;
    const bool playbackChanged = initialized_ && (playing != playing_ || rate != rate_);
    const bool backwardSeek = initialized_ && correctedPositionMs < predictedPositionMs;
    const auto correctionMs = initialized_ ? std::llabs(correctedPositionMs - predictedPositionMs) : durationMs;
    const bool largeCorrection = correctionMs >= 100;

    if (!initialized_ || durationChanged || playbackChanged || backwardSeek || largeCorrection) {
      anchorPositionMs_ = correctedPositionMs;
      anchorSteadyMs_ = nowSteadyMs;
    } else {
      anchorPositionMs_ = predictedPositionMs;
      anchorSteadyMs_ = nowSteadyMs;
    }

    initialized_ = true;
    reportedPositionMs_ = positionMs;
    durationMs_ = durationMs;
    updatedUtcMs_ = updatedUtcMs;
    rate_ = rate;
    playing_ = playing;
  }

  std::int64_t Position(std::int64_t nowSteadyMs) const {
    if (!initialized_) return -1;
    if (!playing_ || nowSteadyMs <= anchorSteadyMs_) return std::clamp(anchorPositionMs_, std::int64_t{0}, durationMs_);
    const auto elapsedMs = nowSteadyMs - anchorSteadyMs_;
    const auto positionMs = anchorPositionMs_ + static_cast<std::int64_t>(elapsedMs * rate_);
    return std::clamp(positionMs, std::int64_t{0}, durationMs_);
  }

  std::int64_t Duration() const { return durationMs_; }
  bool IsInitialized() const { return initialized_; }

 private:
  bool initialized_ = false;
  std::int64_t reportedPositionMs_ = 0;
  std::int64_t durationMs_ = 0;
  std::int64_t updatedUtcMs_ = 0;
  std::int64_t anchorSteadyMs_ = 0;
  std::int64_t anchorPositionMs_ = 0;
  double rate_ = 1.0;
  bool playing_ = false;
};