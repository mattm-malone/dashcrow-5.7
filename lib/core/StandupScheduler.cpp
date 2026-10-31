#include "StandupScheduler.h"

bool StandupScheduler::inWindow(const LocalTime &now) const {
  return now.valid && (cfg_.weekdayMask & (1u << (now.wday % 7))) && now.hour >= cfg_.startHour &&
         now.hour < cfg_.endHour;
}

bool StandupScheduler::due(const LocalTime &now) {
  if (!inWindow(now)) return false;
  // Epoch of today's window opening; an anchor older than that is from a previous window.
  const uint32_t secondsIntoWindow = uint32_t(now.hour - cfg_.startHour) * 3600u + now.minute * 60u + now.second;
  const uint32_t windowStart = now.epoch - secondsIntoWindow;
  if (!haveAnchor_ || anchor_ > now.epoch) {
    anchor_ = now.epoch;  // first valid sighting (or the clock moved backwards)
    haveAnchor_ = true;
  } else if (anchor_ < windowStart) {
    anchor_ = windowStart;
  }
  return now.epoch - anchor_ >= uint32_t(cfg_.intervalMin) * 60u;
}

void StandupScheduler::acknowledge(const LocalTime &now) {
  if (!now.valid) return;
  anchor_ = now.epoch;
  haveAnchor_ = true;
}
