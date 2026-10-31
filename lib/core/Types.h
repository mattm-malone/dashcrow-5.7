#pragma once
#include <stdint.h>

struct LocalTime {
  bool valid;      // false until NTP has set the clock
  uint32_t epoch;  // seconds; only differences are used
  uint8_t wday;    // 0 = Sunday
  uint8_t hour, minute, second;
  uint8_t mday, month;  // month 1-12
};

enum class Page { Timeline, Timer, Clock };
enum class Button { Menu, Exit, Click };

struct InputEvent {
  enum class Type { Press, Turn } type;
  Button button;  // Press only
  int dir;        // Turn only: +1 up, -1 down
  uint32_t ms;    // when the input happened (millis); 0 if unknown
};
