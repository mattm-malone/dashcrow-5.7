#pragma once
#include <Arduino.h>
#include "Types.h"

// Buttons -> InputEvents. Presses are counted in a GPIO interrupt so none is lost while the
// main loop is blocked in a slow e-paper refresh. The "rotary" is two discrete switches (up and
// down) plus a click switch; each press of up/down is one Turn step.
class Inputs {
 public:
  void begin();
  bool poll(InputEvent &out, uint32_t nowMs);
};
