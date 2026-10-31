#pragma once
#include <stdint.h>

enum class RefreshKind { None, Partial, Full };

constexpr int kPartialsBeforeFull = 30;

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
