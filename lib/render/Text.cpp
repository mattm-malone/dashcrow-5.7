#include "Text.h"

int textWidth(Canvas &c, const GFXfont *f, const char *s, int size) {
  int16_t x1, y1;
  uint16_t w, h;
  c.setFont(f);
  c.setTextSize(size);
  c.getTextBounds(s, 0, 0, &x1, &y1, &w, &h);
  return w;
}

void drawText(Canvas &c, const GFXfont *f, int x, int baseline, const char *s, int size) {
  c.setFont(f);
  c.setTextSize(size);
  c.setTextColor(1);
  c.setCursor(x, baseline);
  c.print(s);
}

void drawTextRight(Canvas &c, const GFXfont *f, int right, int baseline, const char *s) {
  drawText(c, f, right - textWidth(c, f, s, 1), baseline, s);
}

void drawTextCentered(Canvas &c, const GFXfont *f, int baseline, const char *s, int size) {
  drawText(c, f, (kW - textWidth(c, f, s, size)) / 2, baseline, s, size);
}

void invertCanvas(Canvas &c) {
  uint8_t *b = c.getBuffer();
  const int n = ((c.width() + 7) / 8) * c.height();
  for (int i = 0; i < n; i++) b[i] ^= 0xFF;
}
