#include <math.h>
#include "BigDigits.h"
#include "TimerStyles.h"

// Halftone hourglass. Each field is a grid of dots whose size shrinks from the field's outer edge toward the
// centre; "mass" m (0..1) sets how far the dense part reaches. The left field holds the time still to come
// (m = remaining fraction), the right field the time gone by (m = 1 - fraction), so dots appear to flow left
// to right. A fixed per-cell hash scatters the thin edge into loose grains and keeps the picture stable between
// redraws, so dots change size smoothly instead of flickering.
namespace timerstyle {

namespace {

constexpr float kRamp = 7.0f;      // columns over which the dots thin out
constexpr float kReach = float(kHgCols) - 1.5f;  // dense core + taper at a full field, so the inner edge always thins
constexpr float kMaxRadius = 5.5f;
constexpr int kColonW = 30;
constexpr int kGap = 14;

// Deterministic 0..1 value for a grid cell.
float cellNoise(int col, int row, int salt) {
  uint32_t h = uint32_t(col) * 374761393u + uint32_t(row) * 668265263u + uint32_t(salt) * 2246822519u;
  h = (h ^ (h >> 13)) * 1274126177u;
  h ^= h >> 16;
  return float(h & 0xFFFF) / 65535.0f;
}

// `drift` slides the loose-grain pattern a column per second: toward the centre on the left, outward on the right.
void drawField(Canvas &c, int x0, float mass, bool mirrored, int salt, int drift) {
  if (mass <= 0.002f) return;
  const float edge = mass * kReach;
  for (int col = 0; col < kHgCols; col++) {
    const float k = float(col);  // distance from the field's outer edge, in columns
    if (k > edge + 2.0f) break;
    for (int row = 0; row < kHgRows; row++) {
      float d = (edge - k) / kRamp;
      d = d < 0.0f ? 0.0f : (d > 1.0f ? 1.0f : d);
      d += (cellNoise(col + drift, row, salt) - 0.5f) * 0.5f;  // loose grains along the thin edge
      if (d <= 0.0f) continue;
      if (d > 1.0f) d = 1.0f;
      const int gridCol = mirrored ? kHgCols - 1 - col : col;
      const int cx = x0 + gridCol * kHgPitch + kHgPitch / 2;
      const int cy = kHgFieldY + row * kHgPitch + kHgPitch / 2;
      const float r = d * kMaxRadius;
      if (r >= 0.9f) c.fillCircle(cx, cy, int(r + 0.5f), 1);
      else if (r >= 0.45f) c.drawPixel(cx, cy, 1);
    }
  }
}

int numberWidth(int height, int minuteDigits) {
  return bigNumberWidth(height, minuteDigits) + kGap + kColonW + kGap + bigNumberWidth(height, 2);
}

// MM:SS, or M:SS... with as many minute digits as needed, scaled down to fit between the fields.
void drawClock(Canvas &c, int minutes, int seconds) {
  const int minuteDigits = minutes >= 100 ? 3 : 2;
  const int room = kHgCenterR - kHgCenterL;
  int height = 100;
  while (height > 40 && numberWidth(height, minuteDigits) > room) height -= 4;
  const int total = numberWidth(height, minuteDigits);
  const int x = kHgCenterL + (room - total) / 2;
  const int y = kHgFrameY + (kHgFrameH - height) / 2;

  drawBigNumber(c, x, y, height, minutes, minuteDigits);
  const int colonX = x + bigNumberWidth(height, minuteDigits) + kGap;
  const int dot = height / 9 + 2;
  c.fillRect(colonX + (kColonW - dot) / 2, y + height / 3 - dot / 2, dot, dot, 1);
  c.fillRect(colonX + (kColonW - dot) / 2, y + 2 * height / 3 - dot / 2, dot, dot, 1);
  drawBigNumber(c, colonX + kColonW + kGap, y, height, seconds, 2);
}

}  // namespace

float streamPhase(uint32_t elapsedMs) {
  // kStreamSpeed = 4/5 spacings per second, in integers so the phase is exact and wraps cleanly.
  return float((uint64_t(elapsedMs) * 4 / 5) % 1000) / 1000.0f;
}

int hourglassStream(const TimerView &v, StreamDot *out, int cap) {
  if (v.style != TimerStyle::Hourglass) return 0;
  if (v.mode != TimerView::Mode::Running && v.mode != TimerView::Mode::Paused) return 0;
  if (v.totalMs == 0 || v.remainingMs == 0 || v.remainingMs >= v.totalMs) return 0;  // nothing pouring

  // The stream runs between the two sand surfaces: from the inner edge of the draining left field to the inner edge
  // of the right pile, which both move as time passes, in a quadratic arch that clears the number.
  const float f = float(v.remainingMs) / float(v.totalMs);
  const float reach = kReach * float(kHgPitch) * 0.9f;
  const float p0x = float(kHgLeftX) + f * reach, p0y = 96.0f;
  const float p2x = float(kW - 20) - (1.0f - f) * reach, p2y = 100.0f;
  const float apex = 62.0f;
  const float p1x = (p0x + p2x) / 2.0f, p1y = (apex - 0.25f * (p0y + p2y)) / 0.5f;  // puts the arch's peak at `apex`
  const float length = (p2x - p0x) * 1.1f;                                           // rough arc length
  const float phase = streamPhase(v.totalMs - v.remainingMs);

  int n = 0;
  for (int i = 0; n < cap; i++) {
    const float t = (float(i) + phase) * kStreamSpacing / length;
    if (t > 1.0f) break;
    const float a = (1.0f - t) * (1.0f - t), b = 2.0f * (1.0f - t) * t, c2 = t * t;
    out[n].x = int(a * p0x + b * p1x + c2 * p2x + 0.5f);
    out[n].y = int(a * p0y + b * p1y + c2 * p2y + 0.5f);
    out[n].r = int(5.2f - 3.0f * t + 0.5f);  // thick where it leaves, thin where it lands
    n++;
  }
  return n;
}

void drawHourglass(Canvas &c, const TimerView &v, bool /*lastMinute*/) {
  // Unlike the other styles the last minute does not rescale the picture: dots cannot flow back.
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

  for (int i = 0; i < 3; i++) c.drawRect(kHgFrameX + i, kHgFrameY + i, kHgFrameW - 2 * i, kHgFrameH - 2 * i, 1);
  const int elapsedSec = v.mode == TimerView::Mode::Setting || v.totalMs < v.remainingMs ? 0 : int((v.totalMs - v.remainingMs) / 1000);
  drawField(c, kHgLeftX, f, false, 1, -elapsedSec);
  drawField(c, kHgRightX, 1.0f - f, true, 2, elapsedSec);

  StreamDot dots[kStreamDots];
  const int n = hourglassStream(v, dots, kStreamDots);
  for (int i = 0; i < n; i++) {
    c.fillCircle(dots[i].x, dots[i].y, dots[i].r, 1);
    // Loose specks either side of each grain give the stream a ragged, liquid edge (fixed per grain, so they move with it).
    const float u = cellNoise(i, 7, 3), w = cellNoise(i, 8, 4);
    if (u > 0.45f) c.fillCircle(dots[i].x + int((u - 0.7f) * 18.0f), dots[i].y + 5 + int(w * 7.0f), 1, 1);
    if (w > 0.55f) c.fillCircle(dots[i].x + int((w - 0.8f) * 18.0f), dots[i].y - 5 - int(u * 5.0f), 1, 1);
  }
  drawClock(c, minutes, seconds);
}

}  // namespace timerstyle
