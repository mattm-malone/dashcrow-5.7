#include <unity.h>
#include "TimerPage.h"

void setUp() {}
void tearDown() {}

using Mode = TimerView::Mode;
constexpr uint32_t kMin = 60000;

static TimerView running(uint32_t totalMin, uint32_t remainingMs) {
  TimerView v{};
  v.mode = Mode::Running;
  v.totalMs = totalMin * kMin;
  v.remainingMs = remainingMs;
  v.minutesLeft = int((remainingMs + kMin - 1) / kMin);
  v.secondsLeft = int((remainingMs + 999) / 1000);
  v.endHour = 15;
  v.endMinute = 25;
  return v;
}

static TimerView setting(int minutes) {
  TimerView v{};
  v.mode = Mode::Setting;
  v.setMinutes = minutes;
  return v;
}

static int barInk(Canvas &c) {
  int n = 0;
  for (int y = kBarY; y < kBarY + kBarH; y++)
    for (int x = kBarX; x < kBarX + kBarW; x++)
      if (c.getPixel(x, y)) n++;
  return n;
}

void test_filled_px_proportional() {
  TEST_ASSERT_EQUAL(384, timerBarFilledPx(30000, 60000, 768));
  TEST_ASSERT_EQUAL(768, timerBarFilledPx(60000, 60000, 768));
  TEST_ASSERT_EQUAL(0, timerBarFilledPx(0, 60000, 768));
}

void test_running_bar_drains_left() {
  Canvas c(kW, kH);
  renderTimer(c, running(100, 25 * kMin));  // 25% of a 100-minute timer
  TEST_ASSERT_EQUAL(1, c.getPixel(kBarX + 10, kBarY + 10));
  TEST_ASSERT_EQUAL(0, c.getPixel(kBarX + kBarW - 10, kBarY + 10));
}

void test_segmented_under_60() {
  Canvas c(kW, kH);
  renderTimer(c, running(25, 20 * kMin));
  TEST_ASSERT_EQUAL(0, c.getPixel(kBarX + kBarW / 25, kBarY + 10));  // gap between segments
  TEST_ASSERT_EQUAL(1, c.getPixel(kBarX + 10, kBarY + 10));          // inside first segment
}

void test_continuous_at_60() {
  Canvas c(kW, kH);
  renderTimer(c, running(60, 30 * kMin));
  for (int x = kBarX + 5; x < kBarX + kBarW / 2 - 5; x++)
    TEST_ASSERT_EQUAL_MESSAGE(1, c.getPixel(x, kBarY + 10), "gap inside continuous fill");
}

void test_last_minute_inverts() {
  Canvas c(kW, kH);
  renderTimer(c, running(25, 30000));
  TEST_ASSERT_EQUAL(1, c.getPixel(2, 2));
  Canvas d(kW, kH);
  renderTimer(d, running(25, 10 * kMin));
  TEST_ASSERT_EQUAL(0, d.getPixel(2, 2));
}

void test_done_inverted() {
  Canvas c(kW, kH);
  TimerView v = running(25, 0);
  v.mode = Mode::Done;
  renderTimer(c, v);
  TEST_ASSERT_EQUAL(1, c.getPixel(2, 2));
}

void test_setting_previews_bar() {
  Canvas a(kW, kH), b(kW, kH);
  renderTimer(a, setting(90));
  renderTimer(b, setting(30));
  TEST_ASSERT_EQUAL(1, a.getPixel(kBarX + kBarW - 10, kBarY + 10));  // 90 fills the whole bar
  TEST_ASSERT_EQUAL(0, b.getPixel(kBarX + kBarW - 10, kBarY + 10));
  TEST_ASSERT_NOT_EQUAL(barInk(a), barInk(b));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_filled_px_proportional);
  RUN_TEST(test_running_bar_drains_left);
  RUN_TEST(test_segmented_under_60);
  RUN_TEST(test_continuous_at_60);
  RUN_TEST(test_last_minute_inverts);
  RUN_TEST(test_done_inverted);
  RUN_TEST(test_setting_previews_bar);
  return UNITY_END();
}
