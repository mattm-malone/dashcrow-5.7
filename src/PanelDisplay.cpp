#include "PanelDisplay.h"
#include "EPD.h"

namespace {
constexpr int kScreenPowerPin = 7;
uint8_t gImage[27200];  // 800x272 driver framebuffer (see EPD_Init.h for the 8-column seam)
}  // namespace

void PanelDisplay::begin() {
  pinMode(kScreenPowerPin, OUTPUT);
  digitalWrite(kScreenPowerPin, HIGH);
  EPD_GPIOInit();
  Paint_NewImage(gImage, EPD_W, EPD_H, Rotation, WHITE);
  Paint_Clear(WHITE);
}

// Paint_SetPixel applies the driver's rotation and the seam offset at x >= 396.
// Canvas ink (1) is the driver's BLACK (0x00).
void PanelDisplay::copyFrom(const Canvas &canvas) {
  for (int y = 0; y < kH; y++)
    for (int x = 0; x < kW; x++) Paint_SetPixel(x, y, canvas.getPixel(x, y) ? BLACK : WHITE);
}

void PanelDisplay::show(const Canvas &canvas, RefreshKind kind) {
  if (kind == RefreshKind::None) return;
  const uint32_t t0 = millis();
  copyFrom(canvas);
  if (kind == RefreshKind::Full) {
    // Clear pass first so the previous image cannot ghost into this one.
    EPD_GPIOInit();
    EPD_FastMode1Init();
    EPD_Display_Clear();
    EPD_Update();
    EPD_Clear_R26A6H();
    EPD_GPIOInit();
    EPD_FastMode1Init();
    EPD_Display(gImage);
    EPD_Update();
    EPD_DeepSleep();
  } else {
    EPD_HW_RESET();
    EPD_Display(gImage);
    EPD_PartUpdate();
    EPD_DeepSleep();
  }
  lastMs_ = millis() - t0;
}
