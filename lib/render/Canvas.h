#pragma once
#include <Adafruit_GFX.h>

// Pages render into a 1-bit canvas: pixel value 1 = black ink, 0 = white paper.
using Canvas = GFXcanvas1;
constexpr int kW = 792;
constexpr int kH = 272;
