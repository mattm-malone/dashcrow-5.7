#include <stdlib.h>
#include <unity.h>
#include "LocalTimeUtil.h"

// US Eastern, the rule used in src/config.h. DST ends Sun 2026-11-01 02:00 EDT -> 01:00 EST,
// and began Sun 2026-03-08 02:00 EST -> 03:00 EDT.
constexpr time_t kOct31_2330_EDT = 1793503800;  // 2026-11-01 03:30Z
constexpr time_t kNov1_0030_EDT = 1793507400;
constexpr time_t kNov1_0130_EDT = 1793511000;   // first 01:30
constexpr time_t kNov1_0130_EST = 1793514600;   // second 01:30
constexpr time_t kNov1_2330_EST = 1793593800;
constexpr time_t kNov1_noon_EST = 1793552400;
constexpr time_t kOct31_noon_EDT = 1793462400;
constexpr time_t kMar8_0130_EST = 1772951400;
constexpr time_t kMar8_0330_EDT = 1772955000;
constexpr time_t kMar9_0030_EDT = 1773030600;

void setUp() {
  setenv("TZ", "EST5EDT,M3.2.0,M11.1.0", 1);
  tzset();
}
void tearDown() {}

void test_unsynced_clock_is_invalid() {
  TEST_ASSERT_FALSE(localTimeFrom(0).valid);
  TEST_ASSERT_FALSE(localTimeFrom(12345).valid);
  TEST_ASSERT_TRUE(localTimeFrom(kNov1_noon_EST).valid);
}

void test_fall_back_repeats_the_one_o_clock_hour() {
  const LocalTime a = localTimeFrom(kNov1_0130_EDT);
  const LocalTime b = localTimeFrom(kNov1_0130_EST);
  TEST_ASSERT_EQUAL(1, a.hour);
  TEST_ASSERT_EQUAL(30, a.minute);
  TEST_ASSERT_EQUAL(1, b.hour);  // same wall-clock reading an hour later
  TEST_ASSERT_EQUAL(30, b.minute);
  TEST_ASSERT_EQUAL_UINT32(3600, b.epoch - a.epoch);  // but real time kept moving forward
  TEST_ASSERT_EQUAL(0, a.wday);  // Sunday
}

void test_spring_forward_skips_two_o_clock() {
  TEST_ASSERT_EQUAL(1, localTimeFrom(kMar8_0130_EST).hour);
  TEST_ASSERT_EQUAL(3, localTimeFrom(kMar8_0330_EDT).hour);
  TEST_ASSERT_EQUAL_UINT32(3600, localTimeFrom(kMar8_0330_EDT).epoch - localTimeFrom(kMar8_0130_EST).epoch);  // wall clock jumped 2 h, real time 1 h
}

void test_offset_zero_is_today() {
  const DayInfo d = dayInfoFor(kNov1_noon_EST, 0);
  TEST_ASSERT_EQUAL(1, d.mday);
  TEST_ASSERT_EQUAL(11, d.month);
  TEST_ASSERT_EQUAL(0, d.wday);
  TEST_ASSERT_EQUAL(304, d.yday);
}

void test_next_day_across_fall_back() {
  // 23:30 EDT on Oct 31 and 00:30 EDT on Nov 1: +1 day must be Nov 1 and Nov 2.
  const DayInfo a = dayInfoFor(kOct31_2330_EDT, 1);
  TEST_ASSERT_EQUAL(1, a.mday);
  TEST_ASSERT_EQUAL(11, a.month);
  const DayInfo b = dayInfoFor(kNov1_0030_EDT, 1);  // adding 24 h would land on 23:30 EST Nov 1 (the same day)
  TEST_ASSERT_EQUAL(2, b.mday);
  TEST_ASSERT_EQUAL(11, b.month);
}

void test_previous_day_across_fall_back() {
  // 23:30 EST on Nov 1: -1 day must be Oct 31 (subtracting 24 h would give 00:30 EDT on Nov 1).
  const DayInfo a = dayInfoFor(kNov1_2330_EST, -1);
  TEST_ASSERT_EQUAL(31, a.mday);
  TEST_ASSERT_EQUAL(10, a.month);
}

void test_previous_day_across_spring_forward() {
  const DayInfo a = dayInfoFor(kMar9_0030_EDT, -1);  // 24 h earlier is 23:30 EST on Mar 7
  TEST_ASSERT_EQUAL(8, a.mday);
  TEST_ASSERT_EQUAL(3, a.month);
}

void test_tz_hours_follow_the_target_day() {
  TEST_ASSERT_TRUE(dayInfoFor(kOct31_noon_EDT, 0).tzHours == -4.0);
  TEST_ASSERT_TRUE(dayInfoFor(kNov1_noon_EST, 0).tzHours == -5.0);
  TEST_ASSERT_TRUE(dayInfoFor(kOct31_noon_EDT, 1).tzHours == -5.0);  // looking ahead to the EST day
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_unsynced_clock_is_invalid);
  RUN_TEST(test_fall_back_repeats_the_one_o_clock_hour);
  RUN_TEST(test_spring_forward_skips_two_o_clock);
  RUN_TEST(test_offset_zero_is_today);
  RUN_TEST(test_next_day_across_fall_back);
  RUN_TEST(test_previous_day_across_fall_back);
  RUN_TEST(test_previous_day_across_spring_forward);
  RUN_TEST(test_tz_hours_follow_the_target_day);
  return UNITY_END();
}
