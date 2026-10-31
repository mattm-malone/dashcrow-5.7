#include "TimelinePage.h"
#include <stdio.h>
#include <string.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include "Text.h"

namespace {

constexpr int kBarInset = 3;  // blocks stay inside the bar outline
constexpr int kTicksY = kTlBarY + kTlBarH + 2;
constexpr int kLabelBaseline = kTlBarY + kTlBarH + 30;
constexpr int kSunY = kTlBarY + kTlBarH + 44;
constexpr int kSunH = 12;
constexpr int kTitleX = 104;
constexpr int kTitleMaxW = 400;

const char *const kDays[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
const char *const kMonths[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

void fmtTime(char *out, size_t n, int minuteOfDay) {
  const int h = (minuteOfDay / 60) % 24;
  const int h12 = h % 12 == 0 ? 12 : h % 12;
  snprintf(out, n, "%d:%02d %s", h12, minuteOfDay % 60, h < 12 ? "AM" : "PM");
}

void drawHatched(Canvas &c, int x0, int x1, int y0, int y1) {
  for (int y = y0; y < y1; y++)
    for (int x = x0; x < x1; x++)
      if (((x + y) & 1) == 0) c.drawPixel(x, y, 1);
}

void drawHeader(Canvas &c, const TimelineView &v, bool live) {
  char buf[40];
  if (v.timeValid) {
    snprintf(buf, sizeof buf, "%s %d %s", kDays[v.wday % 7], v.mday, kMonths[(v.month + 11) % 12]);
    drawText(c, &FreeSansBold18pt7b, 16, 32, buf);
  } else {
    drawText(c, &FreeSansBold18pt7b, 16, 32, "--");
  }
  if (live) {
    fmtTime(buf, sizeof buf, v.hour * 60 + v.minute);
    drawTextRight(c, &FreeSansBold18pt7b, kW - 16, 32, buf);
  }
  if (v.feedStale) drawText(c, &FreeSansBold9pt7b, kStaleX + 4, kStaleY + 20, "FEED STALE");
}

void drawNextLine(Canvas &c, const TimelineView &v, bool live) {
  const int baseline = kNextY + 36;
  if (!live) {
    char buf[32];
    if (v.n == 0) snprintf(buf, sizeof buf, "No meetings");
    else snprintf(buf, sizeof buf, "%d meeting%s", int(v.n), v.n == 1 ? "" : "s");
    drawText(c, &FreeSansBold18pt7b, 16, baseline, buf);
    return;
  }
  const uint16_t nowMin = v.hour * 60 + v.minute;
  const NextInfo next = nextEvent(v.events, v.n, nowMin);
  if (next.index < 0) {
    drawText(c, &FreeSansBold18pt7b, 16, baseline, "No more meetings today");
    return;
  }
  const DayEvent &e = v.events[next.index];
  drawText(c, &FreeSansBold12pt7b, 16, baseline, next.inProgress ? "NOW" : "NEXT");

  char title[32];
  strncpy(title, e.title, sizeof title - 1);
  title[sizeof title - 1] = '\0';
  for (size_t len = strlen(title); len > 1 && textWidth(c, &FreeSansBold18pt7b, title) > kTitleMaxW;) title[--len] = '\0';
  drawText(c, &FreeSansBold18pt7b, kTitleX, baseline, title);

  char a[16], b[16], when[48];
  fmtTime(a, sizeof a, e.startMin);
  fmtTime(b, sizeof b, e.endMin);
  if (next.inProgress) snprintf(when, sizeof when, "until %s", b);
  else snprintf(when, sizeof when, "in %d min  %s", next.minutesUntil, a);
  drawTextRight(c, &FreeSansBold12pt7b, kW - 16, baseline, when);
}

void drawDayBar(Canvas &c, const TimelineView &v, bool live) {
  const uint16_t nowMin = v.hour * 60 + v.minute;
  c.drawRect(kTlX, kTlBarY, kTlW, kTlBarH, 1);
  c.drawRect(kTlX + 1, kTlBarY + 1, kTlW - 2, kTlBarH - 2, 1);

  const int y0 = kTlBarY + kBarInset;
  const int y1 = kTlBarY + kTlBarH - kBarInset;
  for (size_t i = 0; i < v.n; i++) {
    const int x0 = kTlX + minuteToX(v.events[i].startMin, v.window, kTlW);
    const int x1 = kTlX + minuteToX(v.events[i].endMin, v.window, kTlW);
    if (x1 <= x0) continue;
    if (live && v.events[i].endMin <= nowMin) drawHatched(c, x0, x1, y0, y1);
    else c.fillRect(x0, y0, x1 - x0, y1 - y0, 1);
  }

  // Hour ticks and labels.
  for (int m = (v.window.startMin + 59) / 60 * 60; m <= v.window.endMin; m += 60) {
    const int x = kTlX + minuteToX(m, v.window, kTlW);
    c.fillRect(x, kTicksY, 2, 8, 1);
    char label[4];
    const int h12 = (m / 60) % 12 == 0 ? 12 : (m / 60) % 12;
    snprintf(label, sizeof label, "%d", h12);
    int lx = x - textWidth(c, &FreeSansBold9pt7b, label) / 2;
    if (lx < 0) lx = 0;
    drawText(c, &FreeSansBold9pt7b, lx, kLabelBaseline, label);
  }

  // "Now" marker: a line through the bar with a white halo, topped by a triangle.
  if (live && nowMin >= v.window.startMin && nowMin <= v.window.endMin) {
    const int x = kTlX + minuteToX(nowMin, v.window, kTlW);
    c.fillRect(x - 3, y0, 7, y1 - y0, 0);
    c.fillRect(x - 1, kTlBarY - 14, 3, kTlBarH + 20, 1);
    c.fillTriangle(x - 8, kTlBarY - 24, x + 8, kTlBarY - 24, x, kTlBarY - 10, 1);
  }
}

void drawDaylight(Canvas &c, const TimelineView &v) {
  c.drawRect(kTlX, kSunY, kTlW, kSunH, 1);
  if (v.sun.polarNight) {
    c.fillRect(kTlX, kSunY, kTlW, kSunH, 1);
    return;
  }
  if (v.sun.polarDay) return;
  const int rise = minuteToX(v.sun.sunriseMin, v.window, kTlW);
  const int set = minuteToX(v.sun.sunsetMin, v.window, kTlW);
  c.fillRect(kTlX, kSunY, rise, kSunH, 1);
  c.fillRect(kTlX + set, kSunY, kTlW - set, kSunH, 1);
  char a[16], b[16], line[64];
  fmtTime(a, sizeof a, v.sun.sunriseMin);
  fmtTime(b, sizeof b, v.sun.sunsetMin);
  snprintf(line, sizeof line, "sunrise %s    sunset %s", a, b);
  drawText(c, &FreeSansBold9pt7b, kTlX, kSunY + kSunH + 20, line);
}

}  // namespace

void renderTimeline(Canvas &c, const TimelineView &v) {
  c.fillScreen(0);
  const bool live = v.timeValid && v.dayOffset == 0;
  drawHeader(c, v, live);
  drawNextLine(c, v, live);
  drawDayBar(c, v, live);
  drawDaylight(c, v);
}
