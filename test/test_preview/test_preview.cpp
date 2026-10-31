// Renders every page with sample data to build/preview/<page>.pbm so layouts can
// be inspected without flashing: pio test -e native -f test_preview
// then: uv run --with pillow tools/pbm2png.py build/preview/clock.pbm build/preview/clock.png
#include <unity.h>
#include <filesystem>
#include <fstream>
#include "ClockPage.h"
#include "TimelinePage.h"
#include "TimerPage.h"
#include <stdio.h>
#include <string.h>

void setUp() {}
void tearDown() {}

static bool writePbm(const char *name, Canvas &c) {
  std::filesystem::create_directories("build/preview");
  std::ofstream f(std::string("build/preview/") + name + ".pbm", std::ios::binary);
  f << "P4\n" << c.width() << " " << c.height() << "\n";
  f.write(reinterpret_cast<const char *>(c.getBuffer()), ((c.width() + 7) / 8) * c.height());
  return f.good();
}

void test_preview_clock() {
  Canvas c(kW, kH);
  c.fillScreen(0);
  renderClock(c, {true, 14, 41, 4, 30, 10});
  TEST_ASSERT_TRUE(writePbm("clock", c));
}

static TimerView runView(uint32_t totalMin, uint32_t remainingMs) {
  TimerView v{};
  v.mode = TimerView::Mode::Running;
  v.totalMs = totalMin * 60000;
  v.remainingMs = remainingMs;
  v.minutesLeft = int((remainingMs + 59999) / 60000);
  v.secondsLeft = int((remainingMs + 999) / 1000);
  v.endValid = true;
  v.endHour = 15;
  v.endMinute = 25;
  return v;
}

void test_preview_timer_pages() {
  Canvas c(kW, kH);
  TimerView s{};
  s.mode = TimerView::Mode::Setting;
  s.setMinutes = 25;
  renderTimer(c, s);
  TEST_ASSERT_TRUE(writePbm("timer_setting", c));
  renderTimer(c, runView(25, 14 * 60000 + 20000));
  TEST_ASSERT_TRUE(writePbm("timer_running", c));
  renderTimer(c, runView(90, 71 * 60000));
  TEST_ASSERT_TRUE(writePbm("timer_running_long", c));
  renderTimer(c, runView(25, 38000));
  TEST_ASSERT_TRUE(writePbm("timer_last_minute", c));
  TimerView d = runView(25, 0);
  d.mode = TimerView::Mode::Done;
  renderTimer(c, d);
  TEST_ASSERT_TRUE(writePbm("timer_done", c));
}

// Each timer style at several stages of a 25-minute timer, for tools/montage.py contact sheets.
void test_preview_timer_styles() {
  struct Stage { const char *name; uint32_t remainingMs; bool setting; };
  const Stage stages[] = {{"0_setting", 0, true},       {"1_start", 25 * 60000u, false},   {"2_75pct", 18 * 60000u + 45000u, false},
                          {"3_50pct", 12 * 60000u + 30000u, false}, {"4_25pct", 6 * 60000u + 15000u, false}, {"5_90s", 90000u, false},
                          {"6_40s", 40000u, false},      {"7_8s", 8000u, false}};
  const struct { const char *name; TimerStyle style; } styles[] = {
      {"hourglass", TimerStyle::Hourglass}, {"dither", TimerStyle::Dither}, {"disc", TimerStyle::Disc}, {"dots", TimerStyle::Dots}};
  Canvas c(kW, kH);
  for (const auto &st : styles) {
    for (const Stage &g : stages) {
      TimerView v = g.setting ? TimerView{} : runView(25, g.remainingMs);
      v.style = st.style;
      if (g.setting) {
        v.mode = TimerView::Mode::Setting;
        v.setMinutes = 25;
      }
      renderTimer(c, v);
      char name[64];
      snprintf(name, sizeof name, "style_%s_%s", st.name, g.name);
      TEST_ASSERT_TRUE(writePbm(name, c));
    }
  }
}

// Consecutive hourglass frames half a second apart, to judge the pouring motion.
void test_preview_hourglass_flow() {
  const uint32_t base = 12 * 60000u + 30000u;  // 12:30 left of 25 minutes
  Canvas c(kW, kH);
  for (int i = 0; i < 4; i++) {
    TimerView v = runView(25, base - i * 500u);
    v.style = TimerStyle::Hourglass;
    renderTimer(c, v);
    char name[48];
    snprintf(name, sizeof name, "flow_%d", i);
    TEST_ASSERT_TRUE(writePbm(name, c));
  }
  TimerView v = runView(25, 4 * 60000u);
  v.style = TimerStyle::Hourglass;
  renderTimer(c, v);
  TEST_ASSERT_TRUE(writePbm("flow_late", c));
  v = runView(25, 22 * 60000u);
  v.style = TimerStyle::Hourglass;
  renderTimer(c, v);
  TEST_ASSERT_TRUE(writePbm("flow_early", c));
}

static DayEvent ev(const char *t, int a, int b) {
  DayEvent e{};
  strncpy(e.title, t, 31);
  e.startMin = a;
  e.endMin = b;
  return e;
}

void test_preview_timeline() {
  const DayEvent e[] = {ev("Standup", 540, 555), ev("Design review", 600, 660), ev("1:1", 780, 810),
                        ev("Design sync", 900, 930), ev("Retro", 960, 1020)};
  TimelineView v{};
  v.timeValid = true;
  v.hour = 14;
  v.minute = 41;
  v.wday = 4;
  v.mday = 30;
  v.month = 10;
  v.events = e;
  v.n = 5;
  v.window = {480, 1140};
  v.sun = sunTimes(303, 40.7, -74.0, -4.0);
  Canvas c(kW, kH);
  renderTimeline(c, v);
  TEST_ASSERT_TRUE(writePbm("timeline", c));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_preview_clock);
  RUN_TEST(test_preview_timer_pages);
  RUN_TEST(test_preview_timer_styles);
  RUN_TEST(test_preview_hourglass_flow);
  RUN_TEST(test_preview_timeline);
  return UNITY_END();
}
