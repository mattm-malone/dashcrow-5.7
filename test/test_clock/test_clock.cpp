#include <unity.h>
#include "BigDigits.h"
#include "ClockPage.h"

void setUp() {}
void tearDown() {}

static int ink(Canvas &c) {
  int n = 0;
  for (int y = 0; y < c.height(); y++)
    for (int x = 0; x < c.width(); x++)
      if (c.getPixel(x, y)) n++;
  return n;
}

void test_valid_time_draws_ink_in_digit_area() {
  Canvas c(kW, kH);
  c.fillScreen(0);
  renderClock(c, {true, 14, 41, 4, 30, 10});
  TEST_ASSERT_GREATER_THAN(2000, ink(c));
}

void test_invalid_time_differs_from_valid() {
  Canvas a(kW, kH), b(kW, kH);
  a.fillScreen(0);
  b.fillScreen(0);
  renderClock(a, {true, 14, 41, 4, 30, 10});
  renderClock(b, {false, 0, 0, 0, 0, 0});
  TEST_ASSERT_NOT_EQUAL(ink(a), ink(b));
}

void test_invalid_time_still_draws_dashes() {
  Canvas c(kW, kH);
  c.fillScreen(0);
  renderClock(c, {false, 0, 0, 0, 0, 0});
  TEST_ASSERT_GREATER_THAN(500, ink(c));
}

void test_twelve_hour_clock_one_digit_hour() {
  // 13:05 shows "1:05", 10:05 shows "10:05": the latter is wider, so it uses more ink.
  Canvas a(kW, kH), b(kW, kH);
  a.fillScreen(0);
  b.fillScreen(0);
  renderClock(a, {true, 13, 5, 1, 1, 1});
  renderClock(b, {true, 10, 5, 1, 1, 1});
  TEST_ASSERT_GREATER_THAN(ink(a), ink(b));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_valid_time_draws_ink_in_digit_area);
  RUN_TEST(test_invalid_time_differs_from_valid);
  RUN_TEST(test_invalid_time_still_draws_dashes);
  RUN_TEST(test_twelve_hour_clock_one_digit_hour);
  return UNITY_END();
}
