#pragma once
#include <stddef.h>
#include <stdint.h>

// A recurring meeting from config. Times are minutes since local midnight.
// Weekly by default. Set everyNWeeks >= 2 (e.g. 2 = every other week) together with an anchor date: any
// date in the week of a real occurrence. Weeks run Sunday-Saturday and are counted from the anchor's week.
struct Meeting {
  const char *title;
  uint8_t weekdayMask;  // bit 0 = Sunday
  uint16_t startMin, endMin;
  uint8_t everyNWeeks;   // 0 or 1 = every week; the anchor below is then ignored
  uint16_t anchorYear;
  uint8_t anchorMonth, anchorMday;
};

// A calendar day. wday: 0 = Sunday.
struct DayDate {
  uint16_t year;
  uint8_t month, mday, wday;
};

// Days since 1970-01-01 for a Gregorian date (month 1-12). Pure arithmetic: no time zone, no libc.
int32_t daysFromCivil(int year, int month, int mday);

// One event on a specific day (from config meetings or the calendar feed).
struct DayEvent {
  char title[32];
  uint16_t startMin, endMin;
};

// Meetings that occur on `day`, sorted by start. End is clipped to 24:00; zero or negative
// length is dropped; at most `cap` earliest events are returned.
size_t eventsForDay(const Meeting *meetings, size_t n, const DayDate &day, DayEvent *out, size_t cap);

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
