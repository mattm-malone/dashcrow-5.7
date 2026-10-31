#include <Arduino.h>
#include "App.h"
#include "Calendar.h"
#include "Canvas.h"
#include "Daylight.h"
#include "Feed.h"
#include "Merge.h"
#include "ClockPage.h"
#include "Inputs.h"
#include "MakeTimerView.h"
#include "Net.h"
#include "PanelDisplay.h"
#include "RefreshPolicy.h"
#include "StandupOverlay.h"
#include "TimerChip.h"
#include "TimelinePage.h"
#include "TimerPage.h"
#include <time.h>
#include "Text.h"
#include "config.h"

static Canvas canvas(kW, kH);
static PanelDisplay panel;
static Inputs inputs;
static Net net;
static Feed feed;
static RefreshPolicy policy;
static App app(StandupConfig{STANDUP_INTERVAL_MIN, STANDUP_START_HOUR, STANDUP_END_HOUR, STANDUP_WEEKDAY_MASK});


#ifdef DIAG_TIMING
// Logs how long full and partial refreshes take, for tuning kPartialsBeforeFull and cadence.
static void runTimingDiag() {
  canvas.fillScreen(0);
  renderClock(canvas, {true, 10, 0, 1, 30, 10});
  panel.show(canvas, RefreshKind::Full);
  Serial.printf("full (clear pass): %u ms\n", panel.lastRefreshMs());
  panel.setClearPass(false);
  renderClock(canvas, {true, 10, 1, 1, 30, 10});
  panel.show(canvas, RefreshKind::Full);
  Serial.printf("full (no clear pass): %u ms\n", panel.lastRefreshMs());
  for (int i = 2; i <= 11; i++) {
    renderClock(canvas, {true, 10, uint8_t(i), 1, 30, 10});
    panel.show(canvas, RefreshKind::Partial);
    Serial.printf("partial %d: %lu ms (copy %lu, send %lu, waveform %lu, old %lu)\n", i, (unsigned long)panel.lastRefreshMs(),
                  (unsigned long)panel.copyMs(), (unsigned long)panel.sendMs(), (unsigned long)panel.updateMs(),
                  (unsigned long)panel.oldMs());
  }
  Serial.println("diag done");
}
#endif

static bool timerActive() {
  const auto s = app.timer().state();
  return s == TimerModel::State::Running || s == TimerModel::State::Paused;
}

struct DayInfo {
  uint8_t wday, mday, month;
  int yday;       // 0-based day of year
  double tzHours;  // local UTC offset on that day, DST included
};

// Calendar facts for today + offset days, from the system clock (valid once NTP has synced).
static DayInfo dayInfo(int offset) {
  const time_t t = time(nullptr) + time_t(offset) * 86400;
  tm lt, gt;
  localtime_r(&t, &lt);
  gmtime_r(&t, &gt);
  int diff = (lt.tm_hour * 60 + lt.tm_min) - (gt.tm_hour * 60 + gt.tm_min);
  if (diff > 720) diff -= 1440;
  if (diff < -720) diff += 1440;
  return {uint8_t(lt.tm_wday), uint8_t(lt.tm_mday), uint8_t(lt.tm_mon + 1), lt.tm_yday, diff / 60.0};
}

// "YYYY-MM-DD" for local today, or nullptr until the clock is valid.
static const char *todayIso(const LocalTime &t) {
  static char buf[11];
  if (!t.valid) return nullptr;
  const time_t now = time(nullptr);
  tm lt;
  localtime_r(&now, &lt);
  strftime(buf, sizeof buf, "%Y-%m-%d", &lt);
  return buf;
}

static void renderTimelinePage(const LocalTime &t) {
  static DayEvent events[16], config[16], fromFeed[16];
  const DayInfo d = dayInfo(app.dayOffset());
  TimelineView v{};
  v.timeValid = t.valid;
  v.hour = t.hour;
  v.minute = t.minute;
  v.wday = d.wday;
  v.mday = d.mday;
  v.month = d.month;
  v.events = events;
  const size_t nConfig = eventsForDay(MEETINGS, sizeof MEETINGS / sizeof MEETINGS[0], d.wday, config, 16);
  // The feed only knows today; other days show config meetings alone.
  const size_t nFeed = app.dayOffset() == 0 ? feed.events(fromFeed, 16, todayIso(t)) : 0;
  v.n = mergeEvents(config, nConfig, fromFeed, nFeed, events, 16);
  v.window = {DAY_START_MIN, DAY_END_MIN};
  v.sun = sunTimes(d.yday + 1, LATITUDE, LONGITUDE, d.tzHours);
  v.dayOffset = app.dayOffset();
  v.feedStale = feed.enabled() && feed.stale(millis());
  renderTimeline(canvas, v);
}

