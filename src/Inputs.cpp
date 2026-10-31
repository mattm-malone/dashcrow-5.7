#include "Inputs.h"

namespace {
constexpr uint32_t kDebounceMs = 20;
constexpr uint8_t kMenuPin = 2, kExitPin = 1, kClickPin = 5, kUpPin = 6, kDownPin = 4;
}  // namespace

void Inputs::begin() {
  using T = InputEvent::Type;
  keys_[0] = {kMenuPin, {T::Press, Button::Menu, 0}, false, false, 0};
  keys_[1] = {kExitPin, {T::Press, Button::Exit, 0}, false, false, 0};
  keys_[2] = {kClickPin, {T::Press, Button::Click, 0}, false, false, 0};
  keys_[3] = {kUpPin, {T::Turn, Button::Click, +1}, false, false, 0};
  keys_[4] = {kDownPin, {T::Turn, Button::Click, -1}, false, false, 0};
  for (Key &k : keys_) pinMode(k.pin, INPUT_PULLUP);
}

// Reports at most one new press per call; call repeatedly until it returns false.
bool Inputs::poll(InputEvent &out, uint32_t nowMs) {
  for (int n = 0; n < kKeys; n++) {
    Key &k = keys_[(next_ + n) % kKeys];
    const bool pressed = digitalRead(k.pin) == LOW;
    if (pressed != k.raw) {
      k.raw = pressed;
      k.changedMs = nowMs;
    } else if (pressed != k.stable && uint32_t(nowMs - k.changedMs) >= kDebounceMs) {
      k.stable = pressed;
      if (pressed) {
        out = k.event;
        next_ = (next_ + n + 1) % kKeys;
        return true;
      }
    }
  }
  return false;
}
