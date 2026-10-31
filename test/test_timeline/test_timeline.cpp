#include <unity.h>
#include "TimelinePage.h"

void setUp() {}
void tearDown() {}

static const DayWindow kWindow{480, 1140};
static const SunTimes kSun{false, false, 400, 1000};

static TimelineView view(const DayEvent *e, size_t n, uint8_t h = 8, uint8_t m = 0) {
  TimelineView v{};
  v.timeValid = true;
  v.hour = h;
  v.minute = m;
  v.wday = 4;
  v.mday = 30;
  v.month = 10;
  v.events = e;
  v.n = n;
  v.window = kWindow;
  v.sun = kSun;
  return v;
}

static DayEvent ev(const char *title, int start, int end) {
  DayEvent e{};
  strncpy(e.title, title, 31);
  e.startMin = start;
  e.endMin = end;
  return e;
}

static int barMidY() { return kTlBarY + kTlBarH / 2; }

static int inkIn(Canvas &c, int x0, int y0, int x1, int y1) {
  int n = 0;
  for (int y = y0; y < y1; y++)
    for (int x = x0; x < x1; x++) n += c.getPixel(x, y);
  return n;
}

void test_meeting_block_spans_expected_x() {
  const DayEvent e[] = {ev("Design", 600, 660)};
  Canvas c(kW, kH);
  renderTimeline(c, view(e, 1));
  const int x0 = kTlX + minuteToX(600, kWindow, kTlW);
  const int x1 = kTlX + minuteToX(660, kWindow, kTlW);
  TEST_ASSERT_EQUAL(1, c.getPixel(x0 + 2, barMidY()));
  TEST_ASSERT_EQUAL(1, c.getPixel(x1 - 3, barMidY()));
  TEST_ASSERT_EQUAL(0, c.getPixel(x0 - 4, barMidY()));
  TEST_ASSERT_EQUAL(0, c.getPixel(x1 + 4, barMidY()));
}

void test_past_meeting_is_hatched() {
  const DayEvent e[] = {ev("Done", 600, 660)};
  Canvas past(kW, kH), future(kW, kH);
  renderTimeline(past, view(e, 1, 12, 0));    // 12:00, meeting ended
  renderTimeline(future, view(e, 1, 8, 0));   // 08:00, meeting upcoming
  const int x0 = kTlX + minuteToX(600, kWindow, kTlW) + 4;
  const int y0 = kTlBarY + 8;
  const int pastInk = inkIn(past, x0, y0, x0 + 20, y0 + 20);
  const int futureInk = inkIn(future, x0, y0, x0 + 20, y0 + 20);
  TEST_ASSERT_EQUAL(400, futureInk);
  TEST_ASSERT_GREATER_THAN(100, pastInk);
  TEST_ASSERT_LESS_THAN(300, pastInk);
}

void test_now_marker_present_only_today() {
  Canvas c(kW, kH);
  TimelineView v = view(nullptr, 0, 9, 30);
  renderTimeline(c, v);
  const int x = kTlX + minuteToX(570, kWindow, kTlW);
  TEST_ASSERT_EQUAL(1, c.getPixel(x, kTlBarY - 6));
  Canvas d(kW, kH);
  v.dayOffset = 1;
  renderTimeline(d, v);
  TEST_ASSERT_EQUAL(0, d.getPixel(x, kTlBarY - 6));
}

void test_nothing_drawn_outside_bar() {
  const DayEvent e[] = {ev("Early", 360, 420), ev("Late", 1200, 1260)};
  Canvas c(kW, kH);
  renderTimeline(c, view(e, 2));
  TEST_ASSERT_EQUAL(0, c.getPixel(kTlX + 4, barMidY()));
  TEST_ASSERT_EQUAL(0, c.getPixel(kTlX + kTlW - 5, barMidY()));
  TEST_ASSERT_EQUAL(0, inkIn(c, 0, kTlBarY, kTlX - 1, kTlBarY + kTlBarH));
  TEST_ASSERT_EQUAL(0, inkIn(c, kTlX + kTlW + 1, kTlBarY, kW, kTlBarY + kTlBarH));
}

void test_overlap_and_zero_length_do_not_crash() {
  const DayEvent e[] = {ev("A", 600, 700), ev("B", 650, 750), ev("Zero", 800, 800), ev("Backwards", 900, 850)};
  Canvas c(kW, kH);
  renderTimeline(c, view(e, 4));
  const int xz = kTlX + minuteToX(800, kWindow, kTlW);
  TEST_ASSERT_EQUAL(0, c.getPixel(xz, barMidY()));  // zero-length draws nothing
  TEST_ASSERT_EQUAL(1, c.getPixel(kTlX + minuteToX(675, kWindow, kTlW), barMidY()));
}

