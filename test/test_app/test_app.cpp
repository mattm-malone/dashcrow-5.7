#include <unity.h>
#include "App.h"

void setUp() {}
void tearDown() {}

using Type = InputEvent::Type;
using State = TimerModel::State;

static LocalTime mk(uint8_t wday, uint8_t h, uint8_t m, uint8_t s = 0) {
  return {true, uint32_t(wday) * 86400u + h * 3600u + m * 60u + s, wday, h, m, s};
}
static InputEvent press(Button b) { return {Type::Press, b, 0}; }
static InputEvent turn(int dir) { return {Type::Turn, Button::Click, dir}; }

// An App whose stand-up overlay is showing, sitting on the Timer page.
static void toTimerPage(App &a, uint32_t ms, const LocalTime &t) {
  a.onInput(press(Button::Menu), ms, t);  // Clock -> Timeline
  a.onInput(press(Button::Menu), ms, t);  // Timeline -> Timer
}
static void raiseOverlay(App &a, uint32_t ms) {
  a.tick(ms, mk(1, 9, 30));
  a.tick(ms + 1, mk(1, 10, 30));
}

void test_boots_on_clock() {
  App a;
  TEST_ASSERT_TRUE(a.page() == Page::Clock);
  TEST_ASSERT_FALSE(a.overlayActive());
}

void test_menu_cycles_pages_in_order() {
  App a;
  LocalTime t = mk(1, 9, 0);
  a.onInput(press(Button::Menu), 0, t);
  TEST_ASSERT_TRUE(a.page() == Page::Timeline);
  a.onInput(press(Button::Menu), 0, t);
  TEST_ASSERT_TRUE(a.page() == Page::Timer);
  a.onInput(press(Button::Menu), 0, t);
  TEST_ASSERT_TRUE(a.page() == Page::Clock);
}

void test_overlay_appears_when_due() {
  App a;
  raiseOverlay(a, 0);
  TEST_ASSERT_TRUE(a.overlayActive());
}

void test_menu_dismisses_overlay_without_changing_page() {
  App a;
  raiseOverlay(a, 0);
  a.onInput(press(Button::Menu), 10, mk(1, 10, 30));
  TEST_ASSERT_FALSE(a.overlayActive());
  TEST_ASSERT_TRUE(a.page() == Page::Clock);
}

void test_exit_and_click_also_dismiss() {
  App a;
  raiseOverlay(a, 0);
  a.onInput(press(Button::Exit), 10, mk(1, 10, 30));
  TEST_ASSERT_FALSE(a.overlayActive());
  App b;
  raiseOverlay(b, 0);
  b.onInput(press(Button::Click), 10, mk(1, 10, 30));
  TEST_ASSERT_FALSE(b.overlayActive());
}

void test_turn_does_not_dismiss_or_change_timer() {
  App a;
  toTimerPage(a, 0, mk(1, 9, 30));
  raiseOverlay(a, 0);
  a.onInput(turn(1), 10, mk(1, 10, 30));
  TEST_ASSERT_TRUE(a.overlayActive());
  TEST_ASSERT_EQUAL(25, a.timer().setMinutes());
}

void test_dismiss_does_not_start_timer_on_click() {
  App a;
  toTimerPage(a, 0, mk(1, 9, 30));
  raiseOverlay(a, 0);
  a.onInput(press(Button::Click), 10, mk(1, 10, 30));
  TEST_ASSERT_FALSE(a.overlayActive());
  TEST_ASSERT_TRUE(a.page() == Page::Timer);
  TEST_ASSERT_TRUE(a.timer().state() == State::Setting);
}

void test_timer_keeps_running_under_overlay() {
  App a;
  toTimerPage(a, 0, mk(1, 9, 30));
  a.onInput(press(Button::Click), 1000, mk(1, 9, 30));  // start
  a.tick(1000, mk(1, 9, 30));
  a.tick(2000, mk(1, 10, 30));  // overlay appears
  TEST_ASSERT_TRUE(a.overlayActive());
  a.tick(61000, mk(1, 10, 31));
  TEST_ASSERT_TRUE(a.timer().state() == State::Running);
  TEST_ASSERT_EQUAL_UINT32(25u * 60000u - 60000u, a.timer().remainingMs());
}

