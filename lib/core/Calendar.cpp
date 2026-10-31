#include "Calendar.h"
#include <string.h>

int32_t daysFromCivil(int year, int month, int mday) {
  // Howard Hinnant's days_from_civil: shifts the year to start in March so leap days land at its end.
  year -= month <= 2;
  const int era = (year >= 0 ? year : year - 399) / 400;
  const unsigned yoe = unsigned(year - era * 400);
  const unsigned doy = unsigned((153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + mday - 1);
  const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097 + int32_t(doe) - 719468;
}

namespace {

int32_t floorDiv(int32_t a, int32_t b) { return a / b - ((a % b != 0) && ((a < 0) != (b < 0))); }
int32_t floorMod(int32_t a, int32_t b) { return a - floorDiv(a, b) * b; }

// Weekday (0 = Sunday) of a day count; 1970-01-01 was a Thursday.
int weekdayOf(int32_t days) { return floorMod(days + 4, 7); }

bool occursOn(const Meeting &m, const DayDate &day) {
  if (!(m.weekdayMask & (1u << (day.wday % 7)))) return false;
  if (m.everyNWeeks < 2) return true;
  // Weeks are Sunday-based, counted from the Sunday of the anchor's week.
  const int32_t anchor = daysFromCivil(m.anchorYear, m.anchorMonth, m.anchorMday);
  const int32_t anchorSunday = anchor - weekdayOf(anchor);
  const int32_t weeks = floorDiv(daysFromCivil(day.year, day.month, day.mday) - anchorSunday, 7);
  return floorMod(weeks, m.everyNWeeks) == 0;
}

}  // namespace

size_t eventsForDay(const Meeting *meetings, size_t n, const DayDate &day, DayEvent *out, size_t cap) {
  size_t count = 0;
  for (size_t i = 0; i < n && cap > 0; i++) {
    const Meeting &m = meetings[i];
    if (!occursOn(m, day)) continue;
    const uint16_t end = m.endMin > 1440 ? 1440 : m.endMin;
    if (end <= m.startMin) continue;

    // Insertion position keeps `out` sorted by start (stable for equal starts).
    size_t pos = count;
    while (pos > 0 && out[pos - 1].startMin > m.startMin) pos--;
    if (pos >= cap) continue;  // full, and this one would sort after everything kept
    const size_t last = count < cap ? count : cap - 1;
    for (size_t k = last; k > pos; k--) out[k] = out[k - 1];
    strncpy(out[pos].title, m.title, sizeof out[pos].title - 1);
    out[pos].title[sizeof out[pos].title - 1] = '\0';
    out[pos].startMin = m.startMin;
    out[pos].endMin = end;
    if (count < cap) count++;
  }
  return count;
}

NextInfo nextEvent(const DayEvent *events, size_t n, uint16_t nowMin) {
  for (size_t i = 0; i < n; i++) {
    if (events[i].endMin <= nowMin) continue;  // already over
    if (events[i].startMin <= nowMin) return {int(i), true, 0};
    return {int(i), false, int(events[i].startMin) - int(nowMin)};
  }
  return {-1, false, 0};
}

int minuteToX(uint16_t minute, DayWindow w, int widthPx) {
  if (w.endMin <= w.startMin || minute <= w.startMin) return 0;
  if (minute >= w.endMin) return widthPx;
  return int((uint32_t(minute - w.startMin) * uint32_t(widthPx)) / uint32_t(w.endMin - w.startMin));
}
