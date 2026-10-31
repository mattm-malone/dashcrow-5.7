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

// ---- Halftone hourglass ----

static int leftBulbInk(Canvas &c) { return inkIn(c, kHgLeftX - 14, kHgMid - kHgCapHalf - 8, kHgCenterL, kHgMid + kHgCapHalf + 8); }
static int rightBulbInk(Canvas &c) { return inkIn(c, kHgCenterR, kHgMid - kHgCapHalf - 8, kHgRightX + 14, kHgMid + kHgCapHalf + 8); }

// The last minute inverts the whole canvas; count ink after undoing that.
static int invertedInk(Canvas &c, int x0, int y0, int x1, int y1) {
  int n = 0;
  for (int y = y0; y < y1; y++)
    for (int x = x0; x < x1; x++) n += !c.getPixel(x, y);
  return n;
}

static int cellInk(Canvas &c, bool right, int col, int row) {
  int x, y;
  hourglassDotCenter(right, col, row, x, y);
  return inkIn(c, x - 6, y - 6, x + 7, y + 7);
}

void test_dot_size_falls_steadily_from_the_edge_to_the_neck() {
  TEST_ASSERT_GREATER_OR_EQUAL(5, hourglassDotRadius(0));
  TEST_ASSERT_LESS_OR_EQUAL(2, hourglassDotRadius(kHgCols - 1));
  TEST_ASSERT_GREATER_OR_EQUAL(2 * hourglassDotRadius(kHgCols - 1), hourglassDotRadius(0));
  for (int col = 1; col < kHgCols; col++) TEST_ASSERT_LESS_OR_EQUAL(hourglassDotRadius(col - 1), hourglassDotRadius(col));
}

void test_the_bulbs_narrow_to_a_single_row_at_the_neck() {
  TEST_ASSERT_GREATER_OR_EQUAL(13, hourglassRows(0));
  TEST_ASSERT_EQUAL(1, hourglassRows(kHgCols - 1));
  for (int col = 1; col < kHgCols; col++) TEST_ASSERT_LESS_OR_EQUAL(hourglassRows(col - 1), hourglassRows(col));
}

void test_dots_sit_on_a_regular_grid_sized_only_by_column() {
  Canvas c(kW, kH);
  renderTimer(c, setting(TimerStyle::Hourglass, 25));  // full left bulb
  // Within a column every dot is the same size...
  TEST_ASSERT_GREATER_THAN(0, cellInk(c, false, 3, 4));
  TEST_ASSERT_EQUAL(cellInk(c, false, 3, 4), cellInk(c, false, 3, 6));
  TEST_ASSERT_EQUAL(cellInk(c, false, 3, 4), cellInk(c, false, 3, 8));
  // ...and across columns dots shrink steadily toward the neck (columns kept clear of the number).
  int prev = 1 << 30;
  for (int col : {0, 5, 10, 16}) {  // radii 6, 5, 4, 3 px
    const int ink = cellInk(c, false, col, hourglassRows(col) / 2);
    TEST_ASSERT_LESS_THAN(prev, ink);
    prev = ink;
  }
}

void test_hourglass_left_bulb_drains_while_right_bulb_fills() {
  // Each bulb is only measurable where it is changing (outside the number): the left once the sand has receded
  // past the middle, the right until its pile has reached the middle. So use different stages for each.
  int prev = 1 << 30;
  for (uint32_t r : {12 * kMin, 9 * kMin, 6 * kMin, 3 * kMin, 1 * kMin + 5000}) {
    Canvas c(kW, kH);
    renderTimer(c, running(TimerStyle::Hourglass, 25, r));
    TEST_ASSERT_LESS_THAN(prev, leftBulbInk(c));
    prev = leftBulbInk(c);
  }
  prev = -1;
  for (uint32_t r : {22 * kMin, 20 * kMin, 18 * kMin, 16 * kMin, 14 * kMin}) {
    Canvas c(kW, kH);
    renderTimer(c, running(TimerStyle::Hourglass, 25, r));
    TEST_ASSERT_GREATER_THAN(prev, rightBulbInk(c));
    prev = rightBulbInk(c);
  }
}

