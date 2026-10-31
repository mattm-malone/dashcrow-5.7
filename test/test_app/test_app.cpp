#include <unity.h>
#include "App.h"
#include "TimerPage.h"

void setUp() {}
void tearDown() {}

using Type = InputEvent::Type;
using State = TimerModel::State;

static LocalTime mk(uint8_t wday, uint8_t h, uint8_t m, uint8_t s = 0) {
  return {true, uint32_t(wday) * 86400u + h * 3600u + m * 60u + s, wday, h, m, s};
}
static InputEvent press(Button b) { return {Type::Press, b, 0}; }
static InputEvent turn(int dir) { return {Type::Turn, Button::Click, dir}; }
static InputEvent heldTurn(int dir) { return {Type::Turn, Button::Click, dir, 0, true}; }

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

void test_fast_presses_step_one_minute_each() {
  App a;
  toTimerPage(a, 0, mk(1, 9, 0));
  a.onInput(turn(1), 1000, mk(1, 9, 0));
  a.onInput(turn(1), 1010, mk(1, 9, 0));  // 10 ms later: still one minute, no acceleration
  a.onInput(turn(1), 1020, mk(1, 9, 0));
  TEST_ASSERT_EQUAL(28, a.timer().setMinutes());
  a.onInput(turn(-1), 1030, mk(1, 9, 0));
  TEST_ASSERT_EQUAL(27, a.timer().setMinutes());
}

void test_click_on_timer_page_starts() {
  App a;
  toTimerPage(a, 0, mk(1, 9, 0));
  a.onInput(press(Button::Click), 0, mk(1, 9, 0));
  TEST_ASSERT_TRUE(a.timer().state() == State::Running);
}

void test_exit_cycles_the_timer_style_and_leaves_the_timer_alone() {
  App a;
  toTimerPage(a, 0, mk(1, 9, 0));
  TEST_ASSERT_TRUE(a.timerStyle() == TimerStyle::Bar);
  a.onInput(press(Button::Click), 0, mk(1, 9, 0));  // start a timer
  const TimerStyle order[] = {TimerStyle::Hourglass, TimerStyle::Dither, TimerStyle::Disc, TimerStyle::Dots, TimerStyle::Bar, TimerStyle::Hourglass};
  for (TimerStyle s : order) {
    a.onInput(press(Button::Exit), 100, mk(1, 9, 0));
    TEST_ASSERT_TRUE(a.timerStyle() == s);
  }
  TEST_ASSERT_TRUE(a.timer().state() == State::Running);  // EXIT no longer cancels
}

void test_exit_does_not_change_the_style_off_the_timer_page() {
  App a;  // Clock
  a.onInput(press(Button::Exit), 0, mk(1, 9, 0));
  TEST_ASSERT_TRUE(a.timerStyle() == TimerStyle::Bar);
  a.onInput(press(Button::Menu), 0, mk(1, 9, 0));  // Timeline
  a.onInput(press(Button::Exit), 0, mk(1, 9, 0));
  TEST_ASSERT_TRUE(a.timerStyle() == TimerStyle::Bar);
}

void test_the_style_can_be_restored_and_survives_a_timer_cycle() {
  App a;
  a.setTimerStyle(TimerStyle::Disc);
  TEST_ASSERT_TRUE(a.timerStyle() == TimerStyle::Disc);
  toTimerPage(a, 0, mk(1, 9, 0));
  a.onInput(press(Button::Click), 0, mk(1, 9, 0));
  a.onInput(press(Button::ClickLong), 1000, mk(1, 9, 0));  // cancel
  TEST_ASSERT_TRUE(a.timerStyle() == TimerStyle::Disc);
  a.setTimerStyle(TimerStyle::kCount);  // not a real style: ignored
  TEST_ASSERT_TRUE(a.timerStyle() == TimerStyle::Disc);
}

void test_hold_select_cancels_a_running_timer_and_keeps_its_minutes() {
  App a;
  toTimerPage(a, 0, mk(1, 9, 0));
  a.onInput(turn(1), 0, mk(1, 9, 0));  // 26
  a.onInput(press(Button::Click), 0, mk(1, 9, 0));
  a.tick(5000, mk(1, 9, 0));
  a.onInput(press(Button::ClickLong), 6000, mk(1, 9, 0));
  TEST_ASSERT_TRUE(a.timer().state() == State::Setting);
  TEST_ASSERT_EQUAL(26, a.timer().setMinutes());
}

