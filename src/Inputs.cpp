#include "Inputs.h"

namespace {

constexpr int kKeys = 5;
constexpr uint32_t kLockoutMs = 25;  // ignore edges this soon after the previous edge (contact bounce)

struct KeyDef {
  uint8_t pin;
  InputEvent event;
};

// Order: Menu IO2, Exit IO1, rotary click IO5, rotary up IO6, rotary down IO4 (active low).
const KeyDef kDefs[kKeys] = {
    {2, {InputEvent::Type::Press, Button::Menu, 0}},
    {1, {InputEvent::Type::Press, Button::Exit, 0}},
    {5, {InputEvent::Type::Press, Button::Click, 0}},
    {6, {InputEvent::Type::Turn, Button::Click, +1}},
    {4, {InputEvent::Type::Turn, Button::Click, -1}},
};

volatile uint8_t gPending[kKeys];
volatile uint32_t gLastEdgeMs[kKeys];

void IRAM_ATTR onEdge(void *arg) {
  const int i = int(intptr_t(arg));
  const uint32_t now = millis();
  const bool pressed = digitalRead(kDefs[i].pin) == LOW;
  // Count a press only on a falling edge that follows a quiet period; release bounce and
  // press bounce both refresh the quiet-period timer without counting.
  if (pressed && uint32_t(now - gLastEdgeMs[i]) >= kLockoutMs && gPending[i] < 255) gPending[i] = gPending[i] + 1;
  gLastEdgeMs[i] = now;
}

}  // namespace

void Inputs::begin() {
  for (int i = 0; i < kKeys; i++) {
    pinMode(kDefs[i].pin, INPUT_PULLUP);
    attachInterruptArg(kDefs[i].pin, onEdge, reinterpret_cast<void *>(intptr_t(i)), CHANGE);
  }
}

bool Inputs::poll(InputEvent &out, uint32_t) {
  for (int i = 0; i < kKeys; i++) {
    bool have = false;
    noInterrupts();
    if (gPending[i] > 0) {
      gPending[i] = gPending[i] - 1;
      have = true;
    }
    interrupts();
    if (have) {
      out = kDefs[i].event;
      return true;
    }
  }
  return false;
}
