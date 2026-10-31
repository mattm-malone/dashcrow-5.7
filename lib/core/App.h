#pragma once
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
  int dayOffset() const { return dayOffset_; }  // Timeline: days from today, -7..+7

 private:
  Page page_ = Page::Clock;
  bool overlay_ = false;
  int dayOffset_ = 0;
  TimerModel timer_;
  StandupScheduler standup_;
};
