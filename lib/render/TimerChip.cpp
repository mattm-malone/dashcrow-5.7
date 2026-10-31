#include "TimerChip.h"
#include <stdio.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include "Text.h"

void drawTimerChip(Canvas &c, int minutesLeft, int x) {
  c.fillRect(x, kChipY, kChipW, kChipH, 0);
  c.drawRect(x, kChipY, kChipW, kChipH, 1);
  c.drawRect(x + 1, kChipY + 1, kChipW - 2, kChipH - 2, 1);
  char label[16];
  snprintf(label, sizeof label, "%d MIN", minutesLeft);
  drawText(c, &FreeSansBold12pt7b, x + (kChipW - textWidth(c, &FreeSansBold12pt7b, label)) / 2, kChipY + 28, label);
}
