#include "Net.h"
#include <WiFi.h>
#include <time.h>
#include "config.h"

namespace {
constexpr time_t kSyncedAfter = 1700000000;  // any time before Nov 2023 means "not synced"
}

void Net::begin() {
  configTzTime(TZ_STRING, "pool.ntp.org", "time.nist.gov");
  if (strlen(WIFI_SSID) == 0) return;
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
}

LocalTime Net::now() const {
  const time_t t = time(nullptr);
  if (t < kSyncedAfter) return {false, 0, 0, 0, 0, 0};
  tm lt;
  localtime_r(&t, &lt);
  return {true, uint32_t(t), uint8_t(lt.tm_wday), uint8_t(lt.tm_hour), uint8_t(lt.tm_min), uint8_t(lt.tm_sec)};
}
