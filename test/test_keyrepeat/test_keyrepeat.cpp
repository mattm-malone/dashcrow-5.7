#include <unity.h>
#include "KeyRepeat.h"

void setUp() {}
void tearDown() {}

void test_no_repeat_before_delay() {
  KeyRepeat k;
  k.press(1000);
  uint32_t at;
  TEST_ASSERT_FALSE(k.next(1000 + KeyRepeat::kDelayMs - 1, at));
}

void test_first_repeat_at_delay_then_every_interval() {
  KeyRepeat k;
  k.press(1000);
  uint32_t at = 0;
  TEST_ASSERT_TRUE(k.next(1000 + KeyRepeat::kDelayMs, at));
  TEST_ASSERT_EQUAL_UINT32(1000 + KeyRepeat::kDelayMs, at);
  TEST_ASSERT_FALSE(k.next(1000 + KeyRepeat::kDelayMs + KeyRepeat::kIntervalMs - 1, at));
  TEST_ASSERT_TRUE(k.next(1000 + KeyRepeat::kDelayMs + KeyRepeat::kIntervalMs, at));
  TEST_ASSERT_EQUAL_UINT32(1000 + KeyRepeat::kDelayMs + KeyRepeat::kIntervalMs, at);
}

void test_late_poll_drains_every_owed_repeat_with_its_own_time() {
  KeyRepeat k;
  k.press(0);
  uint32_t at = 0;
  int n = 0;
  uint32_t last = 0;
  const uint32_t now = KeyRepeat::kDelayMs + 3 * KeyRepeat::kIntervalMs + 5;  // polled late, e.g. after a refresh
  while (k.next(now, at)) {
    n++;
    last = at;
  }
  TEST_ASSERT_EQUAL(4, n);
  TEST_ASSERT_EQUAL_UINT32(KeyRepeat::kDelayMs + 3 * KeyRepeat::kIntervalMs, last);
}

void test_release_stops_repeats() {
  KeyRepeat k;
  k.press(0);
  k.release();
  uint32_t at;
  TEST_ASSERT_FALSE(k.active());
  TEST_ASSERT_FALSE(k.next(5000, at));
}

void test_new_press_restarts_the_delay() {
  KeyRepeat k;
  k.press(0);
  uint32_t at;
  k.next(KeyRepeat::kDelayMs, at);
  k.press(10000);
  TEST_ASSERT_FALSE(k.next(10000 + KeyRepeat::kDelayMs - 1, at));
  TEST_ASSERT_TRUE(k.next(10000 + KeyRepeat::kDelayMs, at));
}

void test_millis_rollover() {
  KeyRepeat k;
  k.press(0xFFFFFF00u);
  uint32_t at;
  TEST_ASSERT_FALSE(k.next(0xFFFFFF00u + 100, at));
  TEST_ASSERT_TRUE(k.next(0xFFFFFF00u + KeyRepeat::kDelayMs, at));  // wraps past zero
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_no_repeat_before_delay);
  RUN_TEST(test_first_repeat_at_delay_then_every_interval);
  RUN_TEST(test_late_poll_drains_every_owed_repeat_with_its_own_time);
  RUN_TEST(test_release_stops_repeats);
  RUN_TEST(test_new_press_restarts_the_delay);
  RUN_TEST(test_millis_rollover);
  return UNITY_END();
}
