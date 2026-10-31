// Host shim so portable code and Adafruit GFX compile under platform=native.
#pragma once
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "Print.h"

#define PROGMEM
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
typedef uint8_t byte;
#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif
