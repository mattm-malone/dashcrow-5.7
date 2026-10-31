#include <unity.h>
#include "SettleGate.h"

void setUp() {}
void tearDown() {}

void test_ready_when_nothing_pending() {
  SettleGate g;
  TEST_ASSERT_TRUE(g.ready(0));
  TEST_ASSERT_TRUE(g.ready(123456));
}

void test_single_input_waits_for_quiet() {
  SettleGate g;
  g.input(1000);
  TEST_ASSERT_FALSE(g.ready(1000));
  TEST_ASSERT_FALSE(g.ready(1000 + SettleGate::kQuietMs - 1));
  TEST_ASSERT_TRUE(g.ready(1000 + SettleGate::kQuietMs));
}

void test_burst_of_clicks_is_batched() {
  SettleGate g;
  g.input(1000);
  g.input(1020);
  g.input(1040);
  TEST_ASSERT_FALSE(g.ready(1040 + SettleGate::kQuietMs - 1));  // quiet is measured from the last click
  TEST_ASSERT_TRUE(g.ready(1040 + SettleGate::kQuietMs));
}

void test_long_burst_is_cut_off_at_max_delay() {
  SettleGate g;
  g.input(1000);
  g.input(1100);
  g.input(1200);
  TEST_ASSERT_FALSE(g.ready(1000 + SettleGate::kMaxDelayMs - 1));
  TEST_ASSERT_TRUE(g.ready(1000 + SettleGate::kMaxDelayMs));  // 300 ms after the first, though the last click was 100 ms ago
}

void test_held_key_still_redraws_within_max_delay() {
  SettleGate g;
  uint32_t t = 1000;
  g.input(t);
  // A repeat every 120 ms never lets the quiet period elapse; the max delay must still release a redraw.
  bool released = false;
  for (int i = 0; i < 10 && !released; i++) {
    t += 120;
    g.input(t);
    released = g.ready(t);
  }
  TEST_ASSERT_TRUE(released);
  TEST_ASSERT_LESS_OR_EQUAL_UINT32(1000 + SettleGate::kMaxDelayMs + 120, t);
}

void test_refreshed_clears_pending_and_restarts_the_window() {
  SettleGate g;
  g.input(1000);
  g.refreshed();
  TEST_ASSERT_TRUE(g.ready(1001));  // nothing pending now
  g.input(1500);                    // the next burst gets its own full window
  TEST_ASSERT_FALSE(g.ready(1500 + SettleGate::kQuietMs - 1));
}

void test_millis_rollover() {
  SettleGate g;
  g.input(0xFFFFFF00u);
  TEST_ASSERT_FALSE(g.ready(0xFFFFFF00u + 100));
  TEST_ASSERT_TRUE(g.ready(0xFFFFFF00u + SettleGate::kQuietMs));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_ready_when_nothing_pending);
  RUN_TEST(test_single_input_waits_for_quiet);
  RUN_TEST(test_burst_of_clicks_is_batched);
  RUN_TEST(test_long_burst_is_cut_off_at_max_delay);
  RUN_TEST(test_held_key_still_redraws_within_max_delay);
  RUN_TEST(test_refreshed_clears_pending_and_restarts_the_window);
  RUN_TEST(test_millis_rollover);
  return UNITY_END();
}
