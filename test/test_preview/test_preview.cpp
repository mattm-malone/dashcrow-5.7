// Renders every page with sample data to build/preview/<page>.pbm so layouts can
// be inspected without flashing: pio test -e native -f test_preview
// then: uv run --with pillow tools/pbm2png.py build/preview/clock.pbm build/preview/clock.png
#include <unity.h>
#include <filesystem>
#include <fstream>
#include "ClockPage.h"
#include "TimelinePage.h"
#include "TimerPage.h"
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
  RUN_TEST(test_preview_timeline);
  return UNITY_END();
}
