#pragma once
#include <stdint.h>

enum class RefreshKind { None, Partial, Full };

constexpr int kPartialsBeforeFull = 30;

// layoutKey changes on page or overlay change (Full); contentKey changes when the picture
// should update in place (Partial). The first call is always Full.
class RefreshPolicy {
 public:
  RefreshKind next(uint32_t contentKey, uint32_t layoutKey);

 private:
  bool started_ = false;
  uint32_t content_ = 0, layout_ = 0;
  int partials_ = 0;
};
