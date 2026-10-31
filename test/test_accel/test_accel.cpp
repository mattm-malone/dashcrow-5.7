#include <unity.h>
#include "Accel.h"

void setUp() {}
void tearDown() {}

void test_first_step_is_one() {
  RotaryAccel a;
  TEST_ASSERT_EQUAL(1, a.step(1, 1000));
  RotaryAccel b;
  TEST_ASSERT_EQUAL(-1, b.step(-1, 1000));
}

void test_fast_repeat_steps_by_five() {
  RotaryAccel a;
  a.step(1, 1000);
  TEST_ASSERT_EQUAL(5, a.step(1, 1100));
  TEST_ASSERT_EQUAL(5, a.step(1, 1250));
}

void test_slow_repeat_steps_by_one() {
  RotaryAccel a;
  a.step(1, 1000);
  TEST_ASSERT_EQUAL(1, a.step(1, 1151));
}

void test_direction_change_resets() {
  RotaryAccel a;
  a.step(1, 1000);
  a.step(1, 1050);
  TEST_ASSERT_EQUAL(-1, a.step(-1, 1100));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_first_step_is_one);
  RUN_TEST(test_fast_repeat_steps_by_five);
  RUN_TEST(test_slow_repeat_steps_by_one);
  RUN_TEST(test_direction_change_resets);
  return UNITY_END();
}
