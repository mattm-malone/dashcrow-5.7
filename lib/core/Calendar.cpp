#include "Calendar.h"
#include <string.h>

size_t eventsForDay(const Meeting *meetings, size_t n, uint8_t wday, DayEvent *out, size_t cap) {
  size_t count = 0;
  for (size_t i = 0; i < n && cap > 0; i++) {
    const Meeting &m = meetings[i];
    if (!(m.weekdayMask & (1u << (wday % 7)))) continue;
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