void test_hourglass_bulbs_mirror_each_other_at_half_time() {
  Canvas c(kW, kH);
  renderTimer(c, running(TimerStyle::Hourglass, 25, uint32_t(12.5f * kMin)));
  TEST_ASSERT_FLOAT_WITHIN(0.08f, 1.0f, float(leftBulbInk(c)) / float(rightBulbInk(c)));
}

void test_hourglass_start_is_all_left_and_the_end_is_all_right() {
  Canvas start(kW, kH), nearEnd(kW, kH);
  renderTimer(start, setting(TimerStyle::Hourglass, 25));
  renderTimer(nearEnd, running(TimerStyle::Hourglass, 180, 61000));  // a 3-hour timer a minute from done (not yet inverted)
  // (Every window includes the same glass outline, so compare against that rather than against zero.)
  TEST_ASSERT_LESS_THAN(leftBulbInk(start) / 3, leftBulbInk(nearEnd));
  TEST_ASSERT_GREATER_THAN(rightBulbInk(start) + 5000, rightBulbInk(nearEnd));
}

void test_hourglass_does_not_refill_in_the_last_minute() {
  // 30 s left of 25 minutes: the sand is almost all in the right bulb; the picture must not be rescaled to the minute.
  Canvas start(kW, kH), last(kW, kH);
  renderTimer(start, setting(TimerStyle::Hourglass, 25));
  renderTimer(last, running(TimerStyle::Hourglass, 25, 30000));
  TEST_ASSERT_LESS_THAN(leftBulbInk(start) / 3, invertedInk(last, kHgLeftX - 14, kHgMid - kHgCapHalf - 8, kHgCenterL, kHgMid + kHgCapHalf + 8));
}

void test_the_shape_is_an_hourglass_with_empty_corners_above_and_below_the_neck() {
  for (uint32_t r : {25 * kMin, 12 * kMin, 2 * kMin}) {
    Canvas c(kW, kH);
    renderTimer(c, running(TimerStyle::Hourglass, 25, r));
    TEST_ASSERT_EQUAL(0, inkIn(c, kHgCenterL + 30, 48, kHgCenterR - 30, kHgMid - kHgDigitHeight / 2 - 8));      // above the neck
    TEST_ASSERT_EQUAL(0, inkIn(c, kHgCenterL + 30, kHgMid + kHgDigitHeight / 2 + 8, kHgCenterR - 30, kH));     // below it
  }
  Canvas full(kW, kH);
  renderTimer(full, setting(TimerStyle::Hourglass, 25));
  TEST_ASSERT_GREATER_THAN(1000, inkIn(full, kHgLeftX - 14, kHgMid - kHgCapHalf, kHgLeftX + 30, kHgMid + kHgCapHalf));  // wide at the end
}

void test_neck_grains_run_along_the_centre_line_between_the_sand_edges() {
  StreamDot g[kStreamDots];
  const int n = hourglassStream(running(TimerStyle::Hourglass, 25, 12 * kMin + 30000), g, kStreamDots);  // half way
  TEST_ASSERT_GREATER_THAN(4, n);
  for (int i = 0; i < n; i++) {
    TEST_ASSERT_EQUAL(kHgMid, g[i].y);
    TEST_ASSERT_GREATER_OR_EQUAL(2, g[i].r);
    TEST_ASSERT_GREATER_THAN(kHgLeftX + 14 * kHgPitch, g[i].x);   // clear of the sand left in the left bulb (14 columns at half time)
    TEST_ASSERT_LESS_THAN(kHgRightX - 14 * kHgPitch, g[i].x);     // and of the pile in the right one
    if (i > 0) TEST_ASSERT_GREATER_THAN(g[i - 1].x, g[i].x);
  }
}

