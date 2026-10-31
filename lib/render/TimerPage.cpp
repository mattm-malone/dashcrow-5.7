#include "TimerPage.h"
#include <stdio.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include "BigDigits.h"
#include "Text.h"
#include "TimerStyles.h"

namespace {

constexpr int kBorder = 3;
constexpr int kSegGap = 3;
constexpr int kDigitH = 150;
constexpr int kDigitY = 44;
constexpr uint32_t kMinuteMs = 60000;

// Outlined bar; `filled` pixels are solid from the left. segments > 1 cuts one gap per
// minute boundary (3 px, centred on the boundary) so each remaining minute reads as a block.
void drawBar(Canvas &c, int filled, int segments) {
  for (int i = 0; i < kBorder; i++) c.drawRect(kBarX + i, kBarY + i, kBarW - 2 * i, kBarH - 2 * i, 1);
  c.fillRect(kBarX, kBarY, filled, kBarH, 1);
  for (int i = 1; i < segments; i++) c.fillRect(kBarX + i * kBarW / segments - 1, kBarY, kSegGap, kBarH, 0);
}

void formatEnd(char *out, size_t n, uint8_t hour, uint8_t minute) {
  const int h12 = hour % 12 == 0 ? 12 : hour % 12;
  snprintf(out, n, "ends %d:%02d %s", h12, minute, hour < 12 ? "AM" : "PM");
}

}  // namespace

namespace timerstyle {

float remainingFraction(const TimerView &v, bool lastMinute) {
  if (v.mode == TimerView::Mode::Setting) return 1.0f;
  if (lastMinute) return float(v.remainingMs) / float(kMinuteMs);
  if (v.totalMs == 0) return 0.0f;
  const float f = float(v.remainingMs) / float(v.totalMs);
  return f > 1.0f ? 1.0f : f;
}

int displayValue(const TimerView &v, bool lastMinute) {
  if (v.mode == TimerView::Mode::Setting) return v.setMinutes;
  return lastMinute ? v.secondsLeft : v.minutesLeft;
}

void drawValueWithCaption(Canvas &c, const TimerView &v, bool lastMinute, int x, int y, int height, bool captionBelow) {
  const int value = displayValue(v, lastMinute);
  const bool setting = v.mode == TimerView::Mode::Setting;
  drawBigNumber(c, x, y, height, value, 1);
  const int digits = value >= 100 ? 3 : (value >= 10 ? 2 : 1);
  const char *unit = lastMinute ? "SEC" : "MIN";
  if (captionBelow) {
    const int base = y + height + 34;
    drawText(c, &FreeSansBold18pt7b, x, base, unit);
    if (!setting) drawText(c, &FreeSansBold18pt7b, x + 84, base, "LEFT");
    return;
  }
  const int captionX = x + bigNumberWidth(height, digits) + 28;
  drawText(c, &FreeSansBold24pt7b, captionX, y + 68, unit);
  if (!setting) drawText(c, &FreeSansBold24pt7b, captionX, y + 120, "LEFT");
}

void drawBarStyle(Canvas &c, const TimerView &v, bool lastMinute) {
  drawValueWithCaption(c, v, lastMinute, 24, kDigitY, kDigitH, false);
  if (v.mode == TimerView::Mode::Setting) {
    const int m = v.setMinutes > kPreviewMaxMinutes ? kPreviewMaxMinutes : v.setMinutes;
    drawBar(c, m * kBarW / kPreviewMaxMinutes, 0);
  } else {
    const int segments = v.totalMs < 60u * kMinuteMs ? int((v.totalMs + kMinuteMs - 1) / kMinuteMs) : 0;
    drawBar(c, timerBarFilledPx(v.remainingMs, v.totalMs, kBarW), segments);
  }
}

}  // namespace timerstyle

int timerBarFilledPx(uint32_t remainingMs, uint32_t totalMs, int barWidthPx) {
  if (totalMs == 0) return 0;
  if (remainingMs > totalMs) remainingMs = totalMs;
  return int(uint64_t(remainingMs) * uint64_t(barWidthPx) / totalMs);
}

void renderTimer(Canvas &c, const TimerView &v) {
  c.fillScreen(0);

  if (v.mode == TimerView::Mode::Done) {
    drawTextCentered(c, &FreeSansBold24pt7b, 150, "TIME'S UP", 2);
    drawTextCentered(c, &FreeSansBold18pt7b, 230, "press any button", 1);
    invertCanvas(c);
    return;
  }

  const bool setting = v.mode == TimerView::Mode::Setting;
  const bool lastMinute = !setting && v.remainingMs > 0 && v.remainingMs <= kMinuteMs;

  drawText(c, &FreeSansBold18pt7b, 16, 32, setting ? "SET TIMER" : (v.mode == TimerView::Mode::Paused ? "PAUSED" : "FOCUS"));
  if (v.endValid) {
    char end[24];
    formatEnd(end, sizeof end, v.endHour, v.endMinute);
    drawTextRight(c, &FreeSansBold18pt7b, kW - 16, 32, end);
  }

  switch (v.style) {
    case TimerStyle::Hourglass: timerstyle::drawHourglass(c, v, lastMinute); break;
    case TimerStyle::Disc: timerstyle::drawDisc(c, v, lastMinute); break;
    case TimerStyle::Dots: timerstyle::drawDots(c, v, lastMinute); break;
    default: timerstyle::drawBarStyle(c, v, lastMinute); break;
  }

  if (lastMinute) invertCanvas(c);
}
