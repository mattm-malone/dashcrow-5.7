// Prints every level change on the five input pins: pio run -e pintest -t upload
#include <Arduino.h>

static const uint8_t kPins[] = {2, 1, 5, 6, 4};  // MENU, EXIT, click, up, down
static int last[5];

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 5; i++) {
    pinMode(kPins[i], INPUT_PULLUP);
    last[i] = digitalRead(kPins[i]);
  }
  Serial.println("pintest ready");
  for (int i = 0; i < 5; i++) Serial.printf("IO%d start=%d\n", kPins[i], last[i]);
}

void loop() {
  for (int i = 0; i < 5; i++) {
    const int v = digitalRead(kPins[i]);
    if (v != last[i]) {
      Serial.printf("%lu IO%d -> %d\n", (unsigned long)millis(), kPins[i], v);
      last[i] = v;
    }
  }
}
