#include "FeedParse.h"
#include <ArduinoJson.h>
#include <string.h>

namespace {
bool digit(char c) { return c >= '0' && c <= '9'; }
}  // namespace

bool parseClock(const char *s, uint16_t &minutes) {
  if (!s || strlen(s) != 5 || s[2] != ':') return false;
  if (!digit(s[0]) || !digit(s[1]) || !digit(s[3]) || !digit(s[4])) return false;
  const int h = (s[0] - '0') * 10 + (s[1] - '0');
  const int m = (s[3] - '0') * 10 + (s[4] - '0');
  if (h > 24 || m > 59 || (h == 24 && m != 0)) return false;
  minutes = uint16_t(h * 60 + m);
  return true;
}

bool parseFeedJson(const char *json, const char *today, DayEvent *out, size_t cap, size_t &n) {
  n = 0;
  if (!json) return false;
  JsonDocument doc;
  if (deserializeJson(doc, json)) return false;
  const char *date = doc["date"] | static_cast<const char *>(nullptr);
  JsonArrayConst events = doc["events"].as<JsonArrayConst>();
  if (!date || strcmp(date, today) != 0 || events.isNull()) return false;

  for (JsonVariantConst item : events) {
    if (n >= cap) break;
    JsonObjectConst o = item.as<JsonObjectConst>();
    if (o.isNull()) continue;
    uint16_t start, end;
    if (!parseClock(o["start"] | static_cast<const char *>(nullptr), start) ||
        !parseClock(o["end"] | static_cast<const char *>(nullptr), end) || end <= start)
      continue;
    DayEvent &e = out[n++];
    strncpy(e.title, o["title"] | "", sizeof e.title - 1);
    e.title[sizeof e.title - 1] = '\0';
    e.startMin = start;
    e.endMin = end;
  }
  return true;
}
