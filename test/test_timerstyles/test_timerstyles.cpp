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

// Ink in the two dot fields of the halftone hourglass.
static int leftField(Canvas &c) { return inkIn(c, kHgLeftX, kHgFieldY, kHgLeftX + kHgFieldW, kHgFieldY + kHgRows * kHgPitch); }
static int rightField(Canvas &c) { return inkIn(c, kHgRightX, kHgFieldY, kHgRightX + kHgFieldW, kHgFieldY + kHgRows * kHgPitch); }
// The last minute inverts the whole canvas; count dots as ink after undoing that.
static int fieldDots(Canvas &c, int x0, bool inverted) {
  int n = 0;
  for (int y = kHgFieldY; y < kHgFieldY + kHgRows * kHgPitch; y++)
    for (int x = x0; x < x0 + kHgFieldW; x++) n += inverted ? !c.getPixel(x, y) : c.getPixel(x, y);
  return n;
}
static int centerInk(Canvas &c) { return inkIn(c, kHgCenterL, kHgFrameY + 4, kHgCenterR, kHgFrameY + kHgFrameH - 4); }

void test_hourglass_dots_flow_from_left_to_right() {
  const uint32_t stages[] = {25 * kMin, 18 * kMin, 12 * kMin, 6 * kMin, 2 * kMin};
  int prevLeft = 1 << 30, prevRight = -1;
  for (uint32_t r : stages) {
    Canvas c(kW, kH);
    renderTimer(c, running(TimerStyle::Hourglass, 25, r));
    TEST_ASSERT_LESS_THAN(prevLeft, leftField(c));
    TEST_ASSERT_GREATER_THAN(prevRight, rightField(c));
    prevLeft = leftField(c);
    prevRight = rightField(c);
  }
}

void test_hourglass_fields_are_mirror_images_at_half_time() {
  Canvas c(kW, kH);
  renderTimer(c, running(TimerStyle::Hourglass, 25, uint32_t(12.5f * kMin)));
  TEST_ASSERT_FLOAT_WITHIN(0.1f, 1.0f, float(leftField(c)) / float(rightField(c)));
}

void test_hourglass_start_is_all_left_and_finish_is_all_right() {
  Canvas start(kW, kH), done(kW, kH);
  renderTimer(start, setting(TimerStyle::Hourglass, 25));
  renderTimer(done, running(TimerStyle::Hourglass, 25, 1));  // a hair above zero, still counting
  TEST_ASSERT_EQUAL(0, rightField(start));
  TEST_ASSERT_GREATER_THAN(3000, leftField(start));
  // 1 ms left is the last minute (inverted); the last grains of the stream still run through the emptied left field.
  TEST_ASSERT_LESS_THAN(leftField(start) / 5, fieldDots(done, kHgLeftX, true));
  TEST_ASSERT_GREATER_THAN(3000, fieldDots(done, kHgRightX, true));
}

void test_hourglass_does_not_refill_in_the_last_minute() {
  // 30 s left of 25 minutes: the dots are almost all on the right; the picture must not be rescaled to the minute.
  Canvas start(kW, kH), last(kW, kH);
  renderTimer(start, setting(TimerStyle::Hourglass, 25));
  renderTimer(last, running(TimerStyle::Hourglass, 25, 30000));
  const int inverted = fieldDots(last, kHgLeftX, true);
  TEST_ASSERT_LESS_THAN(leftField(start) / 10, inverted);
}

void test_hourglass_number_is_minutes_and_seconds() {
  Canvas a(kW, kH), b(kW, kH), same(kW, kH);
  renderTimer(a, running(TimerStyle::Hourglass, 25, 12 * kMin + 5000));
  renderTimer(b, running(TimerStyle::Hourglass, 25, 12 * kMin + 4000));  // one second later
  renderTimer(same, running(TimerStyle::Hourglass, 25, 12 * kMin + 5000));
  bool differs = false, identical = true;
  for (int y = kHgFrameY; y < kHgFrameY + kHgFrameH; y++)
    for (int x = kHgCenterL; x < kHgCenterR; x++) {
      if (a.getPixel(x, y) != b.getPixel(x, y)) differs = true;
      if (a.getPixel(x, y) != same.getPixel(x, y)) identical = false;
    }
  TEST_ASSERT_TRUE(differs);    // the seconds digits changed
  TEST_ASSERT_TRUE(identical);  // and rendering is deterministic
}

void test_hourglass_number_fits_for_three_digit_minutes() {
  Canvas c(kW, kH);
  renderTimer(c, running(TimerStyle::Hourglass, 180, 125 * kMin));
  TEST_ASSERT_GREATER_THAN(500, centerInk(c));
  // Nothing from the number leaks into the dot fields' gutters or the frame.
  // (The pouring stream passes above the number, so look only at the rows the digits can occupy.)
  const int numberTop = kHgFrameY + (kHgFrameH - 100) / 2;
  TEST_ASSERT_EQUAL(0, inkIn(c, kHgCenterL - 6, numberTop, kHgCenterL - 1, kHgFrameY + kHgFrameH - 4));
  TEST_ASSERT_EQUAL(0, inkIn(c, kHgCenterR + 1, numberTop, kHgCenterR + 6, kHgFrameY + kHgFrameH - 4));
}

