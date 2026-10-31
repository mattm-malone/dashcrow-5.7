#include <Arduino.h>
#include "App.h"
#include "Canvas.h"
#include "ClockPage.h"
#include "Inputs.h"
#include "Net.h"
#include "PanelDisplay.h"
#include "RefreshPolicy.h"
#include "config.h"

static Canvas canvas(kW, kH);
static PanelDisplay panel;
static Inputs inputs;
static Net net;
static RefreshPolicy policy;
static App app(StandupConfig{STANDUP_INTERVAL_MIN, STANDUP_START_HOUR, STANDUP_END_HOUR, STANDUP_WEEKDAY_MASK});

#ifdef DIAG_TIMING
// Logs how long full and partial refreshes take, for tuning kPartialsBeforeFull and cadence.
static void runTimingDiag() {
  canvas.fillScreen(0);
  renderClock(canvas, {true, 10, 0, 1, 30, 10});
  panel.show(canvas, RefreshKind::Full);
  Serial.printf("full: %u ms\n", panel.lastRefreshMs());
  for (int i = 1; i <= 10; i++) {
    renderClock(canvas, {true, 10, uint8_t(i), 1, 30, 10});
    panel.show(canvas, RefreshKind::Partial);
    Serial.printf("partial %d: %u ms\n", i, panel.lastRefreshMs());
  }
  Serial.println("diag done");
}
#endif

static void renderPlaceholder(const char *name) {
  canvas.fillScreen(0);
  canvas.setTextColor(1);
  canvas.setTextSize(4);
  canvas.setCursor(20, 100);
  canvas.print(name);
}

static void render(const LocalTime &t) {
  switch (app.page()) {
    case Page::Clock:
      renderClock(canvas, {t.valid, t.hour, t.minute, t.wday, 0, 0});
      break;
    case Page::Timer: renderPlaceholder("TIMER"); break;
    case Page::Timeline: renderPlaceholder("TIMELINE"); break;
  }
}

void setup() {
  Serial.begin(115200);
  panel.begin();
  inputs.begin();
  net.begin();
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
    app.onInput(ev, ms, t);
  }
  app.tick(ms, t);

  const uint32_t layoutKey = uint32_t(app.page()) | (app.overlayActive() ? 0x100u : 0u);
  const uint32_t contentKey = t.valid ? (1u + t.hour * 60u + t.minute) : 0u;
  const RefreshKind kind = policy.next(contentKey, layoutKey);
  if (kind != RefreshKind::None) {
    render(t);
    panel.show(canvas, kind);
    Serial.printf("refresh %s %u ms\n", kind == RefreshKind::Full ? "full" : "partial", panel.lastRefreshMs());
  }
  delay(10);
}
