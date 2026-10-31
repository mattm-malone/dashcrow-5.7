#include "PanelDisplay.h"
#include "EPD.h"
#include "PanelMap.h"

namespace {
constexpr int kScreenPowerPin = 7;
uint8_t gImage[27200];  // 800x272 driver framebuffer (see EPD_Init.h for the 8-column seam)
}  // namespace

void PanelDisplay::begin() {
  pinMode(kScreenPowerPin, OUTPUT);
  digitalWrite(kScreenPowerPin, HIGH);
  EPD_GPIOInit();
  memset(gImage, 0xFF, sizeof gImage);
}

// Canvas ink (1) becomes the driver's black (0); see lib/core/PanelMap.h for the layout.
void PanelDisplay::copyFrom(const Canvas &canvas) { packForPanel(canvas.getBuffer(), gImage); }

void PanelDisplay::show(const Canvas &canvas, RefreshKind kind) {
  if (kind == RefreshKind::None) return;
  const uint32_t t0 = millis();
  copyFrom(canvas);
  const uint32_t t1 = millis();
  uint32_t t2, t3, t4;
  if (kind == RefreshKind::Full) {
    if (clearPass_) {
      // Clear pass first so the previous image cannot ghost into this one.
      EPD_GPIOInit();
      EPD_FastMode1Init();
      EPD_Display_Clear();
      EPD_Update();
    }
    EPD_GPIOInit();
    EPD_FastMode1Init();
    EPD_Display(gImage);
    t2 = millis();
    EPD_Update();
  } else {
    EPD_HW_RESET();
    EPD_Display(gImage);
    t2 = millis();
    EPD_PartUpdate();
  }
  t3 = millis();
  // Make the reference RAM match what is now on the glass, or the next partial update
  // diffs against stale data (the first page change after a full refresh drew garbage).
  EPD_DisplayOld(gImage);
  EPD_DeepSleep();
  t4 = millis();
  copyMs_ = t1 - t0;
  sendMs_ = t2 - t1;
  updateMs_ = t3 - t2;
  oldMs_ = t4 - t3;
  lastMs_ = t4 - t0;
}
