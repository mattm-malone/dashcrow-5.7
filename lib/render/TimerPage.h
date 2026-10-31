#pragma once
#include <stdint.h>
#include "Canvas.h"
#include "Types.h"

struct TimerView {
  enum class Mode { Setting, Running, Paused, Done } mode;
  int setMinutes;          // Setting
  int minutesLeft;         // ceiling of remaining minutes
  int secondsLeft;         // remaining seconds (shown in the last minute)
  uint32_t remainingMs;
  uint32_t totalMs;
  bool endValid;               // false while wall-clock time is unknown: hide the end time
  uint8_t endHour, endMinute;  // 24-hour wall-clock end time (Running/Paused)
  TimerStyle style;            // zero-initialised = Bar
};

constexpr int kBarX = 12;
constexpr int kBarY = 214;
constexpr int kBarW = 768;
constexpr int kBarH = 46;
constexpr int kPreviewMaxMinutes = 90;  // Setting-mode bar spans 0..90 min; longer stays full

int timerBarFilledPx(uint32_t remainingMs, uint32_t totalMs, int barWidthPx);
void renderTimer(Canvas &c, const TimerView &v);
