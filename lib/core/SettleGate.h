#pragma once
#include <stdint.h>

// Decides when to redraw after input: wait for a quiet moment so a burst of clicks costs one refresh,
// but never hold a redraw back longer than kMaxDelayMs, so a held key keeps updating the screen.
class SettleGate {
 public:
  static constexpr uint32_t kQuietMs = 250;
  static constexpr uint32_t kMaxDelayMs = 300;

  void input(uint32_t nowMs);
  bool ready(uint32_t nowMs) const;  // true when no input is pending, or it has settled or waited long enough
  void refreshed();                  // call once the pending input has been drawn (or needed no draw)

 private:
  bool pending_ = false;
  uint32_t firstMs_ = 0, lastMs_ = 0;
};
