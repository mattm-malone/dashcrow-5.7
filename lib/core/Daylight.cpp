#include "Daylight.h"
#include <math.h>

namespace {
constexpr double kPi = 3.14159265358979323846;
constexpr double kDeg = kPi / 180.0;

uint16_t clampMinute(double m) {
  if (m < 0) return 0;
  if (m > 1439) return 1439;
  return uint16_t(m + 0.5);
}
}  // namespace

SunTimes sunTimes(int dayOfYear, double latDeg, double lonDeg, double tzHours) {
  const double g = 2.0 * kPi / 365.0 * (dayOfYear - 1);
  const double eqTime = 229.18 * (0.000075 + 0.001868 * cos(g) - 0.032077 * sin(g) - 0.014615 * cos(2 * g) -
                                  0.040849 * sin(2 * g));
  const double decl = 0.006918 - 0.399912 * cos(g) + 0.070257 * sin(g) - 0.006758 * cos(2 * g) +
                      0.000907 * sin(2 * g) - 0.002697 * cos(3 * g) + 0.00148 * sin(3 * g);
  const double lat = latDeg * kDeg;
  const double cosHa = cos(90.833 * kDeg) / (cos(lat) * cos(decl)) - tan(lat) * tan(decl);

  SunTimes s{false, false, 0, 1439};
  if (cosHa > 1.0) {
    s.polarNight = true;
    return s;
  }
  if (cosHa < -1.0) {
    s.polarDay = true;
    return s;
  }
  const double haDeg = acos(cosHa) / kDeg;
  s.sunriseMin = clampMinute(720.0 - 4.0 * (lonDeg + haDeg) - eqTime + tzHours * 60.0);
  s.sunsetMin = clampMinute(720.0 - 4.0 * (lonDeg - haDeg) - eqTime + tzHours * 60.0);
  return s;
}