static void render(const LocalTime &t) {
  if (app.overlayActive()) {
    renderStandup(canvas);
    return;
  }
  switch (app.page()) {
    case Page::Clock:
      renderClock(canvas, {t.valid, t.hour, t.minute, t.wday, t.mday, t.month});
      break;
    case Page::Timer:
      renderTimer(canvas, makeTimerView(app.timer(), t));
      return;  // the timer page shows its own state; no chip
    case Page::Timeline:
      renderTimelinePage(t);
      if (timerActive()) drawTimerChip(canvas, app.timer().minutesLeft(), 250);  // clear of the header time
      return;
  }
  if (timerActive()) drawTimerChip(canvas, app.timer().minutesLeft());
}

// What on screen changes with time: Partial refresh when this value changes.
static uint32_t contentKey(const LocalTime &t) {
  const TimerModel &tm = app.timer();
  if (app.overlayActive()) return 0;
  const uint32_t chip = timerActive() ? 1u + uint32_t(tm.minutesLeft()) : 0u;
  switch (app.page()) {
    case Page::Clock: return (t.valid ? 1u + t.hour * 60u + t.minute : 0u) | (chip << 16) | (uint32_t(feed.version()) << 24);
    case Page::Timeline:
      return (t.valid ? 1u + t.hour * 60u + t.minute : 0u) | (uint32_t(app.dayOffset() + 8) << 12) | (chip << 16) | (uint32_t(feed.version()) << 24);
    case Page::Timer: {
      const uint32_t mode = uint32_t(tm.state()) + 1u;
      if (tm.state() == TimerModel::State::Setting) return (mode << 28) | uint32_t(tm.setMinutes());
      if (tm.state() == TimerModel::State::Done) return mode << 28;
      // Bar redraws every 8 px (~16 s on a 25-minute timer); digits every minute; per second in the last minute.
      const uint32_t step = tm.inLastMinute() ? (tm.remainingMs() + 999) / 1000
                                              : uint32_t(timerBarFilledPx(tm.remainingMs(), tm.totalMs(), kBarW) / 8);
      return (mode << 28) | (uint32_t(tm.minutesLeft()) << 16) | step;
    }
  }
  return 0;
}

void setup() {
  Serial.begin(115200);
  panel.begin();
  inputs.begin();
  net.begin();
  feed.begin(FEED_URL, FEED_REFRESH_MIN * 60UL * 1000UL);
#ifdef DIAG_TIMING
  runTimingDiag();
#endif
}

void loop() {
  const uint32_t ms = millis();
  const LocalTime t = net.now();

  InputEvent ev;
  while (inputs.poll(ev, ms)) {
    Serial.printf("input: %s %d\n", ev.type == InputEvent::Type::Press ? "press" : "turn",
                  ev.type == InputEvent::Type::Press ? int(ev.button) : ev.dir);
    // Use the time the press happened, not now: presses queue up during slow refreshes.
    app.onInput(ev, ev.ms, t);
  }
  app.tick(ms, t);
  feed.poll(ms, todayIso(t));

  // No settle delay: an input redraws immediately, and presses made while the panel is busy are queued
  // by the input ISR and applied together on the next pass, so the refresh itself batches them.
  {
    const uint32_t layoutKey = uint32_t(app.page()) | (app.overlayActive() ? 0x100u : 0u);
    const RefreshKind kind = policy.next(contentKey(t), layoutKey);
    if (kind != RefreshKind::None) {
      render(t);
      panel.show(canvas, kind);
      Serial.printf("refresh %s %lu ms (copy %lu, send %lu, waveform %lu, old %lu)\n", kind == RefreshKind::Full ? "full" : "partial",
                    (unsigned long)panel.lastRefreshMs(), (unsigned long)panel.copyMs(), (unsigned long)panel.sendMs(),
                    (unsigned long)panel.updateMs(), (unsigned long)panel.oldMs());
    }
  }
  delay(10);
}
