#include "BigDigits.h"

namespace {

// Segment bits: a(top) b(top-right) c(bottom-right) d(bottom) e(bottom-left) f(top-left) g(middle)
enum : uint8_t { A = 1, B = 2, C = 4, D = 8, E = 16, F = 32, G = 64 };
constexpr uint8_t kDigits[10] = {
    A | B | C | D | E | F,      // 0
    B | C,                      // 1
    A | B | D | E | G,          // 2
    A | B | C | D | G,          // 3
    B | C | F | G,              // 4
    A | C | D | F | G,          // 5
    A | C | D | E | F | G,      // 6
    A | B | C,                  // 7
    A | B | C | D | E | F | G,  // 8
    A | B | C | D | F | G,      // 9
};

int cellWidth(int height) { return height * 11 / 20; }
int gapWidth(int height) { return height / 12; }

void drawDigit(Canvas &c, int x, int y, int h, int digit, uint16_t color) {
  const int w = cellWidth(h);
  const int t = h / 9;
  const int mid = y + h / 2 - t / 2;
  const int half = h / 2 + t / 2;
  const uint8_t s = kDigits[digit];
  if (s & A) c.fillRect(x, y, w, t, color);
  if (s & D) c.fillRect(x, y + h - t, w, t, color);
  if (s & G) c.fillRect(x, mid, w, t, color);
  if (s & F) c.fillRect(x, y, t, half, color);
  if (s & E) c.fillRect(x, mid, t, half, color);
  if (s & B) c.fillRect(x + w - t, y, t, half, color);
  if (s & C) c.fillRect(x + w - t, mid, t, half, color);
}

}  // namespace

int bigNumberWidth(int height, int digits) {
  return digits * cellWidth(height) + (digits > 1 ? (digits - 1) * gapWidth(height) : 0);
}

void drawBigNumber(Canvas &c, int x, int y, int height, int value, int minDigits, uint16_t color) {
  if (value < 0) value = 0;
  int n = 1;
  for (int v = value; v >= 10; v /= 10) n++;
  if (n < minDigits) n = minDigits;
  const int step = cellWidth(height) + gapWidth(height);
  for (int i = n - 1; i >= 0; i--) {
    drawDigit(c, x + i * step, y, height, value % 10, color);
    value /= 10;
  }
}
