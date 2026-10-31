#include <math.h>
#include "BigDigits.h"
#include "TimerStyles.h"

// A sideways hourglass: the left bulb is the top and sand drains left to right through the neck.
// Both bulbs are triangles (wide at the end caps, narrow at the neck), so a sand surface at distance d from the
// neck holds a share (d / L)^2 of a bulb. Sand left is therefore at d = L * sqrt(fraction) in the left bulb
// and the pile on the right reaches the same distance from the neck, giving a steady drain.
namespace timerstyle {

namespace {

constexpr int kCap = 12;        // thickness of the end caps
constexpr int kGlass = 4;       // outline thickness
constexpr int kNeckHalf = 6;    // half-opening of the neck
constexpr int kInset = 6;       // sand stays this far inside the glass

// Half the bulb's inner height at column x (linear from the cap down to the neck).
int halfHeight(int x) {
  const int top = (kHgBottom - kHgTop) / 2;
  const int len = kHgNeck - kHgLeft;
  const int d = x < kHgNeck ? kHgNeck - x : x - kHgNeck;
  return kNeckHalf + (top - kNeckHalf) * d / len;
}

void thickLine(Canvas &c, int x0, int y0, int x1, int y1) {
  for (int o = 0; o < kGlass; o++) {
    c.drawLine(x0, y0 + o, x1, y1 + o, 1);
    c.drawLine(x0, y0 - o, x1, y1 - o, 1);
  }
}

void sandColumns(Canvas &c, int xFrom, int xTo) {
  for (int x = xFrom; x <= xTo; x++) {
    const int h = halfHeight(x) - kInset;
    if (h < 1) continue;
    for (int y = kHgMid - h; y <= kHgMid + h; y++)
      if (((x + y) & 1) == 0) c.drawPixel(x, y, 1);  // 50% dither reads as sand
  }
}

}  // namespace

void drawHourglass(Canvas &c, const TimerView &v, bool lastMinute) {
  const float f = remainingFraction(v, lastMinute);
  const int len = kHgNeck - kHgLeft;

  // Glass: two cones meeting at the neck, closed by end caps.
  const int halfCap = (kHgBottom - kHgTop) / 2;
  thickLine(c, kHgLeft, kHgMid - halfCap, kHgNeck, kHgMid - kNeckHalf);
  thickLine(c, kHgLeft, kHgMid + halfCap, kHgNeck, kHgMid + kNeckHalf);
  thickLine(c, kHgNeck, kHgMid - kNeckHalf, kHgRight, kHgMid - halfCap);
  thickLine(c, kHgNeck, kHgMid + kNeckHalf, kHgRight, kHgMid + halfCap);
  c.fillRect(kHgLeft - kCap, kHgTop - 4, kCap, kHgBottom - kHgTop + 8, 1);
  c.fillRect(kHgRight, kHgTop - 4, kCap, kHgBottom - kHgTop + 8, 1);

  // Distance of both sand surfaces from the neck: L * sqrt(remaining fraction).
  const int d = int(float(len) * sqrtf(f));
  if (f > 0.0f) sandColumns(c, kHgNeck - d, kHgNeck - 2);  // left bulb: from the surface to the neck
  sandColumns(c, kHgNeck + d, kHgRight - 2);               // right bulb: the pile from its surface to the cap

  // A thin stream of grains through the neck while sand is still falling; the phase moves with the clock so
  // it appears to fall between redraws.
  if (v.mode != TimerView::Mode::Setting && f > 0.0f && f < 1.0f) {
    const int phase = (v.secondsLeft & 1) * 4;
    for (int x = kHgNeck + 2 + phase; x < kHgNeck + d; x += 8) c.fillRect(x, kHgMid - 1, 3, 3, 1);
  }

  drawValueWithCaption(c, v, lastMinute, kHgRight + kCap + 28, 70, 110, true);
}

}  // namespace timerstyle
