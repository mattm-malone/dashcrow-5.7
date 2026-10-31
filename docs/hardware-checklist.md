# Hardware checklist (needs a person at the board)

Fill in `src/config.h` first (copy `src/config.example.h`): Wi-Fi credentials, `TZ_STRING`, lat/long.
Flash: `pio run -e crowpanel-579 -t upload`. Serial log: 115200 baud on `/dev/ttyUSB0`.

## Inputs and clock
- [ ] Each of MENU, EXIT, rotary click, rotary up, rotary down prints one `input:` line per press.
- [ ] Rotary "up" increases timer minutes. If reversed, swap the IO6/IO4 entries in `src/Inputs.cpp`.
- [ ] With Wi-Fi configured, the clock shows the right local time within a minute of boot.
- [ ] With Wi-Fi off or wrong SSID, the clock shows `--:--` and nothing crashes.

## Timer
- [ ] MENU cycles Timeline (placeholder) -> Timer -> Clock.
- [ ] Rotary up/down changes minutes (1..180); holding quick presses speeds up by 5.
- [ ] Preview bar resizes while setting; a burst of presses causes one redraw, not one per press.
- [ ] Click starts; bar drains; digits change each minute; end time shows only when the clock is valid.
- [ ] Click pauses/resumes; EXIT cancels.
- [ ] Last minute: screen inverts and shows seconds. Done: inverted TIME'S UP; any click returns to setting.
- [ ] While a timer runs, the Clock page shows the "N MIN" chip top-right.

## Stand-up
- [ ] Temporarily set `STANDUP_INTERVAL_MIN` to 1 and the hours to include now; the overlay appears on any page.
- [ ] Rotary up/down does not dismiss it; MENU, EXIT and click each do, and the previous page returns.
- [ ] A running timer keeps counting while the overlay is up and is unaffected by dismissing it.
- [ ] Restore the interval to 60.

## Ghosting
- [ ] After 30+ partial updates the clock page is not visibly ghosted (if it is, lower `kPartialsBeforeFull`).
