#pragma once
#include <stdint.h>

// Tells a short press from a long hold on one key. Call press() when a press edge is seen, then update() every
// loop with the key's current level. A short press is reported once the key has been released for a moment;
// a hold is reported once, kHoldMs after the press, while the key is still down (no Short follows on release).
class HoldTracker {
 public:
  static constexpr uint32_t kHoldMs = 1000;
  static constexpr uint32_t kReleaseDebounceMs = 25;  // contact chatter shorter than this does not end the press

  enum class Event { None, Short, Long };

  void press(uint32_t nowMs);
  Event update(uint32_t nowMs, bool held);
  bool active() const { return active_; }
  uint32_t pressedAt() const { return startMs_; }

 private:
  bool active_ = false;
  bool longFired_ = false;
  bool releasing_ = false;
  uint32_t startMs_ = 0, releaseStartMs_ = 0;
};
