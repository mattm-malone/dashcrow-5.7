#include "StandupOverlay.h"
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include "Text.h"

void renderStandup(Canvas &c) {
  c.fillScreen(0);
  const char *title = "STAND UP";
  int size = 3;
  while (size > 1 && textWidth(c, &FreeSansBold24pt7b, title, size) > kW - 40) size--;
  drawTextCentered(c, &FreeSansBold24pt7b, 150, title, size);
  drawTextCentered(c, &FreeSansBold18pt7b, 235, "press any button", 1);
  invertCanvas(c);  // drawn as black ink on white, then flipped to white-on-black
}
