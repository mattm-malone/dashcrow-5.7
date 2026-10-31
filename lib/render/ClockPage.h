#pragma once
#include "Canvas.h"

struct ClockView {
  bool valid;
  uint8_t hour, minute;  // 24-hour local time
  uint8_t wday;          // 0 = Sunday
  uint8_t mday, month;   // month 1-12
};

void renderClock(Canvas &c, const ClockView &v);
