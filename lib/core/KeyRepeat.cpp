#include "KeyRepeat.h"

void KeyRepeat::press(uint32_t nowMs) {
  active_ = true;
  nextMs_ = nowMs + kDelayMs;
}

void KeyRepeat::release() { active_ = false; }

bool KeyRepeat::next(uint32_t nowMs, uint32_t &at) {
  // Signed difference keeps the comparison correct across a millis() wrap.
  if (!active_ || int32_t(nowMs - nextMs_) < 0) return false;
  at = nextMs_;
  nextMs_ += kIntervalMs;
  return true;
}