void test_neck_grains_exist_only_while_sand_is_pouring() {
  StreamDot g[kStreamDots];
  TEST_ASSERT_EQUAL(0, hourglassStream(setting(TimerStyle::Hourglass, 25), g, kStreamDots));
  TEST_ASSERT_EQUAL(0, hourglassStream(running(TimerStyle::Hourglass, 25, 25 * kMin), g, kStreamDots));  // not started
  TEST_ASSERT_EQUAL(0, hourglassStream(running(TimerStyle::Hourglass, 25, 0), g, kStreamDots));           // all poured
  TEST_ASSERT_EQUAL(0, hourglassStream(running(TimerStyle::Bar, 25, 12 * kMin), g, kStreamDots));         // other styles
}

void test_neck_grains_move_one_column_per_second() {
  StreamDot a[kStreamDots], b[kStreamDots];
  const uint32_t t = 12 * kMin + 30000;
  const int na = hourglassStream(running(TimerStyle::Hourglass, 25, t), a, kStreamDots);
  const int nb = hourglassStream(running(TimerStyle::Hourglass, 25, t - 1000), b, kStreamDots);  // one second later
  TEST_ASSERT_GREATER_THAN(2, na);
  TEST_ASSERT_GREATER_THAN(2, nb);
  int neckLeft, neckRight, y;
  hourglassDotCenter(false, kHgCols - 1, 0, neckLeft, y);
  hourglassDotCenter(true, kHgCols - 1, 0, neckRight, y);
  for (int i = 0; i < na; i++) {
    // Each grain advances to the next column on the right; one crossing the neck jumps from the left bulb's last
    // column to the right bulb's.
    const int expected = a[i].x == neckLeft ? neckRight : a[i].x + kHgPitch;
    bool found = false;
    for (int j = 0; j < nb; j++) found |= b[j].x == expected;
    if (expected < kHgRightX - 14 * kHgPitch) TEST_ASSERT_TRUE_MESSAGE(found, "a grain did not advance one column");
  }
}

void test_neck_grains_freeze_when_time_stops() {
  StreamDot a[kStreamDots], b[kStreamDots];
  const int na = hourglassStream(running(TimerStyle::Hourglass, 25, 10 * kMin), a, kStreamDots);
  const int nb = hourglassStream(running(TimerStyle::Hourglass, 25, 10 * kMin), b, kStreamDots);
  TEST_ASSERT_EQUAL(na, nb);
  for (int i = 0; i < na; i++) TEST_ASSERT_EQUAL(a[i].x, b[i].x);
}

void test_hourglass_number_is_minutes_and_seconds() {
  Canvas a(kW, kH), b(kW, kH), same(kW, kH);
  renderTimer(a, running(TimerStyle::Hourglass, 25, 12 * kMin + 5000));
  renderTimer(b, running(TimerStyle::Hourglass, 25, 12 * kMin + 4000));  // one second later
  renderTimer(same, running(TimerStyle::Hourglass, 25, 12 * kMin + 5000));
  bool differs = false, identical = true;
  for (int y = kHgMid - kHgDigitHeight / 2; y < kHgMid + kHgDigitHeight / 2; y++)
    for (int x = kHgCenterL; x < kHgCenterR; x++) {
      if (a.getPixel(x, y) != b.getPixel(x, y)) differs = true;
      if (a.getPixel(x, y) != same.getPixel(x, y)) identical = false;
    }
  TEST_ASSERT_TRUE(differs);    // the seconds digits changed
  TEST_ASSERT_TRUE(identical);  // and rendering is deterministic
}

void test_hourglass_three_digit_minutes_still_render_in_the_middle() {
  Canvas c(kW, kH);
  renderTimer(c, running(TimerStyle::Hourglass, 180, 125 * kMin));
  TEST_ASSERT_GREATER_THAN(500, inkIn(c, kHgCenterL, kHgMid - 50, kHgCenterR, kHgMid + 50));
}

// ---- Dithered hourglass ----

static float bandDensity(Canvas &c, int x0, int x1) {  // share of black pixels in a band along the centre line
  const int y0 = kHgMid - 15, y1 = kHgMid + 15;
  return float(inkIn(c, x0, y0, x1, y1)) / float((x1 - x0) * (y1 - y0));
}

