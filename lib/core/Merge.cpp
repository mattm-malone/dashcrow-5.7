#include "Merge.h"
#include <string.h>

namespace {
bool same(const DayEvent &x, const DayEvent &y) {
  return x.startMin == y.startMin && x.endMin == y.endMin && strcmp(x.title, y.title) == 0;
}
}  // namespace

size_t mergeEvents(const DayEvent *a, size_t n, const DayEvent *b, size_t m, DayEvent *out, size_t cap) {
  size_t i = 0, j = 0, count = 0;
  while ((i < n || j < m) && count < cap) {
    // Take from `a` on ties so config meetings keep their position ahead of feed events.
    const bool takeA = j >= m || (i < n && a[i].startMin <= b[j].startMin);
    const DayEvent &next = takeA ? a[i++] : b[j++];
    bool dup = false;
    for (size_t k = 0; k < count && !dup; k++) dup = same(out[k], next);
    if (!dup) out[count++] = next;
  }
  return count;
}
