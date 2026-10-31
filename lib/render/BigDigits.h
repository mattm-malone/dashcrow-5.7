#pragma once
#include "Canvas.h"

// Seven-segment style digits drawn from filled shapes, so any height works.
// `color` is 1 for ink (default) or 0 to erase, e.g. to draw a halo behind the number.
void drawBigNumber(Canvas &c, int x, int y, int height, int value, int minDigits, uint16_t color = 1);
int bigNumberWidth(int height, int digits);
