#include <unity.h>
#include "TimerModel.h"

using State = TimerModel::State;
constexpr uint32_t kMin = 60000;

void setUp() {}
void tearDown() {}

void test_defaults() {
  TimerModel t;
  TEST_ASSERT_TRUE(t.state() == State::Setting);
  TEST_ASSERT_EQUAL(25, t.setMinutes());
}

void test_turn_clamps() {
  TimerModel t;
  t.turn(-100);
  TEST_ASSERT_EQUAL(1, t.setMinutes());
  t.turn(999);
  TEST_ASSERT_EQUAL(180, t.setMinutes());
}

void test_turn_ignored_when_running() {
  TimerModel t;
  t.click(0);
  t.turn(5);
  TEST_ASSERT_EQUAL(25, t.setMinutes());
}

void test_click_starts() {
  TimerModel t;
  t.click(1000);
  TEST_ASSERT_TRUE(t.state() == State::Running);
  TEST_ASSERT_EQUAL_UINT32(25 * kMin, t.remainingMs());
}

void test_remaining_counts_down() {
  TimerModel t;
  t.click(1000);
  t.tick(1000 + kMin);
  TEST_ASSERT_EQUAL_UINT32(24 * kMin, t.remainingMs());
}

void test_minutes_left_is_ceiling() {
  TimerModel t;
  t.turn(-24);  // 1 minute
  t.click(0);
  t.tick(1);
  TEST_ASSERT_EQUAL_UINT32(59999, t.remainingMs());
  TEST_ASSERT_EQUAL(1, t.minutesLeft());
  t.tick(0);
  TEST_ASSERT_EQUAL(1, t.minutesLeft());  // exactly 60000 -> 1
  TimerModel u;
  u.turn(-23);  // 2 minutes
  u.click(0);
  u.tick(59999);
  TEST_ASSERT_EQUAL_UINT32(60001, u.remainingMs());
  TEST_ASSERT_EQUAL(2, u.minutesLeft());
}

void test_done_at_zero() {
  TimerModel t;
  t.click(0);
  t.tick(25 * kMin);
  TEST_ASSERT_TRUE(t.state() == State::Done);
  TEST_ASSERT_EQUAL_UINT32(0, t.remainingMs());
}

void test_pause_freezes_remaining() {
  TimerModel t;
  t.click(0);
  t.click(kMin);  // pause after one minute
  TEST_ASSERT_TRUE(t.state() == State::Paused);
  t.tick(11 * kMin);
  TEST_ASSERT_EQUAL_UINT32(24 * kMin, t.remainingMs());
  t.click(11 * kMin);  // resume
  TEST_ASSERT_TRUE(t.state() == State::Running);
  t.tick(12 * kMin);
  TEST_ASSERT_EQUAL_UINT32(23 * kMin, t.remainingMs());
}

void test_cancel_returns_to_setting_keeping_minutes() {
  TimerModel t;
  t.turn(5);
  t.click(0);
  t.cancel();
  TEST_ASSERT_TRUE(t.state() == State::Setting);
  TEST_ASSERT_EQUAL(30, t.setMinutes());
}

void test_click_in_done_returns_to_setting() {
  TimerModel t;
  t.click(0);
  t.tick(25 * kMin);
  t.click(25 * kMin + 5);
  TEST_ASSERT_TRUE(t.state() == State::Setting);
}

void test_last_minute() {
  TimerModel t;
  t.turn(-24);  // 1 minute
  t.click(0);
  TEST_ASSERT_TRUE(t.inLastMinute());  // remaining == 60000
  t.tick(kMin - 1);
  TEST_ASSERT_TRUE(t.inLastMinute());  // remaining == 1
  TimerModel u;
  u.turn(-23);  // 2 minutes
  u.click(0);
  u.tick(59999);
  TEST_ASSERT_FALSE(u.inLastMinute());  // remaining == 60001
  t.tick(kMin);
  TEST_ASSERT_FALSE(t.inLastMinute());  // Done, remaining == 0
}

void test_millis_rollover() {
  TimerModel t;
  t.click(0xFFFFFC00u);
  t.tick(0x000003E8u);  // 1024 ms to the wrap + 1000 ms after it
  TEST_ASSERT_EQUAL_UINT32(t.totalMs() - 2024, t.remainingMs());
}

static TimerModel at(int minutes) {
  TimerModel t;
  t.turn(minutes - 25);
  return t;
}

void test_step_up_snaps_to_next_multiple() {
  TimerModel a = at(26);
  a.turnToStep(+1, 5);
  TEST_ASSERT_EQUAL(30, a.setMinutes());
  a.turnToStep(+1, 5);
  TEST_ASSERT_EQUAL(35, a.setMinutes());
}

void test_step_down_snaps_to_previous_multiple() {
  TimerModel a = at(26);
  a.turnToStep(-1, 5);
  TEST_ASSERT_EQUAL(25, a.setMinutes());
  a.turnToStep(-1, 5);
  TEST_ASSERT_EQUAL(20, a.setMinutes());
}

void test_step_clamps_at_both_ends() {
  TimerModel hi = at(178);
  hi.turnToStep(+1, 5);
  TEST_ASSERT_EQUAL(180, hi.setMinutes());
  hi.turnToStep(+1, 5);
  TEST_ASSERT_EQUAL(180, hi.setMinutes());
  TimerModel lo = at(3);
  lo.turnToStep(-1, 5);
  TEST_ASSERT_EQUAL(1, lo.setMinutes());
  lo.turnToStep(-1, 5);
  TEST_ASSERT_EQUAL(1, lo.setMinutes());
}

void test_step_updates_total_and_is_ignored_when_running() {
  TimerModel a = at(26);
  a.turnToStep(+1, 5);
  TEST_ASSERT_EQUAL_UINT32(30 * kMin, a.totalMs());
  a.click(0);
  a.turnToStep(+1, 5);
  TEST_ASSERT_EQUAL(30, a.setMinutes());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_defaults);
  RUN_TEST(test_turn_clamps);
  RUN_TEST(test_turn_ignored_when_running);
  RUN_TEST(test_click_starts);
  RUN_TEST(test_remaining_counts_down);
  RUN_TEST(test_minutes_left_is_ceiling);
  RUN_TEST(test_done_at_zero);
  RUN_TEST(test_pause_freezes_remaining);
  RUN_TEST(test_cancel_returns_to_setting_keeping_minutes);
  RUN_TEST(test_click_in_done_returns_to_setting);
  RUN_TEST(test_last_minute);
  RUN_TEST(test_millis_rollover);
  RUN_TEST(test_step_up_snaps_to_next_multiple);
  RUN_TEST(test_step_down_snaps_to_previous_multiple);
  RUN_TEST(test_step_clamps_at_both_ends);
  RUN_TEST(test_step_updates_total_and_is_ignored_when_running);
  return UNITY_END();
}
