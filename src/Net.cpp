#include "Net.h"
#include <WiFi.h>
#include <time.h>
#include "LocalTimeUtil.h"
#include "config.h"

void Net::begin() {
  configTzTime(TZ_STRING, "pool.ntp.org", "time.nist.gov");
  if (strlen(WIFI_SSID) == 0) return;
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
}

LocalTime Net::now() const { return localTimeFrom(time(nullptr)); }