void test_done_state_survives_overlay() {
  App a;
  toTimerPage(a, 0, mk(1, 9, 30));
  a.onInput(turn(-1), 0, mk(1, 9, 30));  // 24 minutes
  a.onInput(press(Button::Click), 0, mk(1, 9, 30));
  a.tick(0, mk(1, 9, 30));
  const uint32_t end = a.timer().totalMs();
  a.tick(end, mk(1, 10, 30));  // timer done and stand-up due in the same tick
  TEST_ASSERT_TRUE(a.timer().state() == State::Done);
  TEST_ASSERT_TRUE(a.overlayActive());
  a.onInput(press(Button::Menu), end + 1, mk(1, 10, 30));
  TEST_ASSERT_TRUE(a.timer().state() == State::Done);
  TEST_ASSERT_TRUE(a.page() == Page::Timer);
}

void test_dismiss_acknowledges_scheduler() {
  App a;
  raiseOverlay(a, 0);
  a.onInput(press(Button::Menu), 10, mk(1, 10, 30, 5));
  a.tick(20, mk(1, 11, 0));
  TEST_ASSERT_FALSE(a.overlayActive());
  a.tick(30, mk(1, 11, 30, 5));
  TEST_ASSERT_TRUE(a.overlayActive());
}

void test_turn_on_timer_page_changes_minutes() {
  App a;
  toTimerPage(a, 0, mk(1, 9, 0));
  a.onInput(turn(1), 1000, mk(1, 9, 0));
  TEST_ASSERT_EQUAL(26, a.timer().setMinutes());
  a.onInput(turn(-1), 5000, mk(1, 9, 0));
  TEST_ASSERT_EQUAL(25, a.timer().setMinutes());
}

void test_click_on_timer_page_starts() {
  App a;
  toTimerPage(a, 0, mk(1, 9, 0));
  a.onInput(press(Button::Click), 0, mk(1, 9, 0));
  TEST_ASSERT_TRUE(a.timer().state() == State::Running);
}

void test_exit_cancels_timer() {
  App a;
  toTimerPage(a, 0, mk(1, 9, 0));
  a.onInput(press(Button::Click), 0, mk(1, 9, 0));
  a.onInput(press(Button::Exit), 100, mk(1, 9, 0));
  TEST_ASSERT_TRUE(a.timer().state() == State::Setting);
}

void test_timer_inputs_ignored_off_timer_page() {
  App a;  // on Clock
  a.onInput(press(Button::Click), 0, mk(1, 9, 0));
  a.onInput(turn(1), 10, mk(1, 9, 0));
  TEST_ASSERT_TRUE(a.timer().state() == State::Setting);
  TEST_ASSERT_EQUAL(25, a.timer().setMinutes());
}

void test_standup_never_fires_without_valid_time() {
  App a;
  LocalTime bad{false, 0, 1, 12, 0, 0};
  for (uint32_t i = 0; i < 10; i++) a.tick(i * 3600000u, bad);
  TEST_ASSERT_FALSE(a.overlayActive());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_boots_on_clock);
  RUN_TEST(test_menu_cycles_pages_in_order);
  RUN_TEST(test_overlay_appears_when_due);
  RUN_TEST(test_menu_dismisses_overlay_without_changing_page);
  RUN_TEST(test_exit_and_click_also_dismiss);
  RUN_TEST(test_turn_does_not_dismiss_or_change_timer);
  RUN_TEST(test_dismiss_does_not_start_timer_on_click);
  RUN_TEST(test_timer_keeps_running_under_overlay);
  RUN_TEST(test_done_state_survives_overlay);
  RUN_TEST(test_dismiss_acknowledges_scheduler);
  RUN_TEST(test_turn_on_timer_page_changes_minutes);
  RUN_TEST(test_click_on_timer_page_starts);
  RUN_TEST(test_exit_cancels_timer);
  RUN_TEST(test_timer_inputs_ignored_off_timer_page);
  RUN_TEST(test_standup_never_fires_without_valid_time);
  return UNITY_END();
}
