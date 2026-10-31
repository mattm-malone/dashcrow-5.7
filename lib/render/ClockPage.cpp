#include "ClockPage.h"
#include <stdio.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include "BigDigits.h"

namespace {

constexpr int kDigitH = 170;
constexpr int kDigitY = 16;
constexpr int kGap = 24;
constexpr int kColonW = 40;

const char *const kDays[] = {"SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY"};
const char *const kMonths[] = {"JANUARY", "FEBRUARY", "MARCH",     "APRIL",   "MAY",      "JUNE",
                               "JULY",    "AUGUST",   "SEPTEMBER", "OCTOBER", "NOVEMBER", "DECEMBER"};

void drawColon(Canvas &c, int x) {
  const int s = kDigitH / 9 + 2;
  const int cx = x + (kColonW - s) / 2;
  c.fillRect(cx, kDigitY + kDigitH / 3 - s / 2, s, s, 1);
  c.fillRect(cx, kDigitY + 2 * kDigitH / 3 - s / 2, s, s, 1);
}

void drawDash(Canvas &c, int x) {
  const int t = kDigitH / 9;
  c.fillRect(x, kDigitY + kDigitH / 2 - t / 2, bigNumberWidth(kDigitH, 1), t, 1);
}

void drawText(Canvas &c, int x, int baseline, const char *s) {
  c.setFont(&FreeSansBold24pt7b);
  c.setTextColor(1);
  c.setCursor(x, baseline);
  c.print(s);
}

}  // namespace

void renderClock(Canvas &c, const ClockView &v) {
  c.fillScreen(0);
  const int hour12 = v.hour % 12 == 0 ? 12 : v.hour % 12;
  const int hourDigits = !v.valid ? 2 : (hour12 >= 10 ? 2 : 1);
  const int hw = bigNumberWidth(kDigitH, hourDigits);
  const int mw = bigNumberWidth(kDigitH, 2);
  const int total = hw + kGap + kColonW + kGap + mw;
  const int x0 = (kW - total) / 2;
  const int xc = x0 + hw + kGap;
  const int xm = xc + kColonW + kGap;

  if (v.valid) {
    drawBigNumber(c, x0, kDigitY, kDigitH, hour12, hourDigits);
    drawBigNumber(c, xm, kDigitY, kDigitH, v.minute, 2);
  } else {
    const int cell = bigNumberWidth(kDigitH, 1);
    const int step = cell + kDigitH / 12;
    drawDash(c, x0);
    drawDash(c, x0 + step);
    drawDash(c, xm);
    drawDash(c, xm + step);
  }
  drawColon(c, xc);

  if (!v.valid) return;
  char line[48];
  snprintf(line, sizeof line, "%s %d %s", kDays[v.wday % 7], v.mday, kMonths[(v.month + 11) % 12]);
  drawText(c, 16, kH - 18, line);
  drawText(c, kW - 90, kH - 18, v.hour < 12 ? "AM" : "PM");
}
