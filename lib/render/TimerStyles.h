#pragma once
#include "TimerPage.h"

// Internals of the timer page: one drawing function per TimerStyle, plus shared helpers.
// Every style draws below the shared header (label + end time) and may use the whole canvas otherwise.
namespace timerstyle {

// Share of the vessel still full: 1 while setting; in the last minute it is remaining/60 s so the picture
// matches the seconds shown; otherwise remaining/total.
float remainingFraction(const TimerView &v, bool lastMinute);

// The number shown: set minutes while setting, seconds in the last minute, else minutes left.
int displayValue(const TimerView &v, bool lastMinute);

// Big number plus "MIN"/"SEC" and "LEFT" captions. Captions go to the right of the number, or below it.
void drawValueWithCaption(Canvas &c, const TimerView &v, bool lastMinute, int x, int y, int height, bool captionBelow);

// Fills the part of the disc (centre cx,cy radius r) whose clockwise angle from 12 o'clock is >= 1 - fraction,
// so the filled wedge ends at 12 o'clock and its other edge sweeps clockwise as `fraction` falls.
void fillWedge(Canvas &c, int cx, int cy, int r, float fraction);

void drawBarStyle(Canvas &c, const TimerView &v, bool lastMinute);
void drawHourglass(Canvas &c, const TimerView &v, bool lastMinute);
void drawDisc(Canvas &c, const TimerView &v, bool lastMinute);
void drawDots(Canvas &c, const TimerView &v, bool lastMinute);

// Sideways hourglass geometry (also used by tests). The left bulb is the top: sand drains left to right.
constexpr int kHgLeft = 24, kHgRight = 528, kHgTop = 52, kHgBottom = 262;
constexpr int kHgNeck = (kHgLeft + kHgRight) / 2;
constexpr int kHgMid = (kHgTop + kHgBottom) / 2;

// Time Timer disc geometry.
constexpr int kDiscCx = 140, kDiscCy = 158, kDiscR = 96;

// One dot per minute (per 5 minutes above 60 minutes; one per second in the last minute).
struct DotLayout {
  int n, cols, rows, cell, x0, y0, radius;
};
int dotCount(const TimerView &v, bool lastMinute);
DotLayout dotLayout(int n);
void dotCenter(const DotLayout &l, int index, int &x, int &y);

}  // namespace timerstyle
