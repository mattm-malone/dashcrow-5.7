# Desk e-paper display — design

Date: 2026-10-30
Hardware: Elecrow CrowPanel ESP32-S3 5.79" e-paper (792×272, 1-bit, partial refresh, MENU/EXIT buttons, rotary switch)
Toolchain: PlatformIO (installed with `uv tool install --python 3.12 platformio`), Arduino framework

## Goal

A USB-powered desk display with three pages, a stand-up reminder that overlays any page, and a heavily visual timer meant to be read at a glance. It complements the existing TRMNL (fridge) and 6" weather display (front door); it deliberately has no todo list, no dev/CI dashboard, no Claude Code status, and no work-email or work-calendar data.

## Non-goals

- Any connection to the work (Microsoft 365) calendar. Work data stays off the device. Recurring work meetings are typed into config by hand.
- Todo lists, dev dashboards, week/year progress, tally counters.
- Battery operation or deep sleep (the panel is on USB at the desk).
- Sound (no buzzer is known to be on the board).

## Controls

| Input | Timeline | Timer | Clock |
|---|---|---|---|
| MENU (IO2) | next page | next page | next page |
| Rotary turn (IO4/IO6) | scroll the day | set minutes | none |
| Rotary click (IO5) | jump to now | start / pause | none |
| EXIT (IO1) | none | cancel / reset | none |

The rotary is reserved for input; page switching is only on MENU. While the stand-up overlay is showing, any button press (MENU, EXIT, rotary click) dismisses it and returns to the previous page. Turning the rotary does not dismiss it.

## Pages

### Timer (the glance page)
- **Setting:** rotary turn sets minutes, and a preview bar resizes live. Rotary click starts.
- **Running:** giant minute digits ("MIN LEFT") over a full-width bar that drains toward the left, so remaining time reads as a block of black.
  - Under 60 min the bar is segmented, one block per minute.
  - Over 60 min it is one continuous bar.
  - In the last minute the digits switch to seconds and the screen inverts (white on black).
  - The header shows the end time of day.
- **Done:** inverted full screen "TIME'S UP"; any button returns to setting.
- Rotary click pauses/resumes; EXIT cancels.
- While a timer runs, other pages show a small timer chip with minutes left.

### Timeline
- Header: date and current time. Large "NEXT" line: title, minutes until, start–end.
- Day bar: meetings as solid blocks, past ones hatched, a "now" marker. Hour ticks under it.
- Thin daylight strip below (dark after sunset).
- Source: recurring meetings from `config.h`, plus an optional personal-calendar feed (see below).

### Clock
- Huge HH:MM, with date and weekday below. Time from NTP.

### Stand-up overlay
- Inverted full screen, "STAND UP", "Press any button". Appears over any page.
- Interval is configurable (default 60 min) with active hours (default weekdays 09:00–17:00).
- The countdown restarts when the overlay is acknowledged.
- A running timer keeps running underneath. On dismiss, the previous page is redrawn.
- Not aware of meetings (possible later).

## Architecture

```
ESP32-S3 firmware (PlatformIO, Arduino)
  Display   Elecrow driver: push 1-bit framebuffer, full + partial refresh
  Render    Adafruit GFX into the framebuffer, generated large bold fonts
  Input     MENU / EXIT / rotary, debounced -> events
  App       page state machine + stand-up overlay + timer chip
  Clock     Wi-Fi + NTP + timezone
  Calendar  recurring meetings (config) + optional JSON feed
```

- The Elecrow driver (`lib/EPD`, vendored from Elecrow's example) only tops out at 48 px text, so large digits use Adafruit GFX with fonts generated for this project; the driver is used to push pixels. Panel pins: SCK 12, MOSI 11, RES 47, DC 46, CS 45, BUSY 48, screen power on IO7 (HIGH). The panel is driven as two SSD1683 halves with an 8-column gap, so the framebuffer is 800×272 with a 792×272 visible area.
- **Calendar feed (later milestone):** a small converter fetches a personal calendar's iCal URL and emits today's events as compact JSON for the device to poll. Recurrence expansion stays off the ESP32.
- **Config:** `config.h` holds Wi-Fi credentials (gitignored; a committed `config.example.h` shows the shape), timezone, day window, recurring meetings, and the stand-up interval and hours.
- **Desktop preview:** a PlatformIO `native` environment that renders pages to PNG, so layouts can be iterated without flashing.
- **Refresh policy:** partial refresh for timer ticks and the clock; a full refresh periodically and on every page change to clear ghosting. Update rates are set after measuring the panel's actual partial-refresh time.

## Milestones

1. Foundation: render layer, big fonts, input, page switching, Clock page via NTP, refresh-speed measurement.
2. Timer: set, run, done, segmented bar.
3. Stand-up reminder and the timer chip on other pages.
4. Timeline: recurring meetings from config, daylight strip.
5. Personal calendar feed: converter service and device fetch.

## Risks and open items

- Partial refresh speed on this panel is unmeasured; the timer's update cadence (bar every few seconds, digits per minute) depends on it.
- Ghosting from partial refresh; mitigated by periodic full refresh.
- No buzzer assumed; "done" is visual only.
- Where the calendar converter runs (laptop, Pi, or a hosted function) is undecided and affects only milestone 5.
- `lib/EPD` is Elecrow's example code; check its license before publishing the repo publicly.
