#include "Feed.h"
#include <HTTPClient.h>
#include <WiFi.h>
#include <string.h>
#include "FeedParse.h"

namespace {
constexpr int kMaxBodyBytes = 8192;  // a day of events is well under 2 KB
constexpr uint32_t kStaleAfterMs = 60UL * 60UL * 1000UL;
constexpr uint16_t kTimeoutMs = 2000;  // fetches block the UI, so keep them short
}  // namespace

void Feed::begin(const char *url, uint32_t refreshMs) {
  url_ = url;
  refreshMs_ = refreshMs;
}

void Feed::poll(uint32_t nowMs, const char *todayIso) {
  if (!enabled() || !todayIso || WiFi.status() != WL_CONNECTED) return;
  const bool dateCurrent = strcmp(date_, todayIso) == 0;  // false after midnight: refetch without waiting
  if (!feedFetchDue(attempted_, nowMs - lastAttemptMs_, lastOk_, dateCurrent, refreshMs_)) return;
  attempted_ = true;
  lastAttemptMs_ = nowMs;
  lastOk_ = false;

  HTTPClient http;
  http.setTimeout(kTimeoutMs);
  http.setConnectTimeout(kTimeoutMs);
  if (!http.begin(url_)) return;
  const int status = http.GET();
  const int size = http.getSize();  // -1 when unknown (chunked); refuse that and anything oversized
  if (status == HTTP_CODE_OK && size > 0 && size <= kMaxBodyBytes) {
    const String body = http.getString();
    DayEvent parsed[kMax];
    size_t n = 0;
    if (parseFeedJson(body.c_str(), todayIso, parsed, kMax, n)) {
      memcpy(events_, parsed, sizeof parsed);
      count_ = n;
      strncpy(date_, todayIso, sizeof date_ - 1);
      date_[sizeof date_ - 1] = '\0';
      lastSuccessMs_ = nowMs;
      succeeded_ = true;
      lastOk_ = true;
      version_++;
    }
  }
  http.end();
}

size_t Feed::events(DayEvent *out, size_t cap, const char *todayIso) const {
  if (!todayIso || strcmp(date_, todayIso) != 0) return 0;
  const size_t n = count_ < cap ? count_ : cap;
  memcpy(out, events_, n * sizeof(DayEvent));
  return n;
}

bool Feed::stale(uint32_t nowMs) const { return !succeeded_ || uint32_t(nowMs - lastSuccessMs_) > kStaleAfterMs; }
