#include <Arduino.h>
#include "EPD.h"

// Panel pins (SCK/MOSI/RES/DC/CS/BUSY) are defined in lib/EPD/spi.h.
static const int PIN_SCREEN_PWR = 7;

static uint8_t ImageBW[27200];  // 800x272 1-bit framebuffer

void setup() {
  Serial.begin(115200);
  pinMode(PIN_SCREEN_PWR, OUTPUT);
  digitalWrite(PIN_SCREEN_PWR, HIGH);

  EPD_GPIOInit();
  Paint_NewImage(ImageBW, EPD_W, EPD_H, Rotation, WHITE);
  Paint_Clear(WHITE);

  EPD_ShowString(20, 20, "Hello, CrowPanel!", 24, BLACK);
  EPD_DrawRectangle(10, 10, 781, 261, BLACK, 0);
  EPD_DrawLine(10, 60, 781, 60, BLACK);

  // Clear pass first so the previous image doesn't ghost into this one.
  EPD_FastMode1Init();
  EPD_Display_Clear();
  EPD_Update();

  // Full-quality waveform (0xF7) for the real image. EPD_FastUpdate (0xC7)
  // is quicker but noticeably grainy.
  EPD_GPIOInit();
  EPD_FastMode1Init();
  EPD_Display(ImageBW);
  EPD_Update();
  EPD_DeepSleep();
  Serial.println("drawn");
}

void loop() {
  delay(1000);
}
