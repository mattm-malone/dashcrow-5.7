#pragma once
#include "Types.h"

// Wi-Fi + NTP. Time is invalid until the first sync.
class Net {
 public:
  void begin();
  LocalTime now() const;
};
