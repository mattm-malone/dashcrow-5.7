#include "RefreshPolicy.h"

RefreshKind RefreshPolicy::next(uint32_t contentKey, uint32_t layoutKey) {
  const bool first = !started_;
  started_ = true;
  const bool layoutChanged = layoutKey != layout_;
  const bool contentChanged = contentKey != content_;
  layout_ = layoutKey;
  content_ = contentKey;

  if (first) {
    partials_ = 0;
    return RefreshKind::Full;
  }
  // Page and overlay changes are partial too (fast); the periodic full refresh clears ghosting.
  if (!layoutChanged && !contentChanged) return RefreshKind::None;
  if (partials_ >= kPartialsBeforeFull) {
    partials_ = 0;
    return RefreshKind::Full;
  }
  partials_++;
  return RefreshKind::Partial;
}