void test_sand_density_runs_from_solid_to_empty() {
  for (float d : {0.0f, 100.0f, 376.0f}) {
    TEST_ASSERT_TRUE(sandDensity(d, 1.0f) == 1.0f);   // a full bulb is solid right up to the neck
    TEST_ASSERT_TRUE(sandDensity(d, 0.0f) == 0.0f);   // an empty one has nothing
  }
  // Along the bulb the density never rises, and it fades over about kSandFade pixels.
  float prev = 1.0f;
  int fadeStart = -1, fadeEnd = -1;
  for (int d = 0; d <= kSandBulb; d++) {
    const float x = sandDensity(float(d), 0.4f);
    TEST_ASSERT_TRUE(x <= prev);
    if (x < 1.0f && fadeStart < 0) fadeStart = d;
    if (x <= 0.0f && fadeEnd < 0) fadeEnd = d;
    prev = x;
  }
  TEST_ASSERT_GREATER_THAN(0, fadeStart);
  TEST_ASSERT_INT_WITHIN(3, int(kSandFade), fadeEnd - fadeStart);
  // More sand = more density at any position.
  for (float d : {20.0f, 150.0f, 300.0f}) {
    float last = -1.0f;
    for (float f : {0.0f, 0.2f, 0.4f, 0.6f, 0.8f, 1.0f}) {
      TEST_ASSERT_TRUE(sandDensity(d, f) >= last);
      last = sandDensity(d, f);
    }
  }
}

void test_dither_full_bulb_is_solid_and_the_far_bulb_is_empty() {
  Canvas c(kW, kH);
  renderTimer(c, setting(TimerStyle::Dither, 25));
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.0f, bandDensity(c, 40, 180));        // left bulb solid
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, bandDensity(c, kW - 180, kW - 40));  // right bulb empty
}

void test_dither_fades_with_a_real_dither_pattern() {
  Canvas c(kW, kH);
  renderTimer(c, running(TimerStyle::Dither, 25, uint32_t(0.3f * 25 * kMin)));  // left bulb 30% full
  const float solid = bandDensity(c, kHgLeftX + 4, kHgLeftX + 24);
  const float fade = bandDensity(c, kHgLeftX + 90, kHgLeftX + 110);
  const float empty = bandDensity(c, 196, 236);
  TEST_ASSERT_TRUE(solid > 0.97f);
  TEST_ASSERT_TRUE(fade > 0.15f && fade < 0.85f);   // neither solid nor empty: it is dithered
  TEST_ASSERT_TRUE(empty < 0.08f);   // no sand here; only the pouring grains cross it (about 4% of the band)
  // And the dither mixes both colours locally instead of making stripes: a small block holds black and white.
  int black = 0, white = 0;
  for (int y = kHgMid - 6; y < kHgMid + 6; y++)
    for (int x = kHgLeftX + 90; x < kHgLeftX + 102; x++) (c.getPixel(x, y) ? black : white)++;
  TEST_ASSERT_GREATER_THAN(10, black);
  TEST_ASSERT_GREATER_THAN(10, white);
}

void test_dither_left_empties_and_right_fills_as_time_passes() {
  int prev = 1 << 30;
  for (uint32_t r : {12 * kMin, 9 * kMin, 6 * kMin, 3 * kMin, 1 * kMin + 5000}) {
    Canvas c(kW, kH);
    renderTimer(c, running(TimerStyle::Dither, 25, r));
    TEST_ASSERT_LESS_THAN(prev, leftBulbInk(c));
    prev = leftBulbInk(c);
  }
  prev = -1;
  for (uint32_t r : {22 * kMin, 20 * kMin, 18 * kMin, 16 * kMin, 14 * kMin}) {
    Canvas c(kW, kH);
    renderTimer(c, running(TimerStyle::Dither, 25, r));
    TEST_ASSERT_GREATER_THAN(prev, rightBulbInk(c));
    prev = rightBulbInk(c);
  }
}

