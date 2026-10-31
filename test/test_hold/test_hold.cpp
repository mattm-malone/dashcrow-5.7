#include <unity.h>
#include "HoldTracker.h"

using Event = HoldTracker::Event;

void setUp() {}
void tearDown() {}

void test_idle_reports_nothing() {
  HoldTracker h;
  TEST_ASSERT_TRUE(h.update(1000, false) == Event::None);
  TEST_ASSERT_TRUE(h.update(2000, true) == Event::None);  // a level with no press edge is ignored
}

void test_short_press_reports_short_after_release() {
  HoldTracker h;
  h.press(1000);
  TEST_ASSERT_TRUE(h.update(1050, true) == Event::None);   // still down: undecided
  TEST_ASSERT_TRUE(h.update(1100, false) == Event::None);  // just released: wait out the chatter window
  TEST_ASSERT_TRUE(h.update(1100 + HoldTracker::kReleaseDebounceMs, false) == Event::Short);
  TEST_ASSERT_FALSE(h.active());
  TEST_ASSERT_TRUE(h.update(5000, false) == Event::None);  // reported once
}

void test_hold_reports_long_once_at_the_threshold_with_no_short_after() {
  HoldTracker h;
  h.press(1000);
  TEST_ASSERT_TRUE(h.update(1000 + HoldTracker::kHoldMs - 1, true) == Event::None);
  TEST_ASSERT_TRUE(h.update(1000 + HoldTracker::kHoldMs, true) == Event::Long);
  TEST_ASSERT_TRUE(h.update(1000 + HoldTracker::kHoldMs + 500, true) == Event::None);  // not again while held
  TEST_ASSERT_TRUE(h.update(3000, false) == Event::None);
  TEST_ASSERT_TRUE(h.update(3100, false) == Event::None);  // the release after a hold is silent
  TEST_ASSERT_FALSE(h.active());
}

void test_brief_contact_chatter_does_not_end_a_press() {
  HoldTracker h;
  h.press(1000);
  TEST_ASSERT_TRUE(h.update(1200, false) == Event::None);  // contact bounces open for a few ms...
  TEST_ASSERT_TRUE(h.update(1210, true) == Event::None);   // ...and closes again
  TEST_ASSERT_TRUE(h.active());
  TEST_ASSERT_TRUE(h.update(1000 + HoldTracker::kHoldMs, true) == Event::Long);  // so the hold still completes
}

void test_late_poll_still_reports_the_long_hold() {
  HoldTracker h;
  h.press(1000);
  // The loop was blocked in a screen refresh and polls well past the threshold with the key still down.
  TEST_ASSERT_TRUE(h.update(1000 + 3000, true) == Event::Long);
}

void test_a_new_press_after_a_report_starts_fresh() {
  HoldTracker h;
  h.press(0);
  h.update(100, false);
  TEST_ASSERT_TRUE(h.update(100 + HoldTracker::kReleaseDebounceMs, false) == Event::Short);
  h.press(5000);
  TEST_ASSERT_TRUE(h.update(5000 + HoldTracker::kHoldMs, true) == Event::Long);
}

void test_millis_rollover() {
  HoldTracker h;
  h.press(0xFFFFFF00u);
  TEST_ASSERT_TRUE(h.update(0xFFFFFF00u + HoldTracker::kHoldMs, true) == Event::Long);  // wraps past zero
}

void test_remembers_when_the_press_happened() {
  HoldTracker h;
  h.press(4242);
  TEST_ASSERT_EQUAL_UINT32(4242, h.pressedAt());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_idle_reports_nothing);
  RUN_TEST(test_short_press_reports_short_after_release);
  RUN_TEST(test_hold_reports_long_once_at_the_threshold_with_no_short_after);
  RUN_TEST(test_brief_contact_chatter_does_not_end_a_press);
  RUN_TEST(test_late_poll_still_reports_the_long_hold);
  RUN_TEST(test_a_new_press_after_a_report_starts_fresh);
  RUN_TEST(test_millis_rollover);
  RUN_TEST(test_remembers_when_the_press_happened);
  return UNITY_END();
}
