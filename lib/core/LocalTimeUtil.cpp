#include "LocalTimeUtil.h"

LocalTime localTimeFrom(time_t t) {
  if (t < kClockSyncedAfter) return {false, 0, 0, 0, 0, 0, 0, 0};
  tm lt;
  localtime_r(&t, &lt);
  return {true, uint32_t(t), uint8_t(lt.tm_wday), uint8_t(lt.tm_hour), uint8_t(lt.tm_min), uint8_t(lt.tm_sec),
          uint8_t(lt.tm_mday), uint8_t(lt.tm_mon + 1)};
}

DayInfo dayInfoFor(time_t now, int offsetDays) {
  tm lt;
  localtime_r(&now, &lt);
  // Move by calendar days and evaluate at local noon: noon exists exactly once on every day, even on the
  // 23- and 25-hour DST days, so mktime lands on the right date. tm_isdst = -1 lets libc decide.
  lt.tm_mday += offsetDays;
  lt.tm_hour = 12;
  lt.tm_min = 0;
  lt.tm_sec = 0;
  lt.tm_isdst = -1;
  const time_t noon = mktime(&lt);

  tm local, utc;
  localtime_r(&noon, &local);
  gmtime_r(&noon, &utc);
  int diffMin = (local.tm_hour * 60 + local.tm_min) - (utc.tm_hour * 60 + utc.tm_min);
  if (diffMin > 720) diffMin -= 1440;
  if (diffMin < -720) diffMin += 1440;
  return {uint8_t(local.tm_wday), uint8_t(local.tm_mday), uint8_t(local.tm_mon + 1), local.tm_yday, diffMin / 60.0,
          uint16_t(local.tm_year + 1900)};
}