void test_dither_bulbs_mirror_each_other_at_half_time() {
  Canvas c(kW, kH);
  renderTimer(c, running(TimerStyle::Dither, 25, uint32_t(12.5f * kMin)));
  TEST_ASSERT_FLOAT_WITHIN(0.08f, 1.0f, float(leftBulbInk(c)) / float(rightBulbInk(c)));
}

void test_dither_shape_leaves_the_corners_empty() {
  for (uint32_t r : {25 * kMin, 12 * kMin, 2 * kMin}) {
    Canvas c(kW, kH);
    renderTimer(c, running(TimerStyle::Dither, 25, r));
    TEST_ASSERT_EQUAL(0, inkIn(c, kHgCenterL + 30, 48, kHgCenterR - 30, kHgMid - kHgDigitHeight / 2 - 8));
    TEST_ASSERT_EQUAL(0, inkIn(c, kHgCenterL + 30, kHgMid + kHgDigitHeight / 2 + 8, kHgCenterR - 30, kH));
  }
}

void test_dither_number_sits_on_a_clean_white_plate() {
  // At the start the left bulb is solid right up to the neck. The 100 px MM:SS number starts at x = 249 and gets a
  // plate 12 px wider on each side (from x = 237), so just left of the digits is bare glass, while sand is solid
  // outside the plate.
  Canvas c(kW, kH);
  renderTimer(c, setting(TimerStyle::Dither, 25));
  TEST_ASSERT_EQUAL(0, c.getPixel(243, kHgMid));
  TEST_ASSERT_EQUAL(0, c.getPixel(243, kHgMid - 20));
  TEST_ASSERT_EQUAL(1, c.getPixel(228, kHgMid));
  TEST_ASSERT_EQUAL(1, c.getPixel(228, kHgMid - 20));
}

void test_dither_does_not_refill_in_the_last_minute() {
  Canvas start(kW, kH), last(kW, kH);
  renderTimer(start, setting(TimerStyle::Dither, 25));
  renderTimer(last, running(TimerStyle::Dither, 25, 30000));
  TEST_ASSERT_LESS_THAN(leftBulbInk(start) / 3, invertedInk(last, kHgLeftX - 14, kHgMid - kHgCapHalf - 8, kHgCenterL, kHgMid + kHgCapHalf + 8));
}

void test_dither_grains_run_between_the_two_fades() {
  StreamDot g[kStreamDots];
  const int n = hourglassStream(running(TimerStyle::Dither, 25, 12 * kMin + 30000), g, kStreamDots);  // wrong function for this style
  TEST_ASSERT_EQUAL(0, n);
  const int m = ditherStream(running(TimerStyle::Dither, 25, 12 * kMin + 30000), g, kStreamDots);
  TEST_ASSERT_GREATER_THAN(3, m);
  for (int i = 0; i < m; i++) {
    TEST_ASSERT_EQUAL(kHgMid, g[i].y);
    TEST_ASSERT_EQUAL(2, g[i].r);
    // Half time: left fade ends at 188 + 60 = 248 px from the left edge; the right pile mirrors it.
    TEST_ASSERT_GREATER_THAN(kHgLeftX + 188 + 60 - 1, g[i].x);
    TEST_ASSERT_LESS_THAN(kHgRightX - 188 - 60 + 1, g[i].x);
    if (i > 0) TEST_ASSERT_EQUAL(kSandGrainPitch, g[i].x - g[i - 1].x);
  }
}

