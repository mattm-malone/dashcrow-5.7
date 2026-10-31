#include <unity.h>
#include <initializer_list>
#include <string.h>
#include "Calendar.h"

void setUp() {}
void tearDown() {}

constexpr uint8_t kMon = 1 << 1, kTue = 1 << 2;
constexpr uint8_t kWeekdays = 0b0111110;

// Any date with the right weekday: 2026-11-01 is a Sunday, so 1 + wday keeps date and weekday consistent.
static DayDate dayOf(uint8_t wday) { return {2026, 11, uint8_t(1 + wday), wday}; }

// The weekday of a date, from the day count alone (1970-01-01 was a Thursday).
static DayDate date(int y, int m, int d) {
  const int32_t n = daysFromCivil(y, m, d);
  return {uint16_t(y), uint8_t(m), uint8_t(d), uint8_t(((n % 7) + 7 + 4) % 7)};
}

static bool occurs(const Meeting &m, const DayDate &day) {
  DayEvent out[2];
  return eventsForDay(&m, 1, day, out, 2) == 1;
}

void test_weekday_filter() {
  const Meeting m[] = {{"Standup", kMon, 540, 555}};
  DayEvent out[4];
  TEST_ASSERT_EQUAL(1, eventsForDay(m, 1, dayOf(1), out, 4));
  TEST_ASSERT_EQUAL(0, eventsForDay(m, 1, dayOf(2), out, 4));
}

void test_sorted_by_start() {
  const Meeting m[] = {{"Late", kWeekdays, 900, 960}, {"Early", kWeekdays, 540, 600}, {"Mid", kWeekdays, 700, 760}};
  DayEvent out[4];
  TEST_ASSERT_EQUAL(3, eventsForDay(m, 3, dayOf(3), out, 4));
  TEST_ASSERT_EQUAL_STRING("Early", out[0].title);
  TEST_ASSERT_EQUAL_STRING("Mid", out[1].title);
  TEST_ASSERT_EQUAL_STRING("Late", out[2].title);
}

void test_zero_length_dropped() {
  const Meeting m[] = {{"Zero", kWeekdays, 600, 600}, {"Backwards", kWeekdays, 700, 650}};
  DayEvent out[4];
  TEST_ASSERT_EQUAL(0, eventsForDay(m, 2, dayOf(3), out, 4));
}

void test_crosses_midnight_is_clipped() {
  const Meeting m[] = {{"Night", kWeekdays, 22 * 60, 2 * 60}};
  DayEvent out[2];
  // end (02:00) is before start, so it is dropped rather than wrapped into the next day...
  TEST_ASSERT_EQUAL(0, eventsForDay(m, 1, dayOf(3), out, 2));
  // ...while an end past 24:00 is clipped to 24:00.
  const Meeting big[] = {{"Long", kWeekdays, 22 * 60, 1500}};
  TEST_ASSERT_EQUAL(1, eventsForDay(big, 1, dayOf(3), out, 2));
  TEST_ASSERT_EQUAL(1440, out[0].endMin);
}

void test_cap_respected_keeps_earliest() {
  const Meeting m[] = {{"C", kWeekdays, 900, 930}, {"A", kWeekdays, 540, 570}, {"B", kWeekdays, 700, 730}};
  DayEvent out[2];
  TEST_ASSERT_EQUAL(2, eventsForDay(m, 3, dayOf(3), out, 2));
  TEST_ASSERT_EQUAL_STRING("A", out[0].title);
  TEST_ASSERT_EQUAL_STRING("B", out[1].title);
}

void test_long_title_truncated_and_terminated() {
  const Meeting m[] = {{"0123456789012345678901234567890123456789", kWeekdays, 540, 600}};
  DayEvent out[1];
  eventsForDay(m, 1, dayOf(3), out, 1);
  TEST_ASSERT_EQUAL(31, strlen(out[0].title));
}

void test_overlap_kept_both() {
  const Meeting m[] = {{"A", kWeekdays, 600, 700}, {"B", kWeekdays, 650, 750}};
  DayEvent out[4];
  TEST_ASSERT_EQUAL(2, eventsForDay(m, 2, dayOf(3), out, 4));
}