void test_stream_exists_only_while_sand_is_pouring() {
  StreamDot dots[kStreamDots];
  TEST_ASSERT_EQUAL(0, hourglassStream(setting(TimerStyle::Hourglass, 25), dots, kStreamDots));
  TEST_ASSERT_EQUAL(0, hourglassStream(running(TimerStyle::Hourglass, 25, 0), dots, kStreamDots));   // all poured
  TEST_ASSERT_EQUAL(0, hourglassStream(running(TimerStyle::Hourglass, 25, 25 * kMin), dots, kStreamDots));  // not started
  TEST_ASSERT_GREATER_THAN(10, hourglassStream(running(TimerStyle::Hourglass, 25, 12 * kMin), dots, kStreamDots));
  TEST_ASSERT_EQUAL(0, hourglassStream(running(TimerStyle::Bar, 25, 12 * kMin), dots, kStreamDots));  // only this style
}

void test_stream_arches_over_the_number_from_the_left_field_to_the_right() {
  StreamDot dots[kStreamDots];
  const int n = hourglassStream(running(TimerStyle::Hourglass, 25, 12 * kMin), dots, kStreamDots);
  const int numberTop = kHgFrameY + (kHgFrameH - 100) / 2;  // the number is at most 100 px tall, centred in the frame
  for (int i = 0; i < n; i++) {
    TEST_ASSERT_LESS_THAN(numberTop - dots[i].r, dots[i].y);                // clear of the digits
    TEST_ASSERT_GREATER_THAN(kHgFrameY + 3 + dots[i].r, dots[i].y);         // inside the frame
    TEST_ASSERT_GREATER_OR_EQUAL(kHgLeftX, dots[i].x);
    TEST_ASSERT_LESS_OR_EQUAL(kHgRightX + kHgFieldW, dots[i].x);
  }
  // It starts inside the left field and ends inside the right one, moving left to right.
  TEST_ASSERT_LESS_THAN(kHgLeftX + kHgFieldW, dots[0].x);
  TEST_ASSERT_GREATER_THAN(kHgRightX, dots[n - 1].x);
  for (int i = 1; i < n; i++) TEST_ASSERT_GREATER_THAN(dots[i - 1].x, dots[i].x);
}

void test_stream_grains_shrink_as_they_fall() {
  StreamDot dots[kStreamDots];
  const int n = hourglassStream(running(TimerStyle::Hourglass, 25, 12 * kMin), dots, kStreamDots);
  TEST_ASSERT_GREATER_THAN(dots[n - 1].r, dots[0].r);
  for (int i = 1; i < n; i++) TEST_ASSERT_LESS_OR_EQUAL(dots[i - 1].r, dots[i].r);
}

void test_stream_advances_with_time_and_freezes_when_time_stops() {
  // Same dot, a quarter of a second later (remaining time falls by 250 ms): it has moved right.
  StreamDot a[kStreamDots], b[kStreamDots], c[kStreamDots];
  const uint32_t base = 12 * kMin;  // elapsed = 13 min exactly: phase 0
  const int na = hourglassStream(running(TimerStyle::Hourglass, 25, base), a, kStreamDots);
  const int nb = hourglassStream(running(TimerStyle::Hourglass, 25, base - 250), b, kStreamDots);
  const int nc = hourglassStream(running(TimerStyle::Hourglass, 25, base), c, kStreamDots);
  TEST_ASSERT_EQUAL(na, nc);
  TEST_ASSERT_GREATER_THAN(0, nb);
  for (int i = 0; i < na && i < nb; i++) {
    TEST_ASSERT_GREATER_THAN(a[i].x, b[i].x);   // moved right
    TEST_ASSERT_EQUAL(a[i].x, c[i].x);          // same time, same picture
    TEST_ASSERT_EQUAL(a[i].y, c[i].y);
  }
}

void test_stream_phase_is_a_fraction_that_wraps() {
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, streamPhase(0));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, kStreamSpeed * 0.25f, streamPhase(250));
  TEST_ASSERT_TRUE(streamPhase(1500) >= 0.0f && streamPhase(1500) < 1.0f);
  TEST_ASSERT_TRUE(streamPhase(4000000000u) >= 0.0f && streamPhase(4000000000u) < 1.0f);
}

void test_stream_is_drawn_on_the_canvas_above_the_digits() {
  Canvas with(kW, kH), without(kW, kH);
  renderTimer(with, running(TimerStyle::Hourglass, 25, 12 * kMin));
  renderTimer(without, setting(TimerStyle::Hourglass, 25));
  const int numberTop = kHgFrameY + (kHgFrameH - 100) / 2;
  const int above = inkIn(with, kHgCenterL, kHgFrameY + 4, kHgCenterR, numberTop - 2);
  TEST_ASSERT_GREATER_THAN(150, above);                                              // the arch is visible in the gap
  TEST_ASSERT_EQUAL(0, inkIn(without, kHgCenterL, kHgFrameY + 4, kHgCenterR, numberTop - 2));  // and absent otherwise
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
  RUN_TEST(test_hourglass_dots_flow_from_left_to_right);
  RUN_TEST(test_hourglass_fields_are_mirror_images_at_half_time);
  RUN_TEST(test_hourglass_start_is_all_left_and_finish_is_all_right);
  RUN_TEST(test_hourglass_does_not_refill_in_the_last_minute);
  RUN_TEST(test_hourglass_number_is_minutes_and_seconds);
  RUN_TEST(test_hourglass_number_fits_for_three_digit_minutes);
  RUN_TEST(test_stream_exists_only_while_sand_is_pouring);
  RUN_TEST(test_stream_arches_over_the_number_from_the_left_field_to_the_right);
  RUN_TEST(test_stream_grains_shrink_as_they_fall);
  RUN_TEST(test_stream_advances_with_time_and_freezes_when_time_stops);
  RUN_TEST(test_stream_phase_is_a_fraction_that_wraps);
  RUN_TEST(test_stream_is_drawn_on_the_canvas_above_the_digits);
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
