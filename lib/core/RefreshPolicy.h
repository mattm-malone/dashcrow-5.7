#pragma once
#include <stdint.h>

enum class RefreshKind { None, Partial, Full };

// A full refresh flashes the whole panel, so it is rare: the panel showed no ghosting after hundreds of
// partial updates (the clock page does 1440 a day; the hourglass timer redraws every second, 1500 in 25 min).
constexpr int kPartialsBeforeFull = 2000;

// Any change to layoutKey (page/overlay) or contentKey is a Partial update; every
// kPartialsBeforeFull-th update, and the first call, is Full to clear ghosting.
class RefreshPolicy {
 public:
  RefreshKind next(uint32_t contentKey, uint32_t layoutKey);

 private:
  bool started_ = false;
  uint32_t content_ = 0, layout_ = 0;
  int partials_ = 0;
};