static size_t sample(DayEvent *out) {
  const Meeting m[] = {{"First", kWeekdays, 540, 600}, {"Second", kWeekdays, 700, 760}};
  return eventsForDay(m, 2, dayOf(3), out, 4);
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

void test_days_from_civil_reference_values() {
  TEST_ASSERT_EQUAL(0, daysFromCivil(1970, 1, 1));
  TEST_ASSERT_EQUAL(20758, daysFromCivil(2026, 11, 1));
  TEST_ASSERT_EQUAL(1, daysFromCivil(2024, 3, 1) - daysFromCivil(2024, 2, 29));  // leap day
  TEST_ASSERT_EQUAL(365, daysFromCivil(2025, 3, 1) - daysFromCivil(2024, 3, 1));
  TEST_ASSERT_EQUAL(366, daysFromCivil(2024, 3, 1) - daysFromCivil(2023, 3, 1));  // spans the leap day
  TEST_ASSERT_EQUAL(11017, daysFromCivil(2000, 3, 1));
}

// Every other Tuesday, anchored on Tue 2026-11-03.
static const Meeting kBiweekly = {"Sprint review", kTue, 840, 900, 2, 2026, 11, 3};

void test_biweekly_alternates_weeks() {
  TEST_ASSERT_TRUE(occurs(kBiweekly, date(2026, 11, 3)));    // anchor
  TEST_ASSERT_FALSE(occurs(kBiweekly, date(2026, 11, 10)));
  TEST_ASSERT_TRUE(occurs(kBiweekly, date(2026, 11, 17)));
  TEST_ASSERT_FALSE(occurs(kBiweekly, date(2026, 11, 24)));
}

void test_biweekly_across_month_and_year_boundaries() {
  TEST_ASSERT_TRUE(occurs(kBiweekly, date(2026, 12, 29)));   // 8 weeks after the anchor
  TEST_ASSERT_FALSE(occurs(kBiweekly, date(2027, 1, 5)));
  TEST_ASSERT_TRUE(occurs(kBiweekly, date(2027, 1, 12)));
  TEST_ASSERT_FALSE(occurs(kBiweekly, date(2027, 3, 2)));    // 17 weeks after the anchor: an off week
  TEST_ASSERT_TRUE(occurs(kBiweekly, date(2027, 3, 9)));     // 18 weeks, past February (not a leap year)
}

void test_biweekly_before_the_anchor() {
  TEST_ASSERT_FALSE(occurs(kBiweekly, date(2026, 10, 27)));  // one week before
  TEST_ASSERT_TRUE(occurs(kBiweekly, date(2026, 10, 20)));   // two weeks before
  TEST_ASSERT_TRUE(occurs(kBiweekly, date(2025, 11, 4)));    // a year (52 weeks) earlier
}

void test_biweekly_never_on_other_weekdays() {
  TEST_ASSERT_FALSE(occurs(kBiweekly, date(2026, 11, 4)));   // Wednesday of an "on" week
  TEST_ASSERT_FALSE(occurs(kBiweekly, date(2026, 11, 2)));   // Monday of an "on" week
}

void test_biweekly_with_several_weekdays_keeps_the_week_together() {
  const Meeting m = {"Pair", uint8_t(kTue | (1 << 4)), 600, 660, 2, 2026, 11, 3};  // Tue + Thu
  TEST_ASSERT_TRUE(occurs(m, date(2026, 11, 3)));
  TEST_ASSERT_TRUE(occurs(m, date(2026, 11, 5)));   // Thursday of the same "on" week
  TEST_ASSERT_FALSE(occurs(m, date(2026, 11, 10)));
  TEST_ASSERT_FALSE(occurs(m, date(2026, 11, 12)));
  TEST_ASSERT_TRUE(occurs(m, date(2026, 11, 17)));
  TEST_ASSERT_TRUE(occurs(m, date(2026, 11, 19)));
}

void test_anchor_may_be_any_day_in_the_on_week() {
  // Anchored on the Monday of the "on" week: the Tuesday that week still counts.
  const Meeting m = {"Anchored Monday", kTue, 600, 660, 2, 2026, 11, 2};
  TEST_ASSERT_TRUE(occurs(m, date(2026, 11, 3)));
  TEST_ASSERT_FALSE(occurs(m, date(2026, 11, 10)));
  TEST_ASSERT_TRUE(occurs(m, date(2026, 11, 17)));
}

void test_every_three_weeks() {
  const Meeting m = {"Quarterly-ish", kTue, 600, 660, 3, 2026, 11, 3};
  TEST_ASSERT_FALSE(occurs(m, date(2026, 11, 10)));
  TEST_ASSERT_FALSE(occurs(m, date(2026, 11, 17)));
  TEST_ASSERT_TRUE(occurs(m, date(2026, 11, 24)));
}

void test_weekly_default_ignores_the_anchor() {
  const Meeting weeklyA = {"A", kTue, 600, 660, 0, 0, 0, 0};
  const Meeting weeklyB = {"B", kTue, 600, 660, 1, 2026, 11, 3};
  const Meeting plain = {"C", kTue, 600, 660};  // the original 4-field form still means weekly
  for (const DayDate &d : {date(2026, 11, 3), date(2026, 11, 10), date(2026, 11, 17)}) {
    TEST_ASSERT_TRUE(occurs(weeklyA, d));
    TEST_ASSERT_TRUE(occurs(weeklyB, d));
    TEST_ASSERT_TRUE(occurs(plain, d));
  }
}

void test_biweekly_meeting_still_sorts_with_weekly_ones() {
  const Meeting ms[] = {{"Weekly", kTue, 900, 960}, {"Biweekly", kTue, 600, 660, 2, 2026, 11, 3}};
  DayEvent out[4];
  TEST_ASSERT_EQUAL(2, eventsForDay(ms, 2, date(2026, 11, 3), out, 4));
  TEST_ASSERT_EQUAL_STRING("Biweekly", out[0].title);
  TEST_ASSERT_EQUAL(1, eventsForDay(ms, 2, date(2026, 11, 10), out, 4));
  TEST_ASSERT_EQUAL_STRING("Weekly", out[0].title);
}

void test_next_meeting_day_finds_the_first_day_with_a_meeting() {
  const Meeting ms[] = {{"Late", kTue, 900, 960}, {"Early", kTue, 600, 660}, {"Other", 1 << 5, 700, 760}};
  const DayDate days[] = {date(2026, 11, 4), date(2026, 11, 5), date(2026, 11, 6), date(2026, 11, 7)};  // Wed..Sat
  DayEvent first;
  TEST_ASSERT_EQUAL(2, nextMeetingDay(ms, 3, days, 4, &first));  // Friday: "Other"
  TEST_ASSERT_EQUAL_STRING("Other", first.title);
  const DayDate nextWeek[] = {date(2026, 11, 8), date(2026, 11, 9), date(2026, 11, 10)};  // Sun, Mon, Tue
  TEST_ASSERT_EQUAL(2, nextMeetingDay(ms, 3, nextWeek, 3, &first));
  TEST_ASSERT_EQUAL_STRING("Early", first.title);  // the earliest of that day's two
  TEST_ASSERT_EQUAL(600, first.startMin);
}

void test_next_meeting_day_none() {
  const Meeting ms[] = {{"Tue only", kTue, 600, 660}};
  const DayDate days[] = {date(2026, 11, 4), date(2026, 11, 5)};
  DayEvent first;
  TEST_ASSERT_EQUAL(-1, nextMeetingDay(ms, 1, days, 2, &first));
  TEST_ASSERT_EQUAL(-1, nextMeetingDay(ms, 0, days, 2, &first));
  TEST_ASSERT_EQUAL(-1, nextMeetingDay(ms, 1, days, 0, &first));
}

void test_next_meeting_day_skips_the_off_week_of_a_biweekly_meeting() {
  DayDate days[14];
  for (int i = 0; i < 14; i++) days[i] = date(2026, 11, 4 + i);  // Wed 4 Nov .. Tue 17 Nov
  DayEvent first;
  TEST_ASSERT_EQUAL(13, nextMeetingDay(&kBiweekly, 1, days, 14, &first));  // Tue 17 Nov, not Tue 10 Nov
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
  RUN_TEST(test_days_from_civil_reference_values);
  RUN_TEST(test_biweekly_alternates_weeks);
  RUN_TEST(test_biweekly_across_month_and_year_boundaries);
  RUN_TEST(test_biweekly_before_the_anchor);
  RUN_TEST(test_biweekly_never_on_other_weekdays);
  RUN_TEST(test_biweekly_with_several_weekdays_keeps_the_week_together);
  RUN_TEST(test_anchor_may_be_any_day_in_the_on_week);
  RUN_TEST(test_every_three_weeks);
  RUN_TEST(test_weekly_default_ignores_the_anchor);
  RUN_TEST(test_biweekly_meeting_still_sorts_with_weekly_ones);
  RUN_TEST(test_next_meeting_day_finds_the_first_day_with_a_meeting);
  RUN_TEST(test_next_meeting_day_none);
  RUN_TEST(test_next_meeting_day_skips_the_off_week_of_a_biweekly_meeting);
  return UNITY_END();
}
