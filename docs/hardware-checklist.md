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

## Timeline and calendar feed
- [ ] MENU to Timeline: date and time header, NEXT line, day bar with your recurring meetings, hour labels, sunrise/sunset strip.
- [ ] The "now" marker sits at the current time; past meetings are hatched.
- [ ] Rotary up/down moves one day per press (header date changes, no "now" marker); click returns to today.
- [ ] Set `LATITUDE`/`LONGITUDE` and compare sunrise/sunset to a weather app (within a few minutes).
- [ ] With a timer running, the "N MIN" chip on Timeline does not overlap the header text.
- [ ] Run `tools/calfeed` (see its README), set `FEED_URL`, and confirm personal events appear within the refresh interval.
- [ ] Stop the feed server: config meetings remain and "FEED STALE" appears after an hour (or shorten `kStaleAfterMs` in `src/Feed.cpp` to check, then restore).

## Timer styles and dial hold
- [ ] EXIT on the Timer page cycles Bar -> Hourglass -> Disc -> Dots -> Bar; the choice survives a reboot; EXIT on other pages does nothing.
- [ ] EXIT no longer cancels a running timer.
- [ ] Short click on the dial starts, pauses and resumes (it now acts when you release the dial, not when you press).
- [ ] Holding the dial click for about a second cancels a running, paused or finished timer (minutes are kept).
- [ ] While setting, holding the dial click resets the minutes to 25.
- [ ] A hold does not also trigger a short click when you let go.
- [ ] Hourglass: a sideways hourglass of dots: big at the outer ends, shrinking steadily toward the neck; the left bulb empties from the neck side, the right fills from its outer edge, a line of grains marches through the neck once a second, and the `MM:SS` counts down over it; no visible ghosting after a few minutes.
- [ ] Disc and dots look right at 100%, 50% and in the last minute (inverted).
