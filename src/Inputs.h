#pragma once
#include <Arduino.h>
#include "Types.h"

// Debounced buttons -> InputEvents. The "rotary" is two discrete switches (up/down) plus a
// click switch; each press of up/down is one Turn step.
class Inputs {
 public:
  void begin();
  bool poll(InputEvent &out, uint32_t nowMs);

 private:
  struct Key {
    uint8_t pin;
    InputEvent event;
    bool stable;        // debounced level, true = pressed (active low)
    bool raw;
    uint32_t changedMs;
  };
  static constexpr int kKeys = 5;
  Key keys_[kKeys];
  int next_ = 0;
};
