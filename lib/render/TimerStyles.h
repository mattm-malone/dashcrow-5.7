#pragma once
#include "TimerPage.h"

// Internals of the timer page: one drawing function per TimerStyle, plus shared helpers.
// Every style draws below the shared header (label + end time) and may use the whole canvas otherwise.
namespace timerstyle {

struct StreamDot {
  int x, y, r;
};
constexpr int kStreamDots = 64;  // capacity of hourglassStream()

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

// Halftone hourglass, drawn sideways: two bulbs of dots meeting at a neck in the middle, behind the MM:SS number.
// Dots sit on a regular grid and their size depends only on how far they are from the outer edge (biggest at the
// edge, smallest at the neck). Sand stays anchored at the outer edges: the left bulb empties from the neck side
// outward while the right bulb fills from its outer edge inward, and a line of grains runs through the neck.
constexpr int kHgPitch = 13;                   // dot grid pitch
constexpr int kHgCols = 28;                    // dot columns in each bulb
constexpr int kHgLeftX = 20;                   // outer edge of the left bulb's dots
constexpr int kHgRightX = kW - 20;             // outer edge of the right bulb's dots
constexpr int kHgMid = 158;                    // vertical centre line
constexpr int kHgCapHalf = 98;                 // half height of a bulb at its outer edge
constexpr int kHgNeckHalf = 8;                 // half height at the neck
constexpr int kHgDigitHeight = 100;
constexpr int kHgCenterL = 246, kHgCenterR = kW - 246;  // room for the MM:SS number
int hourglassDotRadius(int col);               // col 0 = outer edge ... kHgCols - 1 = beside the neck
int hourglassRows(int col);                    // rows of dots in that column (the bulb narrows toward the neck)
void hourglassDotCenter(bool rightBulb, int col, int row, int &x, int &y);
// Grains running through the neck: along the centre line, in the gap between the two sand edges.
int hourglassStream(const TimerView &v, StreamDot *out, int cap);

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
