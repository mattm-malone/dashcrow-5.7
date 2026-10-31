#pragma once
#include "Canvas.h"

// Small bordered "N MIN" chip in the top-right corner, drawn over other pages while a timer runs.
constexpr int kChipX = kW - 150;
constexpr int kChipY = 6;
constexpr int kChipW = 140;
constexpr int kChipH = 40;

void drawTimerChip(Canvas &c, int minutesLeft);
