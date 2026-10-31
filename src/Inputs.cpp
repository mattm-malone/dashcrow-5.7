#include "Inputs.h"

namespace {

constexpr int kKeys = 5;
constexpr uint32_t kLockoutMs = 25;  // ignore edges this soon after the previous edge (contact bounce)
constexpr uint8_t kQueue = 32;

struct KeyDef {
  uint8_t pin;
  InputEvent event;
};

// Order: Menu IO2, Exit IO1, rotary click IO5, rotary up IO6, rotary down IO4 (active low).
const KeyDef kDefs[kKeys] = {
    {2, {InputEvent::Type::Press, Button::Menu, 0, 0}},
    {1, {InputEvent::Type::Press, Button::Exit, 0, 0}},
    {5, {InputEvent::Type::Press, Button::Click, 0, 0}},
    {6, {InputEvent::Type::Turn, Button::Click, +1, 0}},
    {4, {InputEvent::Type::Turn, Button::Click, -1, 0}},
};

// Ring buffer filled by the ISR (producer) and drained by poll() (consumer). Each entry keeps the
// time of the press so acceleration reflects how fast the user turned, not when the loop ran.
volatile uint8_t gKey[kQueue];
volatile uint32_t gAt[kQueue];
volatile uint8_t gHead = 0, gTail = 0;
volatile uint32_t gLastEdgeMs[kKeys];

void IRAM_ATTR onEdge(void *arg) {
  const int i = int(intptr_t(arg)) - 1;  // arg is index + 1: the core treats a null arg as "no argument"
  const uint32_t now = millis();
  const bool pressed = digitalRead(kDefs[i].pin) == LOW;
  // Count a press only on a falling edge that follows a quiet period; release bounce and
  // press bounce both refresh the quiet-period timer without counting.
  if (pressed && uint32_t(now - gLastEdgeMs[i]) >= kLockoutMs) {
    const uint8_t next = (gHead + 1) % kQueue;
    if (next != gTail) {  // drop if full
      gKey[gHead] = i;
      gAt[gHead] = now;
      gHead = next;
    }
  }
  gLastEdgeMs[i] = now;
}

}  // namespace

void Inputs::begin() {
  for (int i = 0; i < kKeys; i++) {
    pinMode(kDefs[i].pin, INPUT_PULLUP);
    attachInterruptArg(kDefs[i].pin, onEdge, reinterpret_cast<void *>(intptr_t(i + 1)), CHANGE);
  }
}

bool Inputs::poll(InputEvent &out, uint32_t) {
  noInterrupts();
  const bool have = gTail != gHead;
  uint8_t key = 0;
  uint32_t at = 0;
  if (have) {
    key = gKey[gTail];
    at = gAt[gTail];
    gTail = (gTail + 1) % kQueue;
  }
  interrupts();
  if (!have) return false;
  out = kDefs[key].event;
  out.ms = at;
  return true;
}
