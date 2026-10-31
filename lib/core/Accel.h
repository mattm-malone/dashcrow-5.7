#pragma once
#include <stdint.h>

// Speeds up repeated rotary steps: same direction within 150 ms steps by 5.
struct RotaryAccel {
  int step(int dir, uint32_t nowMs) {
    const bool fast = have_ && dir == lastDir_ && uint32_t(nowMs - lastMs_) <= 150;
    have_ = true;
    lastDir_ = dir;
    lastMs_ = nowMs;
    return fast ? dir * 5 : dir;
  }

 private:
  bool have_ = false;
  int lastDir_ = 0;
  uint32_t lastMs_ = 0;
};
