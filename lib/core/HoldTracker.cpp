#include "HoldTracker.h"

void HoldTracker::press(uint32_t nowMs) {
  active_ = true;
  longFired_ = false;
  releasing_ = false;
  startMs_ = nowMs;
}

HoldTracker::Event HoldTracker::update(uint32_t nowMs, bool held) {
  if (!active_) return Event::None;
  // Unsigned differences keep these comparisons correct across a millis() wrap.
  if (held) {
    releasing_ = false;
    if (!longFired_ && uint32_t(nowMs - startMs_) >= kHoldMs) {
      longFired_ = true;
      return Event::Long;
    }
    return Event::None;
  }
  if (!releasing_) {
    releasing_ = true;
    releaseStartMs_ = nowMs;
    return Event::None;
  }
  if (uint32_t(nowMs - releaseStartMs_) < kReleaseDebounceMs) return Event::None;
  active_ = false;
  return longFired_ ? Event::None : Event::Short;
}
