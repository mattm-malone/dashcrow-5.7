#include <stdio.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include "Text.h"
#include "TimerStyles.h"

// One dot per minute (per 5 minutes above an hour; one per second in the last minute). Dots empty from the
// end, and the one being used up is drawn as a shrinking pie.
namespace timerstyle {

namespace {
constexpr int kAreaX = 16, kAreaY = 54, kAreaW = 760, kAreaH = 208;
constexpr int kRing = 3;
}  // namespace

int dotCount(const TimerView &v, bool lastMinute) {
  if (lastMinute) return 60;
  if (v.mode == TimerView::Mode::Setting) {
    const int m = v.setMinutes;
    return m <= 60 ? m : (m + 4) / 5;
  }
  const int total = int((v.totalMs + 59999) / 60000);
  return total <= 60 ? total : (total + 4) / 5;
}

DotLayout dotLayout(int n) {
  DotLayout best{n, 1, n, 0, kAreaX, kAreaY, 0};
  if (n < 1) return best;
  for (int cols = 1; cols <= n; cols++) {
    const int rows = (n + cols - 1) / cols;
    const int cell = min(kAreaW / cols, kAreaH / rows);
    if (cell > best.cell) best = {n, cols, rows, cell, 0, 0, 0};
  }
  best.radius = best.cell * 42 / 100;
  best.x0 = kAreaX + (kAreaW - best.cols * best.cell) / 2;
  best.y0 = kAreaY + (kAreaH - best.rows * best.cell) / 2;
  return best;
}

void dotCenter(const DotLayout &l, int index, int &x, int &y) {
  x = l.x0 + (index % l.cols) * l.cell + l.cell / 2;
  y = l.y0 + (index / l.cols) * l.cell + l.cell / 2;
}

void drawDots(Canvas &c, const TimerView &v, bool lastMinute) {
  const int n = dotCount(v, lastMinute);
  if (n < 1) return;
  const DotLayout l = dotLayout(n);

  // Remaining time in dot units: seconds in the last minute, minutes (or 5-minute blocks) otherwise.
  float units;
  if (v.mode == TimerView::Mode::Setting) units = float(n);
  else if (lastMinute) units = float(v.remainingMs) / 1000.0f;
  else units = float(v.remainingMs) / (60000.0f * float(v.totalMs > 60u * 60000u ? 5 : 1));

  for (int i = 0; i < n; i++) {
    int x, y;
    dotCenter(l, i, x, y);
    for (int k = 0; k < kRing; k++) c.drawCircle(x, y, l.radius - k, 1);
    if (units >= float(i + 1)) c.fillCircle(x, y, l.radius, 1);
    else if (units > float(i)) fillWedge(c, x, y, l.radius - 1, units - float(i));
  }

  // The number sits in the header, between the label and the end time.
  char text[24];
  if (v.mode == TimerView::Mode::Setting) snprintf(text, sizeof text, "%d MIN", displayValue(v, lastMinute));
  else snprintf(text, sizeof text, "%d %s LEFT", displayValue(v, lastMinute), lastMinute ? "SEC" : "MIN");
  drawTextCentered(c, &FreeSansBold18pt7b, 32, text, 1);
}

}  // namespace timerstyle
