#include <unity.h>
#include <string.h>
#include "Merge.h"

void setUp() {}
void tearDown() {}

static DayEvent ev(const char *title, int a, int b) {
  DayEvent e{};
  strncpy(e.title, title, 31);
  e.startMin = a;
  e.endMin = b;
  return e;
}

void test_merge_sorted() {
  const DayEvent a[] = {ev("A1", 540, 600), ev("A2", 900, 960)};
  const DayEvent b[] = {ev("B1", 700, 760)};
  DayEvent out[8];
  TEST_ASSERT_EQUAL(3, mergeEvents(a, 2, b, 1, out, 8));
  TEST_ASSERT_EQUAL_STRING("A1", out[0].title);
  TEST_ASSERT_EQUAL_STRING("B1", out[1].title);
  TEST_ASSERT_EQUAL_STRING("A2", out[2].title);
}

void test_duplicates_removed() {
  const DayEvent a[] = {ev("Standup", 540, 555)};
  const DayEvent b[] = {ev("Standup", 540, 555), ev("Standup", 600, 615)};
  DayEvent out[8];
  TEST_ASSERT_EQUAL(2, mergeEvents(a, 1, b, 2, out, 8));
}

void test_same_start_different_title_both_kept_a_first() {
  const DayEvent a[] = {ev("Mine", 540, 600)};
  const DayEvent b[] = {ev("Theirs", 540, 600)};
  DayEvent out[8];
  TEST_ASSERT_EQUAL(2, mergeEvents(a, 1, b, 1, out, 8));
  TEST_ASSERT_EQUAL_STRING("Mine", out[0].title);
}

void test_cap_respected() {
  const DayEvent a[] = {ev("A", 100, 110), ev("B", 200, 210), ev("C", 300, 310)};
  const DayEvent b[] = {ev("D", 150, 160)};
  DayEvent out[2];
  TEST_ASSERT_EQUAL(2, mergeEvents(a, 3, b, 1, out, 2));
  TEST_ASSERT_EQUAL_STRING("A", out[0].title);
  TEST_ASSERT_EQUAL_STRING("D", out[1].title);
}

void test_empty_inputs() {
  const DayEvent a[] = {ev("A", 100, 110)};
  DayEvent out[4];
  TEST_ASSERT_EQUAL(0, mergeEvents(nullptr, 0, nullptr, 0, out, 4));
  TEST_ASSERT_EQUAL(1, mergeEvents(a, 1, nullptr, 0, out, 4));
  TEST_ASSERT_EQUAL(1, mergeEvents(nullptr, 0, a, 1, out, 4));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_merge_sorted);
  RUN_TEST(test_duplicates_removed);
  RUN_TEST(test_same_start_different_title_both_kept_a_first);
  RUN_TEST(test_cap_respected);
  RUN_TEST(test_empty_inputs);
  return UNITY_END();
}
