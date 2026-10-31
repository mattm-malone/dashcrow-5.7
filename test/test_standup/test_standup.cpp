#include <unity.h>
#include "StandupScheduler.h"

void setUp() {}
void tearDown() {}

static LocalTime mk(uint8_t wday, uint8_t h, uint8_t m, uint8_t s = 0) {
  return {true, uint32_t(wday) * 86400u + h * 3600u + m * 60u + s, wday, h, m, s};
}

void test_first_due_one_interval_after_first_valid_time() {
  StandupScheduler s;
  TEST_ASSERT_FALSE(s.due(mk(1, 9, 30)));
  TEST_ASSERT_FALSE(s.due(mk(1, 10, 29, 59)));
  TEST_ASSERT_TRUE(s.due(mk(1, 10, 30)));
}

void test_ack_restarts_countdown() {
  StandupScheduler s;
  s.due(mk(1, 9, 30));
  TEST_ASSERT_TRUE(s.due(mk(1, 10, 30)));
  s.acknowledge(mk(1, 10, 30));
  TEST_ASSERT_FALSE(s.due(mk(1, 11, 29, 59)));
  TEST_ASSERT_TRUE(s.due(mk(1, 11, 30)));
}

void test_not_due_on_weekend() {
  StandupScheduler s;
  s.due(mk(6, 9, 30));
  TEST_ASSERT_FALSE(s.due(mk(6, 12, 0)));
}

void test_not_due_after_end_hour() {
  StandupScheduler s;
  s.due(mk(1, 9, 30));
  TEST_ASSERT_FALSE(s.due(mk(1, 17, 0)));
}

void test_window_open_resets_countdown() {
  StandupScheduler s;
  s.due(mk(1, 9, 30));
  s.acknowledge(mk(1, 16, 30));
  TEST_ASSERT_FALSE(s.due(mk(2, 9, 0)));
  TEST_ASSERT_TRUE(s.due(mk(2, 10, 0)));
}

void test_invalid_time_never_due() {
  StandupScheduler s;
  LocalTime bad{false, 0, 1, 12, 0, 0};
  TEST_ASSERT_FALSE(s.due(bad));
  // The invalid call must not have started a countdown: first valid sighting is 09:30.
  TEST_ASSERT_FALSE(s.due(mk(1, 9, 30)));
  TEST_ASSERT_FALSE(s.due(mk(1, 10, 29, 59)));
  TEST_ASSERT_TRUE(s.due(mk(1, 10, 30)));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_first_due_one_interval_after_first_valid_time);
  RUN_TEST(test_ack_restarts_countdown);
  RUN_TEST(test_not_due_on_weekend);
  RUN_TEST(test_not_due_after_end_hour);
  RUN_TEST(test_window_open_resets_countdown);
  RUN_TEST(test_invalid_time_never_due);
  return UNITY_END();
}
