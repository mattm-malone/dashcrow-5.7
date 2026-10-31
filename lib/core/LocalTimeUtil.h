#pragma once
#include <stdint.h>
#include <time.h>
#include "Types.h"

// Epoch values below this mean NTP has not set the clock yet (anything before Nov 2023).
constexpr time_t kClockSyncedAfter = 1700000000;

// Wall-clock fields in the local zone (TZ env / configTzTime). valid=false until the clock is set.
LocalTime localTimeFrom(time_t t);

struct DayInfo {
  uint8_t wday, mday, month;  // month 1-12
  int yday;                   // 0-based day of year
  double tzHours;             // local UTC offset on that day (noon), DST included
};

// The calendar day `offsetDays` from the local day containing `now`. Steps by calendar days (via noon),
// not by 24 h, so 23- and 25-hour DST days cannot land on the wrong date.
DayInfo dayInfoFor(time_t now, int offsetDays);
