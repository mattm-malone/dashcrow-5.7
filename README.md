# dashcrow

Desk display firmware for the Elecrow CrowPanel ESP32-S3 5.79" e-paper (792x272, black and white).

Three pages, cycled with the MENU button:

- **Timeline**: your day as a bar with recurring meetings (from `src/config.h`; weekly, or every N weeks with an anchor date), a "now" marker, a NEXT line and a sunrise/sunset strip. Up/down scrolls days; click returns to today.
- **Timer**: up/down sets minutes (hold to jump in fives), click starts/pauses, EXIT cancels. A bar drains across the panel; the last minute shows seconds, inverted.
- **Clock**: large time and date.

A **stand-up reminder** takes over the screen on any page at a configurable interval until any button is pressed.
An optional personal-calendar feed (`tools/calfeed`) adds events to the Timeline. It deliberately has no work-calendar access.

## Build

```bash
uv tool install --python 3.12 platformio
cp src/config.example.h src/config.h      # then edit Wi-Fi, timezone, lat/long, meetings (gitignored)
pio run -e crowpanel-579 -t upload        # flash
pio test -e native                        # host tests (no hardware needed)
```

`docs/superpowers/specs` and `docs/superpowers/plans` hold the design and implementation plan; `docs/hardware-checklist.md`
lists what to verify on a real board; `docs/refresh-timing.md` has the measured panel refresh timings.

## Licence

MIT, see `LICENSE`. Third-party notices are in `THIRD_PARTY.md`.
