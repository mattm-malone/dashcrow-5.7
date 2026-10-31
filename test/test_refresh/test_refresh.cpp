#include <unity.h>
#include "RefreshPolicy.h"

void setUp() {}
void tearDown() {}

void test_unchanged_keys_none() {
  RefreshPolicy p;
  p.next(1, 1);  // first call
  TEST_ASSERT_TRUE(p.next(1, 1) == RefreshKind::None);
}

void test_first_call_is_full() {
  RefreshPolicy p;
  TEST_ASSERT_TRUE(p.next(7, 3) == RefreshKind::Full);
}

void test_content_change_is_partial() {
  RefreshPolicy p;
  p.next(1, 1);
  TEST_ASSERT_TRUE(p.next(2, 1) == RefreshKind::Partial);
}

void test_layout_change_is_full() {
  RefreshPolicy p;
  p.next(1, 1);
  TEST_ASSERT_TRUE(p.next(1, 2) == RefreshKind::Full);
}

void test_full_refresh_after_many_partials_then_resets() {
  RefreshPolicy p;
  p.next(0, 0);
  for (int i = 1; i <= kPartialsBeforeFull; i++)
    TEST_ASSERT_TRUE(p.next(i, 0) == RefreshKind::Partial);
  TEST_ASSERT_TRUE(p.next(kPartialsBeforeFull + 1, 0) == RefreshKind::Full);
  TEST_ASSERT_TRUE(p.next(kPartialsBeforeFull + 2, 0) == RefreshKind::Partial);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_unchanged_keys_none);
  RUN_TEST(test_first_call_is_full);
  RUN_TEST(test_content_change_is_partial);
  RUN_TEST(test_layout_change_is_full);
  RUN_TEST(test_full_refresh_after_many_partials_then_resets);
  return UNITY_END();
}
