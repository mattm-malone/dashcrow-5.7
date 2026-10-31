#include <unity.h>
#include <string.h>
#include "PanelMap.h"

void setUp() {}
void tearDown() {}

constexpr int kRow = (kPanelCanvasW + 7) / 8;  // 99 bytes
static uint8_t canvas[kRow * kPanelCanvasH];
static uint8_t out[kPanelBytes];
static uint8_t ref[kPanelBytes];

static void setInk(int x, int y) { canvas[y * kRow + x / 8] |= 0x80 >> (x % 8); }

// Independent restatement of lib/EPD/EPD.cpp Paint_SetPixel for rotate == 180, widthMemory 800.
static void referencePixel(int x, int y, bool black) {
  if (x >= 396) x += 8;
  const int X = 800 - x - 1;
  const int Y = 272 - y - 1;
  const int addr = X / 8 + Y * 100;
  if (black) ref[addr] &= ~(0x80 >> (X % 8));
  else ref[addr] |= 0x80 >> (X % 8);
}

void test_blank_canvas_is_all_white() {
  memset(canvas, 0, sizeof canvas);
  packForPanel(canvas, out);
  for (int i = 0; i < kPanelBytes; i++) TEST_ASSERT_EQUAL_HEX8(0xFF, out[i]);
}

void test_corner_pixel_lands_flipped() {
  memset(canvas, 0, sizeof canvas);
  setInk(0, 0);  // top-left of the picture is the bottom-right of the driver buffer
  packForPanel(canvas, out);
  TEST_ASSERT_EQUAL_HEX8(0xFE, out[271 * 100 + 99]);
  int cleared = 0;
  for (int i = 0; i < kPanelBytes; i++) cleared += out[i] != 0xFF;
  TEST_ASSERT_EQUAL(1, cleared);
}

void test_seam_columns_395_and_396() {
  memset(canvas, 0, sizeof canvas);
  setInk(395, 10);
  setInk(396, 10);
  packForPanel(canvas, out);
  // x=395 -> memory column 800-395-1 = 404 (byte 50, bit 4); x=396 -> 800-404-1 = 395 (byte 49, bit 3).
  const int row = (272 - 10 - 1) * 100;
  TEST_ASSERT_EQUAL_HEX8(0xFF & ~(0x80 >> 4), out[row + 50]);
  TEST_ASSERT_EQUAL_HEX8(0xFF & ~(0x80 >> 3), out[row + 49]);
}

void test_all_ink_leaves_only_the_dead_columns_white() {
  memset(canvas, 0xFF, sizeof canvas);
  packForPanel(canvas, out);
  const int row = 100 * 5;
  TEST_ASSERT_EQUAL_HEX8(0x0F, out[row + 49]);  // memory columns 396..399 stay white
  TEST_ASSERT_EQUAL_HEX8(0xF0, out[row + 50]);  // memory columns 400..403 stay white
  TEST_ASSERT_EQUAL_HEX8(0x00, out[row + 10]);
}

void test_matches_vendor_mapping_on_pseudo_random_canvas() {
  memset(canvas, 0, sizeof canvas);
  memset(ref, 0xFF, sizeof ref);
  uint32_t s = 12345;
  for (int y = 0; y < kPanelCanvasH; y++)
    for (int x = 0; x < kPanelCanvasW; x++) {
      s = s * 1664525u + 1013904223u;
      const bool ink = (s >> 24) & 1;
      if (ink) setInk(x, y);
      referencePixel(x, y, ink);
    }
  packForPanel(canvas, out);
  TEST_ASSERT_EQUAL_MEMORY(ref, out, kPanelBytes);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_blank_canvas_is_all_white);
  RUN_TEST(test_corner_pixel_lands_flipped);
  RUN_TEST(test_seam_columns_395_and_396);
  RUN_TEST(test_all_ink_leaves_only_the_dead_columns_white);
  RUN_TEST(test_matches_vendor_mapping_on_pseudo_random_canvas);
  return UNITY_END();
}
