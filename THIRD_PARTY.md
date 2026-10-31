# Third-party code

This project's own code is MIT licensed (see `LICENSE`). It includes or uses:

| Component | Where | Licence |
|---|---|---|
| Adafruit GFX Library 1.11.9, trimmed (BusIO and SPITFT removed) | `lib/gfx` | BSD 2-clause; see `lib/gfx/license.txt` |
| FreeSansBold 9/12/18/24 pt fonts (from Adafruit GFX; derived from GNU FreeFont) | `lib/gfx/Fonts` | GPL with font exception, via Adafruit. Check the exception covers your use before redistributing a firmware binary. |
| Elecrow CrowPanel 5.79" e-paper driver, modified (hardware SPI, bulk transfer, old-image RAM write) | `lib/EPD` | **No licence stated** by Elecrow (<https://github.com/Elecrow-RD/CrowPanel-ESP32-5.79-E-paper-HMI-Display-with-272-792>). Included at the author's discretion; if Elecrow adds a licence it applies to those files. |
| ArduinoJson | fetched by PlatformIO, not vendored | MIT |
