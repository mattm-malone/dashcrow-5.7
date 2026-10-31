#pragma once
#include <stddef.h>
#include <stdint.h>

// A recurring weekly meeting from config. Times are minutes since local midnight.
struct Meeting {
  const char *title;
  uint8_t weekdayMask;  // bit 0 = Sunday
  uint16_t startMin, endMin;
};

// One event on a specific day (from config meetings or the calendar feed).
struct DayEvent {
  char title[32];
  uint16_t startMin, endMin;
};

// Meetings that occur on `wday`, sorted by start. End is clipped to 24:00; zero or negative
// length is dropped; at most `cap` earliest events are returned.
size_t eventsForDay(const Meeting *meetings, size_t n, uint8_t wday, DayEvent *out, size_t cap);

struct NextInfo {
  int index;         // -1 if nothing is left today
  bool inProgress;
  int minutesUntil;  // 0 when in progress
};
// The event in progress (earliest start) if any, otherwise the next one to start.
NextInfo nextEvent(const DayEvent *events, size_t n, uint16_t nowMin);

struct DayWindow {
  uint16_t startMin, endMin;
};
// Pixel column for a time of day within the window, clamped to [0, widthPx].
int minuteToX(uint16_t minute, DayWindow w, int widthPx);