void test_hold_select_cancels_a_paused_or_finished_timer() {
  App a;
  toTimerPage(a, 0, mk(1, 9, 0));
  a.onInput(press(Button::Click), 0, mk(1, 9, 0));
  a.onInput(press(Button::Click), 1000, mk(1, 9, 0));  // pause
  TEST_ASSERT_TRUE(a.timer().state() == State::Paused);
  a.onInput(press(Button::ClickLong), 2000, mk(1, 9, 0));
  TEST_ASSERT_TRUE(a.timer().state() == State::Setting);
  a.onInput(press(Button::Click), 3000, mk(1, 9, 0));
  a.tick(3000 + 25u * 60000u, mk(1, 9, 30));
  TEST_ASSERT_TRUE(a.timer().state() == State::Done);
  a.onInput(press(Button::ClickLong), 3000 + 25u * 60000u + 10, mk(1, 9, 30));
  TEST_ASSERT_TRUE(a.timer().state() == State::Setting);
}

void test_hold_select_resets_minutes_to_25_when_setting() {
  App a;
  toTimerPage(a, 0, mk(1, 9, 0));
  for (int i = 0; i < 15; i++) a.onInput(turn(1), 1000u + i * 100u, mk(1, 9, 0));
  TEST_ASSERT_EQUAL(40, a.timer().setMinutes());
  a.onInput(press(Button::ClickLong), 5000, mk(1, 9, 0));
  TEST_ASSERT_EQUAL(25, a.timer().setMinutes());
  TEST_ASSERT_TRUE(a.timer().state() == State::Setting);
}

void test_hold_select_does_nothing_off_the_timer_page() {
  App a;  // Clock
  a.onInput(press(Button::ClickLong), 0, mk(1, 9, 0));
  TEST_ASSERT_TRUE(a.page() == Page::Clock);
  a.onInput(press(Button::Menu), 0, mk(1, 9, 0));
  a.onInput(turn(1), 10, mk(1, 9, 0));
  a.onInput(press(Button::ClickLong), 20, mk(1, 9, 0));  // Timeline: the day offset is left as is
  TEST_ASSERT_EQUAL(1, a.dayOffset());
}

