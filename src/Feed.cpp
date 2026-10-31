#include "Feed.h"
#include <HTTPClient.h>
#include <WiFi.h>
#include <string.h>
#include "FeedParse.h"

namespace {
constexpr uint32_t kRetryMs = 60000;
constexpr uint32_t kStaleAfterMs = 60UL * 60UL * 1000UL;
constexpr uint16_t kTimeoutMs = 5000;
}  // namespace

void Feed::begin(const char *url, uint32_t refreshMs) {
  url_ = url;
  refreshMs_ = refreshMs;
}

void Feed::poll(uint32_t nowMs, const char *todayIso) {
  if (!enabled() || !todayIso || WiFi.status() != WL_CONNECTED) return;
  const uint32_t wait = lastOk_ ? refreshMs_ : kRetryMs;
  if (attempted_ && uint32_t(nowMs - lastAttemptMs_) < wait) return;
  attempted_ = true;
  lastAttemptMs_ = nowMs;
  lastOk_ = false;

  HTTPClient http;
  http.setTimeout(kTimeoutMs);
  if (!http.begin(url_)) return;
  const int status = http.GET();
  if (status == HTTP_CODE_OK) {
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
