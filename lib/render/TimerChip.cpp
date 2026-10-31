#include "TimerChip.h"
#include <stdio.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include "Text.h"

void drawTimerChip(Canvas &c, int minutesLeft) {
  c.fillRect(kChipX, kChipY, kChipW, kChipH, 0);
  c.drawRect(kChipX, kChipY, kChipW, kChipH, 1);
  c.drawRect(kChipX + 1, kChipY + 1, kChipW - 2, kChipH - 2, 1);
  char label[16];
  snprintf(label, sizeof label, "%d MIN", minutesLeft);
  drawText(c, &FreeSansBold12pt7b, kChipX + (kChipW - textWidth(c, &FreeSansBold12pt7b, label)) / 2, kChipY + 28, label);
}