static int nextBandInk(Canvas &c) { return inkIn(c, 0, kNextY, kW, kNextY + kNextH); }

static UpcomingMeeting upcoming(const char *title, uint8_t daysAhead, uint8_t wday, uint16_t startMin) {
  UpcomingMeeting u{};
  u.valid = true;
  strncpy(u.title, title, 31);
  u.daysAhead = daysAhead;
  u.wday = wday;
  u.startMin = startMin;
  return u;
}

void test_next_line_names_the_next_meeting_today() {
  const DayEvent e[] = {ev("Design sync", 900, 930)};
  Canvas c(kW, kH);
  renderTimeline(c, view(e, 1, 14, 41));
  TEST_ASSERT_GREATER_THAN(200, nextBandInk(c));
}

void test_next_line_names_a_later_day_when_nothing_is_left_today() {
  const DayEvent e[] = {ev("Done", 540, 600)};  // over by 14:41
  TimelineView v = view(e, 1, 14, 41);
  Canvas without(kW, kH), tomorrow(kW, kH), later(kW, kH);
  renderTimeline(without, v);
  v.upcoming = upcoming("Sprint review", 1, 2, 840);
  renderTimeline(tomorrow, v);
  v.upcoming = upcoming("Sprint review", 4, 2, 840);
  renderTimeline(later, v);
  TEST_ASSERT_EQUAL(0, nextBandInk(without));            // no meeting at all: no text
  TEST_ASSERT_GREATER_THAN(200, nextBandInk(tomorrow));  // names tomorrow's meeting
  TEST_ASSERT_GREATER_THAN(200, nextBandInk(later));     // and one further out, labelled by weekday
  TEST_ASSERT_NOT_EQUAL(nextBandInk(tomorrow), nextBandInk(later));  // "Tomorrow" vs a weekday name
}

void test_a_meeting_left_today_wins_over_a_later_day() {
  const DayEvent e[] = {ev("Today's", 900, 930)};
  TimelineView v = view(e, 1, 14, 41);
  Canvas plain(kW, kH), withUpcoming(kW, kH);
  renderTimeline(plain, v);
  v.upcoming = upcoming("Some other day", 1, 2, 540);
  renderTimeline(withUpcoming, v);
  TEST_ASSERT_EQUAL(nextBandInk(plain), nextBandInk(withUpcoming));
}

void test_no_placeholder_text_without_meetings() {
  Canvas live(kW, kH), otherDay(kW, kH), noClock(kW, kH);
  TimelineView v = view(nullptr, 0, 14, 41);
  renderTimeline(live, v);
  v.dayOffset = 1;
  renderTimeline(otherDay, v);
  v.dayOffset = 0;
  v.timeValid = false;
  renderTimeline(noClock, v);
  TEST_ASSERT_EQUAL(0, nextBandInk(live));
  TEST_ASSERT_EQUAL(0, nextBandInk(otherDay));
  TEST_ASSERT_EQUAL(0, nextBandInk(noClock));
}

void test_stale_marker() {
  Canvas fresh(kW, kH), stale(kW, kH);
  TimelineView v = view(nullptr, 0);
  renderTimeline(fresh, v);
  v.feedStale = true;
  renderTimeline(stale, v);
  TEST_ASSERT_GREATER_THAN(inkIn(fresh, kStaleX, kStaleY, kStaleX + kStaleW, kStaleY + kStaleH),
                           inkIn(stale, kStaleX, kStaleY, kStaleX + kStaleW, kStaleY + kStaleH));
}

void test_invalid_time_has_no_marker_or_countdown_and_does_not_crash() {
  const DayEvent e[] = {ev("Design", 600, 660)};
  Canvas c(kW, kH);
  TimelineView v = view(e, 1);
  v.timeValid = false;
  renderTimeline(c, v);
  const int x = kTlX + minuteToX(480, kWindow, kTlW);
  TEST_ASSERT_EQUAL(0, c.getPixel(x, kTlBarY - 6));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_meeting_block_spans_expected_x);
  RUN_TEST(test_past_meeting_is_hatched);
  RUN_TEST(test_now_marker_present_only_today);
  RUN_TEST(test_nothing_drawn_outside_bar);
  RUN_TEST(test_overlap_and_zero_length_do_not_crash);
  RUN_TEST(test_next_line_names_the_next_meeting_today);
  RUN_TEST(test_next_line_names_a_later_day_when_nothing_is_left_today);
  RUN_TEST(test_a_meeting_left_today_wins_over_a_later_day);
  RUN_TEST(test_no_placeholder_text_without_meetings);
  RUN_TEST(test_stale_marker);
  RUN_TEST(test_invalid_time_has_no_marker_or_countdown_and_does_not_crash);
  return UNITY_END();
}