void test_dither_grains_exist_only_while_pouring_and_advance_each_second() {
  StreamDot a[kStreamDots], b[kStreamDots];
  TEST_ASSERT_EQUAL(0, ditherStream(setting(TimerStyle::Dither, 25), a, kStreamDots));
  TEST_ASSERT_EQUAL(0, ditherStream(running(TimerStyle::Dither, 25, 25 * kMin), a, kStreamDots));
  TEST_ASSERT_EQUAL(0, ditherStream(running(TimerStyle::Dither, 25, 0), a, kStreamDots));
  TEST_ASSERT_EQUAL(0, ditherStream(running(TimerStyle::Hourglass, 25, 12 * kMin), a, kStreamDots));
  const uint32_t t = 12 * kMin + 30000;
  const int na = ditherStream(running(TimerStyle::Dither, 25, t), a, kStreamDots);
  const int nb = ditherStream(running(TimerStyle::Dither, 25, t - 1000), b, kStreamDots);  // one second later
  TEST_ASSERT_GREATER_THAN(3, nb);
  // Same lattice, shifted right by kSandGrainStep (mod the pitch): each grain at s+1 sits 4 px to the right of one at s.
  for (int j = 1; j < nb; j++) {
    bool found = false;
    for (int i = 0; i < na; i++) found |= a[i].x + kSandGrainStep == b[j].x;
    TEST_ASSERT_TRUE_MESSAGE(found, "a grain did not advance by the step");
  }
}

void test_dither_grains_freeze_when_time_stops() {
  StreamDot a[kStreamDots], b[kStreamDots];
  const int na = ditherStream(running(TimerStyle::Dither, 25, 10 * kMin), a, kStreamDots);
  const int nb = ditherStream(running(TimerStyle::Dither, 25, 10 * kMin), b, kStreamDots);
  TEST_ASSERT_EQUAL(na, nb);
  for (int i = 0; i < na; i++) TEST_ASSERT_EQUAL(a[i].x, b[i].x);
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
  for (TimerStyle s : {TimerStyle::Bar, TimerStyle::Hourglass, TimerStyle::Dither, TimerStyle::Disc, TimerStyle::Dots}) {
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
  for (TimerStyle s : {TimerStyle::Bar, TimerStyle::Hourglass, TimerStyle::Dither, TimerStyle::Disc, TimerStyle::Dots}) {
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
  RUN_TEST(test_dot_size_falls_steadily_from_the_edge_to_the_neck);
  RUN_TEST(test_the_bulbs_narrow_to_a_single_row_at_the_neck);
  RUN_TEST(test_dots_sit_on_a_regular_grid_sized_only_by_column);
  RUN_TEST(test_hourglass_left_bulb_drains_while_right_bulb_fills);
  RUN_TEST(test_hourglass_bulbs_mirror_each_other_at_half_time);
  RUN_TEST(test_hourglass_start_is_all_left_and_the_end_is_all_right);
  RUN_TEST(test_hourglass_does_not_refill_in_the_last_minute);
  RUN_TEST(test_the_shape_is_an_hourglass_with_empty_corners_above_and_below_the_neck);
  RUN_TEST(test_neck_grains_run_along_the_centre_line_between_the_sand_edges);
  RUN_TEST(test_neck_grains_exist_only_while_sand_is_pouring);
  RUN_TEST(test_neck_grains_move_one_column_per_second);
  RUN_TEST(test_neck_grains_freeze_when_time_stops);
  RUN_TEST(test_hourglass_number_is_minutes_and_seconds);
  RUN_TEST(test_hourglass_three_digit_minutes_still_render_in_the_middle);
  RUN_TEST(test_sand_density_runs_from_solid_to_empty);
  RUN_TEST(test_dither_full_bulb_is_solid_and_the_far_bulb_is_empty);
  RUN_TEST(test_dither_fades_with_a_real_dither_pattern);
  RUN_TEST(test_dither_left_empties_and_right_fills_as_time_passes);
  RUN_TEST(test_dither_bulbs_mirror_each_other_at_half_time);
  RUN_TEST(test_dither_shape_leaves_the_corners_empty);
  RUN_TEST(test_dither_number_sits_on_a_clean_white_plate);
  RUN_TEST(test_dither_does_not_refill_in_the_last_minute);
  RUN_TEST(test_dither_grains_run_between_the_two_fades);
  RUN_TEST(test_dither_grains_exist_only_while_pouring_and_advance_each_second);
  RUN_TEST(test_dither_grains_freeze_when_time_stops);
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
