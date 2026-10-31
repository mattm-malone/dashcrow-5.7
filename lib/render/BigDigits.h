#pragma once
#include "Canvas.h"

// Seven-segment style digits drawn from filled shapes, so any height works.
void drawBigNumber(Canvas &c, int x, int y, int height, int value, int minDigits);
int bigNumberWidth(int height, int digits);
