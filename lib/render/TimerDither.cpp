#include <math.h>
#include "TimerStyles.h"

// The hourglass again, but the sand is pixel dither instead of dots. A bulb that is `f` full is solid from its outer
// edge up to a sand edge, then fades to empty over kSandFade pixels; the fade is an 8x8 ordered (Bayer) dither, so as
// time passes the dithered band slides along the bulb: the emptying bulb gets lighter, the filling one heavier.
// A line of grains runs along the centre between the two fades.
namespace timerstyle {

namespace {

// 8x8 Bayer matrix: a pixel is black where the density exceeds (value + 0.5) / 64.
const uint8_t kBayer[64] = {0,  32, 8,  40, 2,  34, 10, 42, 48, 16, 56, 24, 50, 18, 58, 26, 12, 44, 4,  36, 14, 46,
                            6,  38, 60, 28, 52, 20, 62, 30, 54, 22, 3,  35, 11, 43, 1,  33, 9,  41, 51, 19, 59, 27,
                            49, 17, 57, 25, 15, 47, 7,  39, 13, 45, 5,  37, 63, 31, 55, 23, 61, 29, 53, 21};

int floorMod(int a, int b) { return ((a % b) + b) % b; }

// Half the height of the glass at this distance from a bulb's outer edge.
float bulbHalfHeight(int d) {
  return float(kHgNeckHalf) + float(kHgCapHalf - kHgNeckHalf) * float(kSandBulb - d) / float(kSandBulb);
}

void drawBulb(Canvas &c, bool rightBulb, float fraction) {
  const int xStart = rightBulb ? kW / 2 : kHgLeftX;
  const int xEnd = rightBulb ? kHgRightX : kW / 2 - 1;
  for (int x = xStart; x <= xEnd; x++) {
    const int d = rightBulb ? kHgRightX - x : x - kHgLeftX;
    const float density = sandDensity(float(d), fraction);
    if (density <= 0.0f) continue;
    const int half = int(bulbHalfHeight(d));
    for (int y = kHgMid - half; y <= kHgMid + half; y++) {
      const float threshold = (float(kBayer[(y & 7) * 8 + (x & 7)]) + 0.5f) / 64.0f;
      if (density >= 1.0f || density > threshold) c.drawPixel(x, y, 1);
    }
  }
}

}  // namespace

float sandDensity(float distFromOuter, float fraction) {
  const float edge = -kSandFade / 2.0f + fraction * (float(kSandBulb) + kSandFade);  // centre of the fade
  const float d = (edge - distFromOuter) / kSandFade + 0.5f;
  return d < 0.0f ? 0.0f : (d > 1.0f ? 1.0f : d);
}

int ditherStream(const TimerView &v, StreamDot *out, int cap) {
  if (v.style != TimerStyle::Dither) return 0;
  if (v.mode != TimerView::Mode::Running && v.mode != TimerView::Mode::Paused) return 0;
  if (v.totalMs == 0 || v.remainingMs == 0 || v.remainingMs >= v.totalMs) return 0;  // nothing is pouring

  const float f = float(v.remainingMs) / float(v.totalMs);
  const float travel = float(kSandBulb) + kSandFade;
  const int from = int(ceilf(float(kHgLeftX) + f * travel));               // where the left fade has ended
  const int to = int(floorf(float(kHgRightX) - (1.0f - f) * travel));      // where the right fade begins
  const int elapsedSec = int((v.totalMs - v.remainingMs) / 1000);
  const int residue = (kSandGrainStep * elapsedSec) % kSandGrainPitch;     // the lattice marches right each second

  int n = 0;
  for (int x = from + floorMod(residue - from, kSandGrainPitch); x <= to && n < cap; x += kSandGrainPitch) {
    out[n].x = x;
    out[n].y = kHgMid;
    out[n].r = 2;
    n++;
  }
  return n;
}

void drawDitherHourglass(Canvas &c, const TimerView &v, bool /*lastMinute*/) {
  float f;
  int minutes, seconds;
  hourglassValues(v, f, minutes, seconds);

  drawHourglassGlass(c);
  drawBulb(c, false, f);
  drawBulb(c, true, 1.0f - f);

  StreamDot grains[kStreamDots];
  const int n = ditherStream(v, grains, kStreamDots);
  for (int i = 0; i < n; i++) c.fillCircle(grains[i].x, grains[i].y, grains[i].r, 1);

  drawHourglassNumber(c, minutes, seconds, true);  // on a clean white plate: solid sand would swamp the digits
}

}  // namespace timerstyle
