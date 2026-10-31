#include <unity.h>
#include "BigDigits.h"

void setUp() {}
void tearDown() {}

static int ink(Canvas &c) {
  int n = 0;
  for (int y = 0; y < c.height(); y++)
    for (int x = 0; x < c.width(); x++)
      if (c.getPixel(x, y)) n++;
  return n;
}

static int inkOf(int value, int minDigits) {
  Canvas c(300, 200);
  c.fillScreen(0);
  drawBigNumber(c, 0, 0, 150, value, minDigits);
  return ink(c);
}

void test_digit_zero_has_hole() {
  Canvas c(200, 200);
  c.fillScreen(0);
  drawBigNumber(c, 0, 0, 150, 0, 1);
  int w = bigNumberWidth(150, 1);
  TEST_ASSERT_EQUAL(0, c.getPixel(w / 2, 75));  // centre of the glyph stays white
  TEST_ASSERT_EQUAL(1, c.getPixel(2, 75));      // left bar is black
}

void test_one_uses_less_ink_than_eight() {
  TEST_ASSERT_LESS_THAN(inkOf(8, 1), inkOf(1, 1));
}

void test_width_scales_with_digits() {
  TEST_ASSERT_GREATER_THAN(bigNumberWidth(150, 1), bigNumberWidth(150, 2));
}

void test_min_digits_pads() {
  Canvas c(300, 200);
  c.fillScreen(0);
  drawBigNumber(c, 0, 0, 150, 5, 2);
  int cell = bigNumberWidth(150, 1);
  int leftInk = 0;
  for (int y = 0; y < 150; y++)
    for (int x = 0; x < cell; x++)
      if (c.getPixel(x, y)) leftInk++;
  TEST_ASSERT_GREATER_THAN(0, leftInk);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_digit_zero_has_hole);
  RUN_TEST(test_one_uses_less_ink_than_eight);
  RUN_TEST(test_width_scales_with_digits);
  RUN_TEST(test_min_digits_pads);
  return UNITY_END();
}
