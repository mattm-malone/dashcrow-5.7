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
  // Full refresh normally does a clear pass first (slow, cleanest); off = one full-waveform pass.
  void setClearPass(bool on) { clearPass_ = on; }

 private:
  void copyFrom(const Canvas &canvas);
  uint32_t lastMs_ = 0;
  bool clearPass_ = true;
};
