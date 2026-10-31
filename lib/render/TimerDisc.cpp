#include <math.h>
#include "TimerStyles.h"

// Time Timer style: a solid disc whose filled wedge shrinks as time passes, ticks every 5 minutes of a
// 60-minute dial, and the number beside it.
namespace timerstyle {

void fillWedge(Canvas &c, int cx, int cy, int r, float fraction) {
  if (fraction <= 0.0f) return;
  constexpr float kTwoPi = 6.28318530718f;
  const float from = 1.0f - fraction;
  for (int dy = -r; dy <= r; dy++) {
    for (int dx = -r; dx <= r; dx++) {
      if (dx * dx + dy * dy > r * r) continue;
      float t = atan2f(float(dx), float(-dy)) / kTwoPi;  // clockwise from 12 o'clock
      if (t < 0.0f) t += 1.0f;
      if (t >= from) c.drawPixel(cx + dx, cy + dy, 1);
    }
  }
}

void drawDisc(Canvas &c, const TimerView &v, bool lastMinute) {
  constexpr float kTwoPi = 6.28318530718f;
  const float f = remainingFraction(v, lastMinute);

  // Rim and 12 ticks (every 30 degrees; thicker at 12, 3, 6 and 9).
  for (int i = 0; i < 3; i++) c.drawCircle(kDiscCx, kDiscCy, kDiscR + 4 + i, 1);
  for (int k = 0; k < 12; k++) {
    const float a = kTwoPi * float(k) / 12.0f;
    const bool major = k % 3 == 0;
    const int inner = kDiscR + 9, outer = kDiscR + (major ? 22 : 16);
    const int x0 = kDiscCx + int(roundf(sinf(a) * inner)), y0 = kDiscCy - int(roundf(cosf(a) * inner));
    const int x1 = kDiscCx + int(roundf(sinf(a) * outer)), y1 = kDiscCy - int(roundf(cosf(a) * outer));
    c.drawLine(x0, y0, x1, y1, 1);
    c.drawLine(x0 + 1, y0, x1 + 1, y1, 1);
    if (major) c.drawLine(x0, y0 + 1, x1, y1 + 1, 1);
  }

  fillWedge(c, kDiscCx, kDiscCy, kDiscR, f);
  drawValueWithCaption(c, v, lastMinute, 300, 56, 150, false);
}

}  // namespace timerstyle
