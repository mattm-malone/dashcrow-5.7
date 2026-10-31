#include <unity.h>
#include <initializer_list>
#include <math.h>
#include "TimerStyles.h"

using namespace timerstyle;
using Mode = TimerView::Mode;
constexpr uint32_t kMin = 60000;

void setUp() {}
void tearDown() {}

static TimerView running(TimerStyle style, uint32_t totalMin, uint32_t remainingMs) {
  TimerView v{};
  v.mode = Mode::Running;
  v.style = style;
  v.totalMs = totalMin * kMin;
  v.remainingMs = remainingMs;
  v.minutesLeft = int((remainingMs + kMin - 1) / kMin);
  v.secondsLeft = int((remainingMs + 999) / 1000);
  return v;
}

static TimerView setting(TimerStyle style, int minutes) {
  TimerView v{};
  v.mode = Mode::Setting;
  v.style = style;
  v.setMinutes = minutes;
  return v;
}

static int inkIn(Canvas &c, int x0, int y0, int x1, int y1) {
  int n = 0;
  for (int y = y0; y < y1; y++)
    for (int x = x0; x < x1; x++) n += c.getPixel(x, y);
  return n;
}

static int discInk(Canvas &c) {
  int n = 0;
  for (int dy = -(kDiscR - 1); dy < kDiscR; dy++)
    for (int dx = -(kDiscR - 1); dx < kDiscR; dx++)
      if (dx * dx + dy * dy < (kDiscR - 1) * (kDiscR - 1)) n += c.getPixel(kDiscCx + dx, kDiscCy + dy);
  return n;
}

static int discArea() {
  int n = 0;
  for (int dy = -(kDiscR - 1); dy < kDiscR; dy++)
    for (int dx = -(kDiscR - 1); dx < kDiscR; dx++)
      if (dx * dx + dy * dy < (kDiscR - 1) * (kDiscR - 1)) n++;
  return n;
}

// Sand in the left (top) bulb and the right (bottom) bulb of the hourglass, excluding glass, caps and the stream row.
static int leftSand(Canvas &c) { return inkIn(c, kHgLeft + 8, kHgTop + 8, kHgNeck - 14, kHgMid - 3) + inkIn(c, kHgLeft + 8, kHgMid + 3, kHgNeck - 14, kHgBottom - 8); }
static int rightSand(Canvas &c) { return inkIn(c, kHgNeck + 14, kHgTop + 8, kHgRight - 8, kHgMid - 3) + inkIn(c, kHgNeck + 14, kHgMid + 3, kHgRight - 8, kHgBottom - 8); }

void test_hourglass_left_drains_while_right_fills() {
  const uint32_t stages[] = {25 * kMin, 18 * kMin, 12 * kMin, 6 * kMin, 2 * kMin};
  int prevLeft = 1 << 30, prevRight = -1;
  for (uint32_t r : stages) {
    Canvas c(kW, kH);
    renderTimer(c, running(TimerStyle::Hourglass, 25, r));
    TEST_ASSERT_LESS_THAN(prevLeft, leftSand(c));
    TEST_ASSERT_GREATER_THAN(prevRight, rightSand(c));
    prevLeft = leftSand(c);
    prevRight = rightSand(c);
  }
}

void test_hourglass_sand_amount_follows_the_bulb_area_not_the_length() {
  // Each bulb is a triangle, so sand depth d holds (d/L)^2 of it; the drain is steady in time when the surface sits
  // at d = L * sqrt(fraction). Differences between stages cancel the glass outline that crosses the windows:
  //   left:  (L(1)-L(.5)) / (L(.5)-L(.25)) = (1-.5)/(.5-.25) = 2     (a surface linear in time would give 4)
  //   right: (R(.5)-R(1)) / (R(.25)-R(.5)) = (.5-0)/(.75-.5) = 2
  Canvas f100(kW, kH), f50(kW, kH), f25(kW, kH);
  renderTimer(f100, setting(TimerStyle::Hourglass, 25));
  renderTimer(f50, running(TimerStyle::Hourglass, 25, uint32_t(12.5f * kMin)));
  renderTimer(f25, running(TimerStyle::Hourglass, 25, uint32_t(6.25f * kMin)));
  const float leftRatio = float(leftSand(f100) - leftSand(f50)) / float(leftSand(f50) - leftSand(f25));
  const float rightRatio = float(rightSand(f50) - rightSand(f100)) / float(rightSand(f25) - rightSand(f50));
  TEST_ASSERT_FLOAT_WITHIN(0.3f, 2.0f, leftRatio);
  TEST_ASSERT_FLOAT_WITHIN(0.3f, 2.0f, rightRatio);
}

