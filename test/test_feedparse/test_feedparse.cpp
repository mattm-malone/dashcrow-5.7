#include <unity.h>
#include <initializer_list>
#include <string.h>
#include "FeedParse.h"

void setUp() {}
void tearDown() {}

void test_parse_clock_valid_and_invalid() {
  uint16_t m = 0;
  TEST_ASSERT_TRUE(parseClock("09:05", m));
  TEST_ASSERT_EQUAL(545, m);
  TEST_ASSERT_TRUE(parseClock("24:00", m));
  TEST_ASSERT_EQUAL(1440, m);
  TEST_ASSERT_TRUE(parseClock("00:00", m));
  TEST_ASSERT_EQUAL(0, m);
  const char *const bads[] = {"", "9:05", "09:5", "25:00", "12:60", "24:01", "ab:cd", "09-05", "09:055", nullptr};
  for (const char *bad : bads)
    TEST_ASSERT_FALSE_MESSAGE(parseClock(bad, m), bad ? bad : "(null)");
}

void test_valid_feed() {
  const char *json =
      "{\"date\":\"2026-10-30\",\"events\":[{\"title\":\"Dentist\",\"start\":\"10:00\",\"end\":\"11:00\"},"
      "{\"title\":\"Call\",\"start\":\"14:00\",\"end\":\"24:00\"}]}";
  DayEvent out[8];
  size_t n = 99;
  TEST_ASSERT_TRUE(parseFeedJson(json, "2026-10-30", out, 8, n));
  TEST_ASSERT_EQUAL(2, n);
  TEST_ASSERT_EQUAL_STRING("Dentist", out[0].title);
  TEST_ASSERT_EQUAL(600, out[0].startMin);
  TEST_ASSERT_EQUAL(660, out[0].endMin);
  TEST_ASSERT_EQUAL(1440, out[1].endMin);
}

void test_free_day_is_valid_and_empty() {
  DayEvent out[4];
  size_t n = 99;
  TEST_ASSERT_TRUE(parseFeedJson("{\"date\":\"2026-10-30\",\"events\":[]}", "2026-10-30", out, 4, n));
  TEST_ASSERT_EQUAL(0, n);
}

void test_wrong_date_rejected() {
  DayEvent out[4];
  size_t n = 99;
  TEST_ASSERT_FALSE(parseFeedJson("{\"date\":\"2026-10-29\",\"events\":[]}", "2026-10-30", out, 4, n));
  TEST_ASSERT_EQUAL(0, n);
}

void test_malformed_json_rejected() {
  DayEvent out[4];
  size_t n = 99;
  for (const char *bad : {"", "not json", "{\"date\":", "[]", "{\"error\":\"boom\"}", "{\"date\":\"2026-10-30\"}"}) {
    n = 99;
    TEST_ASSERT_FALSE_MESSAGE(parseFeedJson(bad, "2026-10-30", out, 4, n), bad);
    TEST_ASSERT_EQUAL(0, n);
  }
  TEST_ASSERT_FALSE(parseFeedJson(nullptr, "2026-10-30", out, 4, n));
}

void test_malformed_events_skipped() {
  const char *json =
      "{\"date\":\"2026-10-30\",\"events\":[42,{\"title\":\"NoTimes\"},"
      "{\"title\":\"BadTime\",\"start\":\"9:00\",\"end\":\"10:00\"},"
      "{\"title\":\"Backwards\",\"start\":\"11:00\",\"end\":\"10:00\"},"
      "{\"title\":\"Zero\",\"start\":\"11:00\",\"end\":\"11:00\"},"
      "{\"start\":\"12:00\",\"end\":\"12:30\"},"
      "{\"title\":\"Good\",\"start\":\"13:00\",\"end\":\"14:00\"}]}";
  DayEvent out[8];
  size_t n = 0;
  TEST_ASSERT_TRUE(parseFeedJson(json, "2026-10-30", out, 8, n));
  TEST_ASSERT_EQUAL(2, n);  // the untitled 12:00 event and "Good"
  TEST_ASSERT_EQUAL_STRING("", out[0].title);
  TEST_ASSERT_EQUAL_STRING("Good", out[1].title);
}

void test_cap_and_long_title() {
  const char *json =
      "{\"date\":\"2026-10-30\",\"events\":[{\"title\":\"0123456789012345678901234567890123456789\",\"start\":\"09:00\","
      "\"end\":\"10:00\"},{\"title\":\"B\",\"start\":\"10:00\",\"end\":\"11:00\"},{\"title\":\"C\",\"start\":\"11:00\","
      "\"end\":\"12:00\"}]}";
  DayEvent out[2];
  size_t n = 0;
  TEST_ASSERT_TRUE(parseFeedJson(json, "2026-10-30", out, 2, n));
  TEST_ASSERT_EQUAL(2, n);
  TEST_ASSERT_EQUAL(31, strlen(out[0].title));
}

void test_fetch_due_schedule() {
  constexpr uint32_t kRefresh = 900000;
  TEST_ASSERT_TRUE(feedFetchDue(false, 0, false, false, kRefresh));            // never tried
  TEST_ASSERT_FALSE(feedFetchDue(true, kRefresh - 1, true, true, kRefresh));   // good and current: wait
  TEST_ASSERT_TRUE(feedFetchDue(true, kRefresh, true, true, kRefresh));
  TEST_ASSERT_FALSE(feedFetchDue(true, kFeedRetryMs - 1, false, true, kRefresh));  // failed: retry soon
  TEST_ASSERT_TRUE(feedFetchDue(true, kFeedRetryMs, false, true, kRefresh));
}

void test_fetch_due_after_midnight_does_not_wait_full_refresh() {
  constexpr uint32_t kRefresh = 900000;
  TEST_ASSERT_TRUE(feedFetchDue(true, kFeedRetryMs, true, false, kRefresh));   // stored events are yesterday's
  TEST_ASSERT_FALSE(feedFetchDue(true, kFeedRetryMs - 1, true, false, kRefresh));  // but no hammering
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_parse_clock_valid_and_invalid);
  RUN_TEST(test_valid_feed);
  RUN_TEST(test_free_day_is_valid_and_empty);
  RUN_TEST(test_wrong_date_rejected);
  RUN_TEST(test_malformed_json_rejected);
  RUN_TEST(test_malformed_events_skipped);
  RUN_TEST(test_cap_and_long_title);
  RUN_TEST(test_fetch_due_schedule);
  RUN_TEST(test_fetch_due_after_midnight_does_not_wait_full_refresh);
  return UNITY_END();
}
