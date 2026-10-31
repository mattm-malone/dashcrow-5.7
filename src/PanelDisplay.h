#pragma once
#include <Arduino.h>
#include "Canvas.h"
#include "RefreshPolicy.h"

// Pushes a rendered 1-bit canvas to the CrowPanel via the Elecrow driver (lib/EPD).
class PanelDisplay {
 public:
  void begin();
  void show(const Canvas &canvas, RefreshKind kind);
  uint32_t lastRefreshMs() const { return lastMs_; }

 private:
  void copyFrom(const Canvas &canvas);
  uint32_t lastMs_ = 0;
};
