#include <unity.h>
#include "MakeTimerView.h"
#include "StandupOverlay.h"
#include "TimerChip.h"

void setUp() {}
void tearDown() {}

static LocalTime at(uint8_t h, uint8_t m, uint8_t s = 0) {
  return {true, uint32_t(h) * 3600u + m * 60u + s, 1, h, m, s};
}

void test_standup_overlay_is_mostly_black() {
  Canvas c(kW, kH);
  renderStandup(c);
  int black = 0;
  for (int y = 0; y < kH; y++)
    for (int x = 0; x < kW; x++) black += c.getPixel(x, y);
  TEST_ASSERT_GREATER_THAN(int(kW * kH * 0.8), black);
}

void test_standup_has_white_text() {
  Canvas c(kW, kH);
  renderStandup(c);
  int white = 0;
  for (int y = 60; y < 200; y++)
    for (int x = 0; x < kW; x++) white += !c.getPixel(x, y);
  TEST_ASSERT_GREATER_THAN(1000, white);
}

void test_chip_draws_in_corner_only() {
  Canvas c(kW, kH);
  c.fillScreen(0);
  drawTimerChip(c, 14);
  int inside = 0, outside = 0;
  for (int y = 0; y < kH; y++)
    for (int x = 0; x < kW; x++) {
      const bool in = x >= kChipX && x < kChipX + kChipW && y >= kChipY && y < kChipY + kChipH;
      if (c.getPixel(x, y)) (in ? inside : outside)++;
    }
  TEST_ASSERT_GREATER_THAN(100, inside);
  TEST_ASSERT_EQUAL(0, outside);
}

void test_view_end_time_is_now_plus_remaining() {
  TimerModel t;
  t.click(0);  // 25 minutes
  TimerView v = makeTimerView(t, at(14, 0));
  TEST_ASSERT_TRUE(v.endValid);
  TEST_ASSERT_EQUAL(14, v.endHour);
  TEST_ASSERT_EQUAL(25, v.endMinute);
  TEST_ASSERT_TRUE(v.mode == TimerView::Mode::Running);
  TEST_ASSERT_EQUAL(25, v.minutesLeft);
}

void test_view_end_time_wraps_past_midnight() {
  TimerModel t;
  t.click(0);
  TimerView v = makeTimerView(t, at(23, 50));
  TEST_ASSERT_EQUAL(0, v.endHour);
  TEST_ASSERT_EQUAL(15, v.endMinute);
}

void test_view_without_valid_time_hides_end() {
  TimerModel t;
  t.click(0);
  LocalTime bad{false, 0, 0, 0, 0, 0};
  TEST_ASSERT_FALSE(makeTimerView(t, bad).endValid);
}

void test_view_setting_and_done_modes() {
  TimerModel t;
  t.turn(5);
  TimerView s = makeTimerView(t, at(9, 0));
  TEST_ASSERT_TRUE(s.mode == TimerView::Mode::Setting);
  TEST_ASSERT_EQUAL(30, s.setMinutes);
  t.click(0);
  t.tick(30u * 60000u);
  TEST_ASSERT_TRUE(makeTimerView(t, at(9, 0)).mode == TimerView::Mode::Done);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_standup_overlay_is_mostly_black);
  RUN_TEST(test_standup_has_white_text);
  RUN_TEST(test_chip_draws_in_corner_only);
  RUN_TEST(test_view_end_time_is_now_plus_remaining);
  RUN_TEST(test_view_end_time_wraps_past_midnight);
  RUN_TEST(test_view_without_valid_time_hides_end);
  RUN_TEST(test_view_setting_and_done_modes);
  return UNITY_END();
}
