#pragma once
#include <stddef.h>
#include <stdint.h>
#include "Calendar.h"
#include "Canvas.h"
#include "Daylight.h"

// The next meeting on a later day, shown when nothing is left today.
struct UpcomingMeeting {
  bool valid;
  char title[32];
  uint8_t daysAhead;  // 1 = tomorrow
  uint8_t wday;       // 0 = Sunday
  uint16_t startMin;
};

struct TimelineView {
  bool timeValid;             // false: no "now" marker, no countdown
  uint8_t hour, minute;       // current time
  uint8_t wday, mday, month;  // the day being shown (month 1-12)
  const DayEvent *events;
  size_t n;
  DayWindow window;
  SunTimes sun;
  int dayOffset;              // 0 = today; other days show no "now" marker or countdown
  bool feedStale;
  UpcomingMeeting upcoming;   // used for today's NEXT line when no meeting is left
};

constexpr int kTlX = 12;
constexpr int kTlW = 768;
constexpr int kTlBarY = 128;
constexpr int kTlBarH = 48;
constexpr int kNextY = 46;
constexpr int kNextH = 56;
constexpr int kStaleX = 410;
constexpr int kStaleY = 4;
constexpr int kStaleW = 140;
constexpr int kStaleH = 30;

void renderTimeline(Canvas &c, const TimelineView &v);
