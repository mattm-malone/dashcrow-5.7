#include <unity.h>
#include "Daylight.h"

void setUp() {}
void tearDown() {}

// Hand-checked against the NOAA equations: equator, prime meridian, 21 March (day 80):
// sunrise ~364.5 min, sunset ~1091.2 min (equation of time is about -8 min).
void test_equator_equinox() {
  const SunTimes s = sunTimes(80, 0.0, 0.0, 0.0);
  TEST_ASSERT_FALSE(s.polarDay);
  TEST_ASSERT_FALSE(s.polarNight);
  TEST_ASSERT_INT_WITHIN(6, 364, s.sunriseMin);
  TEST_ASSERT_INT_WITHIN(6, 1091, s.sunsetMin);
  TEST_ASSERT_INT_WITHIN(5, 727, int(s.sunsetMin) - int(s.sunriseMin));
}

void test_timezone_shifts_both_times() {
  const SunTimes a = sunTimes(80, 0.0, 0.0, 0.0);
  const SunTimes b = sunTimes(80, 0.0, 0.0, 2.0);
  TEST_ASSERT_EQUAL(a.sunriseMin + 120, b.sunriseMin);
  TEST_ASSERT_EQUAL(a.sunsetMin + 120, b.sunsetMin);
}

void test_polar_day() { TEST_ASSERT_TRUE(sunTimes(172, 80.0, 0.0, 0.0).polarDay); }
void test_polar_night() { TEST_ASSERT_TRUE(sunTimes(355, 80.0, 0.0, 0.0).polarNight); }

void test_longer_day_in_summer_at_mid_latitude() {
  const SunTimes summer = sunTimes(172, 45.0, 0.0, 0.0);
  const SunTimes winter = sunTimes(355, 45.0, 0.0, 0.0);
  TEST_ASSERT_GREATER_THAN(int(winter.sunsetMin) - int(winter.sunriseMin), int(summer.sunsetMin) - int(summer.sunriseMin));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_equator_equinox);
  RUN_TEST(test_timezone_shifts_both_times);
  RUN_TEST(test_polar_day);
  RUN_TEST(test_polar_night);
  RUN_TEST(test_longer_day_in_summer_at_mid_latitude);
  return UNITY_END();
}
