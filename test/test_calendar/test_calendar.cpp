#include <unity.h>
#include <string.h>
#include "Calendar.h"

void setUp() {}
void tearDown() {}

constexpr uint8_t kMon = 1 << 1, kTue = 1 << 2;
constexpr uint8_t kWeekdays = 0b0111110;

void test_weekday_filter() {
  const Meeting m[] = {{"Standup", kMon, 540, 555}};
  DayEvent out[4];
  TEST_ASSERT_EQUAL(1, eventsForDay(m, 1, 1, out, 4));
  TEST_ASSERT_EQUAL(0, eventsForDay(m, 1, 2, out, 4));
}

void test_sorted_by_start() {
  const Meeting m[] = {{"Late", kWeekdays, 900, 960}, {"Early", kWeekdays, 540, 600}, {"Mid", kWeekdays, 700, 760}};
  DayEvent out[4];
  TEST_ASSERT_EQUAL(3, eventsForDay(m, 3, 3, out, 4));
  TEST_ASSERT_EQUAL_STRING("Early", out[0].title);
  TEST_ASSERT_EQUAL_STRING("Mid", out[1].title);
  TEST_ASSERT_EQUAL_STRING("Late", out[2].title);
}

void test_zero_length_dropped() {
  const Meeting m[] = {{"Zero", kWeekdays, 600, 600}, {"Backwards", kWeekdays, 700, 650}};
  DayEvent out[4];
  TEST_ASSERT_EQUAL(0, eventsForDay(m, 2, 3, out, 4));
}

void test_crosses_midnight_is_clipped() {
  const Meeting m[] = {{"Night", kWeekdays, 22 * 60, 2 * 60}};
  DayEvent out[2];
  // end (02:00) is before start, so it is dropped rather than wrapped into the next day...
  TEST_ASSERT_EQUAL(0, eventsForDay(m, 1, 3, out, 2));
  // ...while an end past 24:00 is clipped to 24:00.
  const Meeting big[] = {{"Long", kWeekdays, 22 * 60, 1500}};
  TEST_ASSERT_EQUAL(1, eventsForDay(big, 1, 3, out, 2));
  TEST_ASSERT_EQUAL(1440, out[0].endMin);
}

void test_cap_respected_keeps_earliest() {
  const Meeting m[] = {{"C", kWeekdays, 900, 930}, {"A", kWeekdays, 540, 570}, {"B", kWeekdays, 700, 730}};
  DayEvent out[2];
  TEST_ASSERT_EQUAL(2, eventsForDay(m, 3, 3, out, 2));
  TEST_ASSERT_EQUAL_STRING("A", out[0].title);
  TEST_ASSERT_EQUAL_STRING("B", out[1].title);
}

void test_long_title_truncated_and_terminated() {
  const Meeting m[] = {{"0123456789012345678901234567890123456789", kWeekdays, 540, 600}};
  DayEvent out[1];
  eventsForDay(m, 1, 3, out, 1);
  TEST_ASSERT_EQUAL(31, strlen(out[0].title));
}

void test_overlap_kept_both() {
  const Meeting m[] = {{"A", kWeekdays, 600, 700}, {"B", kWeekdays, 650, 750}};
  DayEvent out[4];
  TEST_ASSERT_EQUAL(2, eventsForDay(m, 2, 3, out, 4));
}

static size_t sample(DayEvent *out) {
  const Meeting m[] = {{"First", kWeekdays, 540, 600}, {"Second", kWeekdays, 700, 760}};
  return eventsForDay(m, 2, 3, out, 4);
}

void test_next_event_before_first() {
  DayEvent e[4];
  const size_t n = sample(e);
  const NextInfo i = nextEvent(e, n, 480);
  TEST_ASSERT_EQUAL(0, i.index);
  TEST_ASSERT_FALSE(i.inProgress);
  TEST_ASSERT_EQUAL(60, i.minutesUntil);
}

void test_next_event_in_progress() {
  DayEvent e[4];
  const size_t n = sample(e);
  const NextInfo i = nextEvent(e, n, 560);
  TEST_ASSERT_EQUAL(0, i.index);
  TEST_ASSERT_TRUE(i.inProgress);
  TEST_ASSERT_EQUAL(0, i.minutesUntil);
}

void test_next_event_none_after_last() {
  DayEvent e[4];
  const size_t n = sample(e);
  TEST_ASSERT_EQUAL(-1, nextEvent(e, n, 800).index);
  TEST_ASSERT_EQUAL(-1, nextEvent(e, 0, 100).index);
}

void test_minute_to_x() {
  const DayWindow w{480, 1140};
  TEST_ASSERT_EQUAL(120, minuteToX(600, w, 660));
  TEST_ASSERT_EQUAL(0, minuteToX(400, w, 660));
  TEST_ASSERT_EQUAL(660, minuteToX(1200, w, 660));
  TEST_ASSERT_EQUAL(0, minuteToX(600, DayWindow{600, 600}, 660));  // degenerate window
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_weekday_filter);
  RUN_TEST(test_sorted_by_start);
  RUN_TEST(test_zero_length_dropped);
  RUN_TEST(test_crosses_midnight_is_clipped);
  RUN_TEST(test_cap_respected_keeps_earliest);
  RUN_TEST(test_long_title_truncated_and_terminated);
  RUN_TEST(test_overlap_kept_both);
  RUN_TEST(test_next_event_before_first);
  RUN_TEST(test_next_event_in_progress);
  RUN_TEST(test_next_event_none_after_last);
  RUN_TEST(test_minute_to_x);
  return UNITY_END();
}
