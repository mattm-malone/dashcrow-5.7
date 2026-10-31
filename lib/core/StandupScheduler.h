#pragma once
#include "Types.h"

struct StandupConfig {
  uint16_t intervalMin = 60;
  uint8_t startHour = 9;          // inclusive
  uint8_t endHour = 17;           // exclusive
  uint8_t weekdayMask = 0b0111110;  // bit 0 = Sunday; default Mon-Fri
};

// Due once `intervalMin` has passed since the last acknowledgement (or first valid time
// of the day's window), only inside the active window. Stays due until acknowledged.
class StandupScheduler {
 public:
  explicit StandupScheduler(StandupConfig cfg = {}) : cfg_(cfg) {}
  bool due(const LocalTime &now);
  void acknowledge(const LocalTime &now);

 private:
  bool inWindow(const LocalTime &now) const;

  StandupConfig cfg_;
  bool haveAnchor_ = false;
  uint32_t anchor_ = 0;
};