void test_hold_select_dismisses_the_overlay_without_acting() {
  App a;
  toTimerPage(a, 0, mk(1, 9, 30));
  a.onInput(press(Button::Click), 0, mk(1, 9, 30));  // running
  raiseOverlay(a, 0);
  TEST_ASSERT_TRUE(a.overlayActive());
  a.onInput(press(Button::ClickLong), 10, mk(1, 10, 30));
  TEST_ASSERT_FALSE(a.overlayActive());
  TEST_ASSERT_TRUE(a.timer().state() == State::Running);  // the dismissing press was swallowed
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

static void toTimelinePage(App &a) { a.onInput(press(Button::Menu), 0, mk(1, 9, 0)); }

void test_timeline_turn_changes_day_offset() {
  App a;
  toTimelinePage(a);
  TEST_ASSERT_EQUAL(0, a.dayOffset());
  a.onInput(turn(1), 1000, mk(1, 9, 0));
  TEST_ASSERT_EQUAL(1, a.dayOffset());
  a.onInput(turn(-1), 2000, mk(1, 9, 0));
  a.onInput(turn(-1), 3000, mk(1, 9, 0));
  TEST_ASSERT_EQUAL(-1, a.dayOffset());
}

void test_timeline_fast_turns_still_step_one_day() {
  App a;
  toTimelinePage(a);
  a.onInput(turn(1), 1000, mk(1, 9, 0));
  a.onInput(turn(1), 1050, mk(1, 9, 0));  // within the acceleration window
  TEST_ASSERT_EQUAL(2, a.dayOffset());
}

void test_timeline_click_resets_offset() {
  App a;
  toTimelinePage(a);
  a.onInput(turn(1), 1000, mk(1, 9, 0));
  a.onInput(turn(1), 5000, mk(1, 9, 0));
  a.onInput(press(Button::Click), 6000, mk(1, 9, 0));
  TEST_ASSERT_EQUAL(0, a.dayOffset());
}

void test_offset_clamped_to_week() {
  App a;
  toTimelinePage(a);
  for (int i = 0; i < 20; i++) a.onInput(turn(1), 1000u + i * 1000u, mk(1, 9, 0));
  TEST_ASSERT_EQUAL(7, a.dayOffset());
  for (int i = 0; i < 40; i++) a.onInput(turn(-1), 100000u + i * 1000u, mk(1, 9, 0));
  TEST_ASSERT_EQUAL(-7, a.dayOffset());
}

void test_timer_finishing_off_page_switches_to_timer_page() {
  App a;
  toTimerPage(a, 0, mk(1, 9, 0));
  a.onInput(press(Button::Click), 0, mk(1, 9, 0));   // start 25 min
  a.onInput(press(Button::Menu), 1000, mk(1, 9, 0));  // Timer -> Clock
  TEST_ASSERT_TRUE(a.page() == Page::Clock);
  a.tick(24u * 60000u, mk(1, 9, 24));
  TEST_ASSERT_TRUE(a.page() == Page::Clock);  // still running
  a.tick(25u * 60000u, mk(1, 9, 25));
  TEST_ASSERT_TRUE(a.timer().state() == State::Done);
  TEST_ASSERT_TRUE(a.page() == Page::Timer);  // finished timer must be seen
}

void test_single_press_steps_one_but_held_repeats_snap_to_fives() {
  App a;
  toTimerPage(a, 0, mk(1, 9, 0));
  a.onInput(turn(1), 1000, mk(1, 9, 0));  // initial press: 25 -> 26
  TEST_ASSERT_EQUAL(26, a.timer().setMinutes());
  a.onInput(heldTurn(1), 1400, mk(1, 9, 0));
  TEST_ASSERT_EQUAL(30, a.timer().setMinutes());
  a.onInput(heldTurn(1), 1520, mk(1, 9, 0));
  TEST_ASSERT_EQUAL(35, a.timer().setMinutes());
  a.onInput(heldTurn(-1), 5000, mk(1, 9, 0));
  TEST_ASSERT_EQUAL(30, a.timer().setMinutes());
}

void test_held_repeat_on_timeline_still_scrolls_one_day() {
  App a;
  toTimelinePage(a);
  a.onInput(turn(1), 1000, mk(1, 9, 0));
  a.onInput(heldTurn(1), 1400, mk(1, 9, 0));
  TEST_ASSERT_EQUAL(2, a.dayOffset());
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
  RUN_TEST(test_fast_presses_step_one_minute_each);
  RUN_TEST(test_click_on_timer_page_starts);
  RUN_TEST(test_exit_cycles_the_timer_style_and_leaves_the_timer_alone);
  RUN_TEST(test_exit_does_not_change_the_style_off_the_timer_page);
  RUN_TEST(test_the_style_can_be_restored_and_survives_a_timer_cycle);
  RUN_TEST(test_hold_select_cancels_a_running_timer_and_keeps_its_minutes);
  RUN_TEST(test_hold_select_cancels_a_paused_or_finished_timer);
  RUN_TEST(test_hold_select_resets_minutes_to_25_when_setting);
  RUN_TEST(test_hold_select_does_nothing_off_the_timer_page);
  RUN_TEST(test_hold_select_dismisses_the_overlay_without_acting);
  RUN_TEST(test_timer_inputs_ignored_off_timer_page);
  RUN_TEST(test_standup_never_fires_without_valid_time);
  RUN_TEST(test_timeline_turn_changes_day_offset);
  RUN_TEST(test_timeline_fast_turns_still_step_one_day);
  RUN_TEST(test_timeline_click_resets_offset);
  RUN_TEST(test_offset_clamped_to_week);
  RUN_TEST(test_timer_finishing_off_page_switches_to_timer_page);
  RUN_TEST(test_single_press_steps_one_but_held_repeats_snap_to_fives);
  RUN_TEST(test_held_repeat_on_timeline_still_scrolls_one_day);
  return UNITY_END();
}
