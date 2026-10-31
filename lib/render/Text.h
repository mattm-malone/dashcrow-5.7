#pragma once
#include <Adafruit_GFX.h>
#include "Canvas.h"

// Text and whole-canvas helpers shared by the pages.
int textWidth(Canvas &c, const GFXfont *f, const char *s, int size = 1);
void drawText(Canvas &c, const GFXfont *f, int x, int baseline, const char *s, int size = 1);
void drawTextRight(Canvas &c, const GFXfont *f, int right, int baseline, const char *s);
void drawTextCentered(Canvas &c, const GFXfont *f, int baseline, const char *s, int size = 1);
void invertCanvas(Canvas &c);
