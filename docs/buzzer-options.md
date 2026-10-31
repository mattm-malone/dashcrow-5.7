# Adding sound: buzzer options

Status: **undecided, nothing bought or built.** Goal: an audible alert when the timer finishes (the display has no
other cue for that) and optionally when the stand-up overlay appears.

## What the board offers

Pin header on the top edge (from Elecrow's pinout image,
<https://www.elecrow.com/wiki/assets/images/CrowPanel_ESP32_E-paper_5.79-inch_HMI_Display/ESP32-EPAPER-5.79inch-pinout.webp>):

| | | | | | | | | |
|---|---|---|---|---|---|---|---|---|
| top row | IO3 | IO9 | IO15 | IO17 | IO19 | IO21 | GND | GND | GND |
| bottom row | IO8 | IO14 | IO16 | IO18 | IO20 | IO38 | 3V3 | 3V3 | 3V3 |

- Power on the header is **3.3 V only (labelled VCC/3V3); there is no 5 V pin.**
- The firmware uses none of these pins (it uses IO1, 2, 4, 5, 6, 7, 11, 12, 45, 46, 47, 48; the SD slot uses IO10, 13, 39, 40).
- Avoid **IO3** (boot-strapping pin) and **IO19/IO20** (native USB data lines).
- Not verified: whether the header is male pins or female sockets. Check the board before ordering jumper wires:
  metal pins sticking up means you need female-ended Dupont wires; sockets with holes means male-ended ones.

### GPIO current

The ESP32-S3 datasheets give about 40 mA source / 28 mA sink per pin **at the strongest drive setting**. The default
drive strength is about 20 mA for most pins (IO17/IO18: 10 mA). Treat that as a ceiling and design for 10 mA or less per
pin. Sources: [ESP32-S3 datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf),
[ESP32-S3-WROOM-1 datasheet](https://documentation.espressif.com/esp32-s3-wroom-1_wroom-1u_datasheet_en.pdf).
(The full DC characteristics table was not read directly; confirm there if you design close to the limit.)

## Option A: bare passive piezo buzzer

A two-pin piezo disc driven with PWM tones straight from a GPIO.

- **Buy:** a *passive* piezo buzzer rated for 3 V use ("passive", not "active"; an active buzzer only plays one fixed
  tone), female or male Dupont jumper wires to match the header, and a 100-330 ohm resistor.
- **Avoid:** magnetic buzzers, which can draw 30 mA or more and need a transistor.
- **Wiring (power off first):** IO21 -> resistor -> one buzzer lead; the other lead -> the adjacent GND. Nothing on 3V3.
  IO21 sits right next to the GND pins at the end of the top row.
- **Software:** ESP32 LEDC PWM tone on IO21 (`ledcWriteTone`), driven by a small non-blocking melody sequencer.
- **Pros:** cheapest, fewest parts, two wires.
- **Cons:** the ESP32 spends a GPIO and does the timing; sound is a basic piezo beep.

## Option B: Arduino Modulino Buzzer (SKU ABX00108)

A small board with a piezo buzzer and its own STM32C011F4 microcontroller; you send it tone commands over I2C and it
plays them itself. Facts from Arduino's datasheet
(<https://docs.arduino.cc/resources/datasheets/ABX00108-datasheet.pdf>); the Micro Center page
(<https://www.microcenter.com/product/707364/arduino-modulino-buzzer>) blocked automated fetches, so the price is unknown.

- **Power:** 3.3 V (2.0-3.6 V), about 6.4 mA while sounding.
- **Interface:** I2C over two Qwiic connectors (1.0 mm 4-pin), plus unpopulated 2.54 mm pads: a 1x4 row
  (GND, 3V3, SDA, SCL) and a 1x8 row (GND, 3V3, RESET, SWCLK, SWDIO, TX1, RX1, PA0).
- **Address:** datasheet lists `0x3C` (Modulino convention) with a "hardware" address of `0x1E`; confirm with an I2C scan.
- **Pull-ups:** none fitted (pads for 4.7 kohm 0402 resistors exist). The CrowPanel header has none either.
- **Box contents:** the module and one Qwiic-to-Qwiic cable, which does **not** fit the CrowPanel header.
- **Buy:** a Qwiic-to-female-Dupont cable (4 wires), or solder a 4-pin header to the 1x4 pads; and two 4.7 kohm
  resistors (SDA and SCL to 3V3).
- **Wiring (power off first):** GND (top row), 3V3 (bottom row), SDA -> IO21, SCL -> IO17 (top row). Pins are
  configurable in software; avoid IO3 and IO19/IO20.
- **Software:** the Arduino_Modulino library is aimed at Arduino cores, so I would write the few I2C writes directly for
  the ESP32 (command format not yet verified; check the library source or sniff it on hardware), behind the same
  sequencer interface as option A.
- **Pros:** tidy plug-in module; the module times the tone itself; low current.
- **Cons:** costs more, 4 wires plus pull-ups, an extra I2C bus to debug, and needs a cable adapter.

## Decision checklist

1. Check whether the header is male pins or female sockets (decides the jumper wire type).
2. Pick A or B.
3. Order parts (list under the chosen option).
4. Tell Claude which you have; the firmware work is the same either way:
   - a `Chime` module with a host-tested, non-blocking sequencer (note list + timing, no hardware calls);
   - a thin device driver for the chosen buzzer;
   - chime on timer Done, optional chime on stand-up, and a quiet-hours setting in `config.h`.
