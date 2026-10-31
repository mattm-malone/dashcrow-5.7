#include "MakeTimerView.h"

namespace {
TimerView::Mode modeOf(TimerModel::State s) {
  switch (s) {
    case TimerModel::State::Running: return TimerView::Mode::Running;
    case TimerModel::State::Paused: return TimerView::Mode::Paused;
    case TimerModel::State::Done: return TimerView::Mode::Done;
    default: return TimerView::Mode::Setting;
  }
}
}  // namespace

TimerView makeTimerView(const TimerModel &timer, const LocalTime &now) {
  TimerView v{};
  v.mode = modeOf(timer.state());
  v.setMinutes = timer.setMinutes();
  v.minutesLeft = timer.minutesLeft();
  v.remainingMs = timer.remainingMs();
  v.totalMs = timer.totalMs();
  v.secondsLeft = int((v.remainingMs + 999) / 1000);
  v.endValid = now.valid;
  if (now.valid) {
    const uint32_t nowSec = uint32_t(now.hour) * 3600u + now.minute * 60u + now.second;
    const uint32_t endMin = ((nowSec + (v.remainingMs + 999) / 1000) / 60) % 1440u;
    v.endHour = uint8_t(endMin / 60);
    v.endMinute = uint8_t(endMin % 60);
  }
  return v;
}
