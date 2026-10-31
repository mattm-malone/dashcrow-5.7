#include "PanelMap.h"
#include <string.h>

void packForPanel(const uint8_t *canvasBits, uint8_t *out) {
  constexpr int kRowBytes = (kPanelCanvasW + 7) / 8;
  constexpr int kSeamX = 396;  // first logical column on the second controller
  constexpr int kSeamGap = 8;
  constexpr int kMemW = 800;
  constexpr int kMemRowBytes = kMemW / 8;

  memset(out, 0xFF, kPanelBytes);
  for (int y = 0; y < kPanelCanvasH; y++) {
    const uint8_t *row = canvasBits + y * kRowBytes;
    uint8_t *dst = out + (kPanelCanvasH - 1 - y) * kMemRowBytes;
    for (int bx = 0; bx < kRowBytes; bx++) {
      const uint8_t bits = row[bx];
      if (!bits) continue;  // blank 8-pixel run: nothing to clear
      for (int b = 0; b < 8; b++) {
        if (!(bits & (0x80 >> b))) continue;
        int x = bx * 8 + b;
        if (x >= kSeamX) x += kSeamGap;
        const int memX = kMemW - 1 - x;
        dst[memX >> 3] &= ~(0x80 >> (memX & 7));
      }
    }
  }
}
