#pragma once
#include "Accel.h"
#include "StandupScheduler.h"
#include "TimerModel.h"
#include "Types.h"

class App {
 public:
  explicit App(StandupConfig cfg = {}) : standup_(cfg) {}

  void onInput(const InputEvent &ev, uint32_t nowMs, const LocalTime &now);
  void tick(uint32_t nowMs, const LocalTime &now);

  Page page() const { return page_; }
  bool overlayActive() const { return overlay_; }
  const TimerModel &timer() const { return timer_; }

 private:
  Page page_ = Page::Clock;
  bool overlay_ = false;
  TimerModel timer_;
  StandupScheduler standup_;
  RotaryAccel accel_;
};
