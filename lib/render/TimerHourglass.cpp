#include <math.h>
#include "BigDigits.h"
#include "TimerStyles.h"

// A sideways hourglass of halftone dots. Dots sit on a regular grid (see kHg* in TimerStyles.h); each column has
// its own dot size, biggest at the outer edge and smallest beside the neck, so size depends on position alone.
// The left bulb holds the time still to come (it empties from the neck side outward, one column per 1/28 of the
// timer), the right bulb the time gone by (it fills from its outer edge inward), and a line of grains runs
// through the neck between the two sand edges. The MM:SS number sits over the neck with a white halo so it stays
// legible where the small neck dots overlap it.
namespace timerstyle {

namespace {

constexpr int kColonW = 30;
constexpr int kNumberGap = 14;
constexpr int kHalo = 3;
constexpr int kGlassMargin = 8;     // outline sits this far outside the dots
constexpr int kGlassThickness = 4;
constexpr int kGrainSpacing = 3;    // every third column carries a grain

int halfHeightAt(int col) { return kHgNeckHalf + (kHgCapHalf - kHgNeckHalf) * (kHgCols - 1 - col) / (kHgCols - 1); }

int columnX(bool rightBulb, int col) {
  return rightBulb ? kHgRightX - col * kHgPitch - kHgPitch / 2 : kHgLeftX + col * kHgPitch + kHgPitch / 2;
}

int floorMod(int a, int b) { return ((a % b) + b) % b; }

void thickLine(Canvas &c, int x0, int y0, int x1, int y1) {
  for (int o = -kGlassThickness / 2; o < kGlassThickness / 2; o++) c.drawLine(x0, y0 + o, x1, y1 + o, 1);
}

// The glass outline stops where the number starts, so it points at the neck without cutting through the digits.
void drawGlass(Canvas &c) {
  const int capTop = kHgMid - kHgCapHalf - kGlassMargin, capBottom = kHgMid + kHgCapHalf + kGlassMargin;
  const int neckTop = kHgMid - kHgNeckHalf - kGlassMargin, neckBottom = kHgMid + kHgNeckHalf + kGlassMargin;
  const int xl = kHgLeftX - 6, xr = kHgRightX + 6, xm = kW / 2;

  // Where the left diagonals reach the number's box (the right ones mirror them).
  const int yTopAtBox = capTop + (neckTop - capTop) * (kHgCenterL - xl) / (xm - xl);
  const int yBottomAtBox = capBottom + (neckBottom - capBottom) * (kHgCenterL - xl) / (xm - xl);
  thickLine(c, xl, capTop, kHgCenterL, yTopAtBox);
  thickLine(c, xl, capBottom, kHgCenterL, yBottomAtBox);
  thickLine(c, kHgCenterR, yTopAtBox, xr, capTop);
  thickLine(c, kHgCenterR, yBottomAtBox, xr, capBottom);
  c.fillRect(xl - 10, capTop - 2, 10, capBottom - capTop + 4, 1);  // end caps
  c.fillRect(xr, capTop - 2, 10, capBottom - capTop + 4, 1);
}

// One bulb of sand. `extent` is how many columns, counted from the outer edge, still hold dots; the last of them
// shrinks smoothly so the picture changes a little at every redraw.
void drawSand(Canvas &c, bool rightBulb, float extent) {
  for (int col = 0; col < kHgCols && float(col) < extent; col++) {
    float scale = extent - float(col);
    if (scale > 1.0f) scale = 1.0f;
    const float r = float(hourglassDotRadius(col)) * scale;
    for (int row = 0; row < hourglassRows(col); row++) {
      int x, y;
      hourglassDotCenter(rightBulb, col, row, x, y);
      if (r >= 0.9f) c.fillCircle(x, y, int(r + 0.5f), 1);
      else if (r >= 0.45f) c.drawPixel(x, y, 1);
    }
  }
}

int numberWidth(int height, int minuteDigits) {
  return bigNumberWidth(height, minuteDigits) + kNumberGap + kColonW + kNumberGap + bigNumberWidth(height, 2);
}

// MM:SS (or MMM:SS), scaled to fit the room in the middle; `color` 0 erases, for the halo.
void drawNumber(Canvas &c, int x, int y, int height, int minutes, int minuteDigits, int seconds, uint16_t color, int grow) {
  drawBigNumber(c, x, y, height, minutes, minuteDigits, color);
  const int colonX = x + bigNumberWidth(height, minuteDigits) + kNumberGap;
  const int dot = height / 9 + 2 + 2 * grow;
  c.fillRect(colonX + (kColonW - dot) / 2, y + height / 3 - dot / 2, dot, dot, color);
  c.fillRect(colonX + (kColonW - dot) / 2, y + 2 * height / 3 - dot / 2, dot, dot, color);
  drawBigNumber(c, colonX + kColonW + kNumberGap, y, height, seconds, 2, color);
}

void drawClock(Canvas &c, int minutes, int seconds) {
  const int minuteDigits = minutes >= 100 ? 3 : 2;
  const int room = kHgCenterR - kHgCenterL;
  int height = kHgDigitHeight;
  while (height > 40 && numberWidth(height, minuteDigits) > room) height -= 4;
  const int x = kHgCenterL + (room - numberWidth(height, minuteDigits)) / 2;
  const int y = kHgMid - height / 2;

  // White halo: the number printed in white, shifted around by a few pixels, before it is printed in black.
  for (int dy = -kHalo; dy <= kHalo; dy += kHalo)
    for (int dx = -kHalo; dx <= kHalo; dx += kHalo)
      if (dx || dy) drawNumber(c, x + dx, y + dy, height, minutes, minuteDigits, seconds, 0, 0);
  drawNumber(c, x, y, height, minutes, minuteDigits, seconds, 1, 0);
}

}  // namespace

int hourglassDotRadius(int col) {
  // Linear from 6.2 px at the outer edge to 1.5 px beside the neck, rounded: the same for every dot in a column.
  const float r = 1.5f + 4.7f * float(kHgCols - 1 - col) / float(kHgCols - 1);
  return int(r + 0.5f);
}

int hourglassRows(int col) {
  const int rows = 2 * halfHeightAt(col) / kHgPitch;
  return rows < 1 ? 1 : rows;
}

void hourglassDotCenter(bool rightBulb, int col, int row, int &x, int &y) {
  x = columnX(rightBulb, col);
  y = kHgMid + (2 * row - (hourglassRows(col) - 1)) * kHgPitch / 2;
}

int hourglassStream(const TimerView &v, StreamDot *out, int cap) {
  if (v.style != TimerStyle::Hourglass) return 0;
  if (v.mode != TimerView::Mode::Running && v.mode != TimerView::Mode::Paused) return 0;
  if (v.totalMs == 0 || v.remainingMs == 0 || v.remainingMs >= v.totalMs) return 0;  // nothing is pouring

  const float f = float(v.remainingMs) / float(v.totalMs);
  const int firstCol = int(ceilf(f * float(kHgCols)));            // first column past the left bulb's sand
  const int lastCol = 2 * kHgCols - 1 - int(ceilf((1.0f - f) * float(kHgCols)));  // last column before the right pile
  const int elapsedSec = int((v.totalMs - v.remainingMs) / 1000);

  int n = 0;
  for (int u = firstCol; u <= lastCol && n < cap; u++) {
    if (floorMod(u - elapsedSec, kGrainSpacing) != 0) continue;  // one grain in three, marching right each second
    const bool rightBulb = u >= kHgCols;
    const int col = rightBulb ? 2 * kHgCols - 1 - u : u;
    out[n].x = columnX(rightBulb, col);
    out[n].y = kHgMid;
    out[n].r = hourglassDotRadius(col) + 1;
    n++;
  }
  return n;
}

void drawHourglass(Canvas &c, const TimerView &v, bool /*lastMinute*/) {
  // Unlike the other styles the last minute does not rescale the picture: sand cannot flow back.
  float f = 1.0f;
  int minutes, seconds;
  if (v.mode == TimerView::Mode::Setting) {
    minutes = v.setMinutes;
    seconds = 0;
  } else {
    f = v.totalMs == 0 ? 0.0f : float(v.remainingMs) / float(v.totalMs);
    if (f > 1.0f) f = 1.0f;
    const int totalSeconds = int((v.remainingMs + 999) / 1000);
    minutes = totalSeconds / 60;
    seconds = totalSeconds % 60;
  }

  drawGlass(c);
  drawSand(c, false, f * float(kHgCols));
  drawSand(c, true, (1.0f - f) * float(kHgCols));

  StreamDot grains[kStreamDots];
  const int n = hourglassStream(v, grains, kStreamDots);
  for (int i = 0; i < n; i++) c.fillCircle(grains[i].x, grains[i].y, grains[i].r, 1);

  drawClock(c, minutes, seconds);
}

}  // namespace timerstyle