void test_hourglass_stream_runs_only_while_sand_is_falling() {
  // The stream lives in the empty gap between the neck and the surface of the sand in the right bulb.
  auto streamInk = [](Canvas &c, float f) {
    const int d = int(float(kHgNeck - kHgLeft) * sqrtf(f));
    return inkIn(c, kHgNeck + 3, kHgMid - 1, kHgNeck + d, kHgMid + 3);
  };
  Canvas mid(kW, kH), start(kW, kH);
  renderTimer(mid, running(TimerStyle::Hourglass, 25, 12 * kMin + 30000));
  renderTimer(start, setting(TimerStyle::Hourglass, 25));
  TEST_ASSERT_GREATER_THAN(0, streamInk(mid, 0.5f));
  TEST_ASSERT_EQUAL(0, streamInk(start, 1.0f));
}

void test_disc_wedge_area_matches_the_fraction() {
  for (float f : {0.25f, 0.5f, 0.75f}) {
    Canvas c(kW, kH);
    renderTimer(c, running(TimerStyle::Disc, 20, uint32_t(f * 20 * kMin)));
    TEST_ASSERT_FLOAT_WITHIN(0.03f, f, float(discInk(c)) / float(discArea()));
  }
  Canvas full(kW, kH), none(kW, kH);
  renderTimer(full, setting(TimerStyle::Disc, 25));
  renderTimer(none, running(TimerStyle::Disc, 25, 0));
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.0f, float(discInk(full)) / float(discArea()));
  TEST_ASSERT_EQUAL(0, discInk(none));
}

void test_disc_wedge_ends_at_twelve_o_clock_and_sweeps_clockwise() {
  Canvas c(kW, kH);
  renderTimer(c, running(TimerStyle::Disc, 20, 5 * kMin));  // a quarter left: the top-left quadrant
  TEST_ASSERT_EQUAL(1, c.getPixel(kDiscCx - 40, kDiscCy - 40));  // top-left is filled
  TEST_ASSERT_EQUAL(0, c.getPixel(kDiscCx + 40, kDiscCy - 40));  // top-right has been used up
  TEST_ASSERT_EQUAL(0, c.getPixel(kDiscCx + 40, kDiscCy + 40));
  TEST_ASSERT_EQUAL(0, c.getPixel(kDiscCx - 40, kDiscCy + 40));
}

void test_dot_counts() {
  TEST_ASSERT_EQUAL(25, dotCount(running(TimerStyle::Dots, 25, 10 * kMin), false));
  TEST_ASSERT_EQUAL(60, dotCount(running(TimerStyle::Dots, 60, 10 * kMin), false));
  TEST_ASSERT_EQUAL(18, dotCount(running(TimerStyle::Dots, 90, 10 * kMin), false));   // 5-minute dots above an hour
  TEST_ASSERT_EQUAL(36, dotCount(running(TimerStyle::Dots, 180, 10 * kMin), false));
  TEST_ASSERT_EQUAL(60, dotCount(running(TimerStyle::Dots, 25, 30000), true));        // one per second in the last minute
  TEST_ASSERT_EQUAL(25, dotCount(setting(TimerStyle::Dots, 25), false));
  TEST_ASSERT_EQUAL(18, dotCount(setting(TimerStyle::Dots, 90), false));
}

