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
  // Phase timings of the last refresh: pixel copy, SPI transfer of the new image, waveform, SPI transfer of the old image.
  uint32_t copyMs() const { return copyMs_; }
  uint32_t sendMs() const { return sendMs_; }
  uint32_t updateMs() const { return updateMs_; }
  uint32_t oldMs() const { return oldMs_; }
  // Full refresh normally does a clear pass first (slow, cleanest); off = one full-waveform pass.
  void setClearPass(bool on) { clearPass_ = on; }

 private:
  void copyFrom(const Canvas &canvas);
  uint32_t lastMs_ = 0, copyMs_ = 0, sendMs_ = 0, updateMs_ = 0, oldMs_ = 0;
  bool clearPass_ = true;
};
