#include "SettleGate.h"

void SettleGate::input(uint32_t nowMs) {
  if (!pending_) {
    pending_ = true;
    firstMs_ = nowMs;
  }
  lastMs_ = nowMs;
}

bool SettleGate::ready(uint32_t nowMs) const {
  if (!pending_) return true;
  // Unsigned differences keep this correct across a millis() wrap.
  return uint32_t(nowMs - lastMs_) >= kQuietMs || uint32_t(nowMs - firstMs_) >= kMaxDelayMs;
}

void SettleGate::refreshed() { pending_ = false; }
