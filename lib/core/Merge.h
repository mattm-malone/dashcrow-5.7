#pragma once
#include "Calendar.h"

// Union of two event lists sorted by start (ties: `a` first); exact duplicates (same title and
// times) are dropped; at most `cap` events. Either list may be empty (pointer may be null).
size_t mergeEvents(const DayEvent *a, size_t n, const DayEvent *b, size_t m, DayEvent *out, size_t cap);
