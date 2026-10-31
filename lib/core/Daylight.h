#pragma once
#include <stdint.h>

struct SunTimes {
  bool polarDay, polarNight;
  uint16_t sunriseMin, sunsetMin;  // local minutes since midnight (meaningless when polar)
};

// NOAA solar equations. dayOfYear is 1-based; tzHours is the local UTC offset (with DST).
SunTimes sunTimes(int dayOfYear, double latDeg, double lonDeg, double tzHours);
