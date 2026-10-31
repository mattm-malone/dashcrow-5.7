#pragma once
#include <stdint.h>

// Auto-repeat for a held key: nothing for kDelayMs after the press, then one repeat every
// kIntervalMs for as long as the key stays held. Call next() in a loop to drain due repeats.
class KeyRepeat {
 public:
  static constexpr uint32_t kDelayMs = 400;
  static constexpr uint32_t kIntervalMs = 120;

  void press(uint32_t nowMs);
  void release();
  bool active() const { return active_; }
  // True and `at` = the time the repeat was due, if one is owed at nowMs.
  bool next(uint32_t nowMs, uint32_t &at);

 private:
  bool active_ = false;
  uint32_t nextMs_ = 0;
};
