#pragma once
#include "Calendar.h"

// "HH:MM" (00:00..24:00) -> minutes since midnight. False for anything else.
bool parseClock(const char *s, uint16_t &minutes);

// Parses {"date": "YYYY-MM-DD", "events": [{"title","start","end"}, ...]} from tools/calfeed.
// Returns false (n = 0) when the JSON is invalid or its date is not `today`. Individual malformed
// events are skipped; at most `cap` are kept.
bool parseFeedJson(const char *json, const char *today, DayEvent *out, size_t cap, size_t &n);