void test_dot_layout_fits_the_canvas_without_overlap() {
  for (int n : {1, 2, 5, 18, 25, 36, 60}) {
    const DotLayout l = dotLayout(n);
    TEST_ASSERT_GREATER_OR_EQUAL(2 * l.radius, l.cell);
    TEST_ASSERT_GREATER_THAN(0, l.radius);
    for (int i = 0; i < n; i++) {
      int x, y;
      dotCenter(l, i, x, y);
      TEST_ASSERT_GREATER_OR_EQUAL(l.radius, x);
      TEST_ASSERT_LESS_OR_EQUAL(kW - l.radius, x);
      TEST_ASSERT_GREATER_OR_EQUAL(54 + l.radius, y);  // below the header
      TEST_ASSERT_LESS_OR_EQUAL(kH - l.radius, y);
    }
  }
}

void test_dots_empty_from_the_end() {
  Canvas c(kW, kH);
  renderTimer(c, running(TimerStyle::Dots, 25, 12 * kMin + 30000));  // 12.5 minutes left of 25
  const DotLayout l = dotLayout(25);
  for (int i = 0; i < 25; i++) {
    int x, y;
    dotCenter(l, i, x, y);
    if (i < 12) TEST_ASSERT_EQUAL_MESSAGE(1, c.getPixel(x, y), "used dot should still be full");
    if (i > 12) TEST_ASSERT_EQUAL_MESSAGE(0, c.getPixel(x, y), "spent dot should be empty");
  }
}

void test_dots_setting_shows_one_dot_per_minute_and_the_number() {
  Canvas a(kW, kH), b(kW, kH);
  renderTimer(a, setting(TimerStyle::Dots, 10));
  renderTimer(b, setting(TimerStyle::Dots, 20));
  TEST_ASSERT_GREATER_THAN(inkIn(a, 0, 54, kW, kH), inkIn(b, 0, 54, kW, kH));
  TEST_ASSERT_GREATER_THAN(100, inkIn(a, 250, 0, 540, 40));  // "10 MIN" in the header
}

void test_every_style_inverts_in_the_last_minute_and_when_done() {
  for (TimerStyle s : {TimerStyle::Bar, TimerStyle::Hourglass, TimerStyle::Disc, TimerStyle::Dots}) {
    Canvas last(kW, kH), done(kW, kH), normal(kW, kH);
    renderTimer(last, running(s, 25, 30000));
    TimerView d = running(s, 25, 0);
    d.mode = Mode::Done;
    renderTimer(done, d);
    renderTimer(normal, running(s, 25, 10 * kMin));
    TEST_ASSERT_EQUAL(1, last.getPixel(2, 2));
    TEST_ASSERT_EQUAL(1, done.getPixel(2, 2));
    TEST_ASSERT_EQUAL(0, normal.getPixel(2, 2));
  }
}

void test_every_style_shows_the_number_of_minutes_left() {
  // The big number is the one thing every style must keep legible: two different values render differently.
  for (TimerStyle s : {TimerStyle::Bar, TimerStyle::Hourglass, TimerStyle::Disc, TimerStyle::Dots}) {
    Canvas a(kW, kH), b(kW, kH);
    renderTimer(a, running(s, 25, 20 * kMin));
    renderTimer(b, running(s, 25, 19 * kMin));
    bool differs = false;
    for (int y = 0; y < kH && !differs; y++)
      for (int x = 0; x < kW; x++)
        if (a.getPixel(x, y) != b.getPixel(x, y)) {
          differs = true;
          break;
        }
    TEST_ASSERT_TRUE(differs);
  }
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_hourglass_left_drains_while_right_fills);
  RUN_TEST(test_hourglass_sand_amount_follows_the_bulb_area_not_the_length);
  RUN_TEST(test_hourglass_stream_runs_only_while_sand_is_falling);
  RUN_TEST(test_disc_wedge_area_matches_the_fraction);
  RUN_TEST(test_disc_wedge_ends_at_twelve_o_clock_and_sweeps_clockwise);
  RUN_TEST(test_dot_counts);
  RUN_TEST(test_dot_layout_fits_the_canvas_without_overlap);
  RUN_TEST(test_dots_empty_from_the_end);
  RUN_TEST(test_dots_setting_shows_one_dot_per_minute_and_the_number);
  RUN_TEST(test_every_style_inverts_in_the_last_minute_and_when_done);
  RUN_TEST(test_every_style_shows_the_number_of_minutes_left);
  return UNITY_END();
}
