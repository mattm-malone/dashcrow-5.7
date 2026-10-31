#pragma once
#include <Arduino.h>
#include "Calendar.h"

// Polls the personal-calendar JSON feed (tools/calfeed). On any failure it keeps the last good
// events; the feed is "stale" until a fetch has succeeded within the last hour.
class Feed {
 public:
  void begin(const char *url, uint32_t refreshMs);
  bool enabled() const { return url_ && url_[0]; }
  // todayIso: "YYYY-MM-DD" in local time, or nullptr while the clock is not valid (no fetch then).
  void poll(uint32_t nowMs, const char *todayIso);
  // Today's feed events, or none if the stored events belong to another day.
  size_t events(DayEvent *out, size_t cap, const char *todayIso) const;
  bool stale(uint32_t nowMs) const;
  uint8_t version() const { return version_; }  // bumps when new events arrive

 private:
  static constexpr size_t kMax = 16;
  const char *url_ = nullptr;
  uint32_t refreshMs_ = 0;
  uint32_t lastAttemptMs_ = 0;
  uint32_t lastSuccessMs_ = 0;
  bool attempted_ = false;
  bool succeeded_ = false;
  bool lastOk_ = false;
  uint8_t version_ = 0;
  char date_[11] = "";
  DayEvent events_[kMax];
  size_t count_ = 0;
};
