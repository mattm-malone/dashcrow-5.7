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
// Click is a short press of the dial's centre switch; ClickLong is the same switch held for about a second.
enum class Button { Menu, Exit, Click, ClickLong };

struct InputEvent {
  enum class Type { Press, Turn } type;
  Button button;  // Press only
  int dir;        // Turn only: +1 up, -1 down
  uint32_t ms;    // when the input happened (millis); 0 if unknown
  bool repeat;    // Turn only: auto-repeat of a held key (not the initial press)
};

// How the timer is drawn. Bar is the full-width draining bar; the others are the hourglass (halftone dots),
// the same hourglass in dithered sand, the Time Timer disc and one dot per minute. EXIT cycles through them in this order.
enum class TimerStyle : uint8_t { Bar, Hourglass, Dither, Disc, Dots, kCount };
