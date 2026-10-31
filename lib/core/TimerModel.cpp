#include "TimerModel.h"

namespace {
constexpr int kMinMinutes = 1;
constexpr int kMaxMinutes = 180;
constexpr uint32_t kMinuteMs = 60000;
}  // namespace

void TimerModel::turn(int delta) {
  if (state_ != State::Setting) return;
  minutes_ += delta;
  if (minutes_ < kMinMinutes) minutes_ = kMinMinutes;
  if (minutes_ > kMaxMinutes) minutes_ = kMaxMinutes;
  total_ = remaining_ = uint32_t(minutes_) * kMinuteMs;
}

void TimerModel::turnToStep(int dir, int step) {
  if (state_ != State::Setting || step < 1) return;
  // Next multiple of `step` strictly above (dir > 0) or strictly below (dir < 0) the current value.
  const int target = dir > 0 ? (minutes_ / step + 1) * step : ((minutes_ + step - 1) / step - 1) * step;
  minutes_ = target < kMinMinutes ? kMinMinutes : (target > kMaxMinutes ? kMaxMinutes : target);
  total_ = remaining_ = uint32_t(minutes_) * kMinuteMs;
}

// Elapsed time uses unsigned subtraction so a millis() wrap mid-timer is harmless.
void TimerModel::settle(uint32_t nowMs) {
  const uint32_t elapsed = nowMs - startMs_;
  remaining_ = elapsed >= baseRemaining_ ? 0 : baseRemaining_ - elapsed;
}

void TimerModel::click(uint32_t nowMs) {
  switch (state_) {
    case State::Setting:
      total_ = remaining_ = baseRemaining_ = uint32_t(minutes_) * kMinuteMs;
      startMs_ = nowMs;
      state_ = State::Running;
      break;
    case State::Running:
      settle(nowMs);
      state_ = State::Paused;
      break;
    case State::Paused:
      baseRemaining_ = remaining_;
      startMs_ = nowMs;
      state_ = State::Running;
      break;
    case State::Done:
      cancel();
      break;
  }
}

void TimerModel::cancel() {
  state_ = State::Setting;
  total_ = remaining_ = uint32_t(minutes_) * kMinuteMs;
}

void TimerModel::tick(uint32_t nowMs) {
  if (state_ != State::Running) return;
  settle(nowMs);
  if (remaining_ == 0) state_ = State::Done;
}

TimerModel::State TimerModel::state() const { return state_; }
int TimerModel::setMinutes() const { return minutes_; }
uint32_t TimerModel::remainingMs() const { return remaining_; }
uint32_t TimerModel::totalMs() const { return total_; }
int TimerModel::minutesLeft() const { return int((remaining_ + kMinuteMs - 1) / kMinuteMs); }

bool TimerModel::inLastMinute() const {
  return (state_ == State::Running || state_ == State::Paused) && remaining_ > 0 && remaining_ <= kMinuteMs;
}
