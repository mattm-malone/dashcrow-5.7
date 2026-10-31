# Desk Display Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build firmware for the CrowPanel 5.79" e-paper as a desk display: Clock, Timer and Timeline pages, MENU-button page switching, and a stand-up overlay that appears on any page.

**Architecture:** Everything with logic (timer, stand-up scheduler, app state machine, calendar math, refresh policy) and all page rendering is portable C++ in `lib/`, tested on the host with `pio test -e native`. Rendering goes into an Adafruit `GFXcanvas1`; a host tool writes it to a PBM image for previews, and the device copies it into the Elecrow panel driver. Only `src/` touches hardware (panel, GPIO, Wi-Fi/NTP).

**Tech Stack:** PlatformIO (espressif32, Arduino), Adafruit GFX (`GFXcanvas1`), Unity tests (`native` env), ArduinoJson (device), Python + pytest + `icalendar`/`recurring-ical-events` (calendar converter, run with `uv`).

**Spec:** `docs/superpowers/specs/2026-10-30-desk-display-design.md`

## Global Constraints

- Run PlatformIO as `export PATH=$HOME/.local/bin:$PATH` then `pio ...`. Native tests: `pio test -e native`. Device build: `pio run -e crowpanel-579`. Upload: `pio run -e crowpanel-579 -t upload`.
- Panel: 792×272 visible, framebuffer 800×272 (8 dead columns between the two SSD1683 halves). Panel pins live in `lib/EPD/spi.h` (SCK 12, MOSI 11, RES 47, DC 46, CS 45, BUSY 48). Screen power IO7 must be driven HIGH.
- Inputs: MENU = IO2, EXIT = IO1, rotary Down = IO4, rotary Up = IO6, rotary click = IO5. The "rotary" is discrete switches (one pin per direction), not a quadrature encoder; treat each pulse as one step.
- Pages cycle with MENU in the order Timeline → Timer → Clock → Timeline; the device boots on Clock.
- Rotary is input only; page switching only on MENU. Stand-up overlay dismisses on any button press (MENU, EXIT, rotary click) and never on a rotary turn. The previous page is restored unchanged.
- Timer: 1–180 minutes, default 25. Bar is segmented (one block per minute) under 60 min and continuous at 60+. Last minute (remaining ≤ 60 s) shows seconds and inverts. Done shows an inverted "TIME'S UP"; no sound.
- Stand-up defaults: 60 min interval, weekdays 09:00 (inclusive) to 17:00 (exclusive); the countdown restarts when the overlay is acknowledged.
- No work-calendar or work-email data on the device. Recurring work meetings are typed into `config.h`.
- `src/config.h` is gitignored; `src/config.example.h` is committed and shows every field.
- Commit trailer on every commit: `Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>`.

## Review Focus

1. NTP or Wi-Fi unavailable at boot or lost later: Clock shows `--:--`, no crash, and the stand-up never fires until time is valid (Task 4, Task 6).
2. `millis()` rollover while a timer runs (after ~49 days uptime): remaining time stays correct (Task 2).
3. Stand-up fires while a timer is running or in Done: timer keeps its state, and dismissal does not start, pause or cancel it (Task 4).
4. Meetings that overlap, have zero length, cross midnight, or fall outside the day window: timeline must not crash or draw outside the bar (Task 7).
5. Calendar feed unreachable or malformed: fall back to config meetings and flag the feed as stale (Task 9).

---

### Task 1: Native test env, preview tool, big digits, Clock page

**Files:**
- Modify: `platformio.ini` (add `[env:native]`, keep `[env:crowpanel-579]`, add lib_deps below)
- Create: `native_shim/Arduino.h`, `native_shim/Print.h` (minimal shims so Adafruit GFX compiles on the host)
- Create: `lib/render/Canvas.h` (`using Canvas = GFXcanvas1; constexpr int kW = 792, kH = 272;`)
- Create: `lib/render/BigDigits.h`, `lib/render/BigDigits.cpp`
- Create: `lib/render/ClockPage.h`, `lib/render/ClockPage.cpp`
- Create: `tools/preview/main.cpp` (native program that renders every page with sample data and writes `build/preview/<page>.pbm`), `tools/pbm2png.py`
- Test: `test/test_render/test_bigdigits.cpp`

**Interfaces:**
- Produces: `void drawBigNumber(Canvas&, int x, int y, int height, int value, int minDigits)` (draws black-on-white seven-segment-style digits with `fillRect`/triangles, resolution independent; returns nothing); `int bigNumberWidth(int height, int digits)`; `struct ClockView { bool valid; uint8_t hour, minute, wday, mday, month; }`; `void renderClock(Canvas&, const ClockView&)`. Black = pixel value 1 in `Canvas`.

- [ ] **Step 1: Add the native env.** `[env:native]`: `platform = native`, `test_framework = unity`, `build_flags = -std=gnu++17 -Inative_shim`, `lib_deps = adafruit/Adafruit GFX Library`, `lib_compat_mode = off`. Device env gets `adafruit/Adafruit GFX Library` and `bblanchon/ArduinoJson@^7` in `lib_deps`. Expected: `pio test -e native` reports "no tests" rather than a config error.
- [ ] **Step 2: Write the failing tests** in `test_bigdigits.cpp`: `test_digit_zero_has_hole` (on a 200×200 canvas, `drawBigNumber(c, 0, 0, 150, 0, 1)` leaves the glyph centre pixel white and a pixel on the left bar black), `test_one_uses_less_ink_than_eight` (black pixel count of `1` < of `8`), `test_width_scales_with_digits` (`bigNumberWidth(150, 2) > bigNumberWidth(150, 1)`), `test_min_digits_pads` (value 5 with minDigits 2 draws ink in the left digit cell). Counting helper: iterate `c.getPixel`.
- [ ] **Step 3: Run `pio test -e native`; expect compile failure or FAIL** (functions undefined).
- [ ] **Step 4: Make the shim compile Adafruit GFX** (`Print` base class with virtual `write(uint8_t)`, `pgm_read_*` macros, `PROGMEM`, `min/max/abs`, `byte`, `String` only if the compiler demands it). Expand the shim only as compile errors require.
- [ ] **Step 5: Implement `drawBigNumber` / `bigNumberWidth`** with seven segments drawn as filled bars; segment thickness = height/9.
- [ ] **Step 6: Implement `renderClock`**: huge `HH:MM` (height 170, colon as two squares), weekday and date in `FreeSansBold24pt7b` (bundled with Adafruit GFX under `Fonts/`) bottom-left; if `!valid` draw `--:--`. Match the Clock mockup in the spec.
- [ ] **Step 7: Implement `tools/preview/main.cpp`** with sample `ClockView{true, 14, 41, 4, 30, 10}`, writing PBM (P4), and `tools/pbm2png.py` (`uv run --with pillow tools/pbm2png.py in.pbm out.png`). Build with `pio run -e native` (add a `src_filter` so the preview main is used only by an extra `[env:preview]` that extends native).
- [ ] **Step 8: Run `pio test -e native`; expect PASS.** Run the preview, convert to PNG, and open `build/preview/clock.png` to check the layout by eye.
- [ ] **Step 9: Commit** (`git add -A && git commit -m "feat: native test env, big digit renderer, clock page"`).

---

### Task 2: Timer model and rotary acceleration

**Files:**
- Create: `lib/core/TimerModel.h`, `lib/core/TimerModel.cpp`, `lib/core/Accel.h`
- Test: `test/test_core/test_timer.cpp`, `test/test_core/test_accel.cpp`

**Interfaces:**
- Produces: 
  `class TimerModel { enum class State { Setting, Running, Paused, Done }; void turn(int delta); void click(uint32_t nowMs); void cancel(); void tick(uint32_t nowMs); State state() const; int setMinutes() const; uint32_t remainingMs() const; uint32_t totalMs() const; int minutesLeft() const; bool inLastMinute() const; }`
  `inline int accelDelta(int dir, uint32_t nowMs)` in `Accel.h` (stateful via a small `struct RotaryAccel { int step(int dir, uint32_t nowMs); }` instead of a free function; use the struct).

- [ ] **Step 1: Write failing tests (`test_timer.cpp`):**
  `test_defaults` (Setting, `setMinutes()==25`); `test_turn_clamps` (`turn(-100)` → 1, `turn(999)` → 180); `test_turn_ignored_when_running`; `test_click_starts` (state Running, `remainingMs()==25*60000` at t0); `test_remaining_counts_down` (`tick(t0+60000)` → `24*60000`); `test_minutes_left_is_ceiling` (remaining 59999 → 1, 60000 → 1, 60001 → 2); `test_done_at_zero` (`tick(t0+25*60000)` → Done, `remainingMs()==0`); `test_pause_freezes_remaining` (click pauses; `tick` 10 min later, remaining unchanged; click resumes and counts on); `test_cancel_returns_to_setting_keeping_minutes`; `test_click_in_done_returns_to_setting`; `test_last_minute` (true at 60000 and 1, false at 60001 and at 0); `test_millis_rollover` (start at `0xFFFFFC00`, `tick(0x000003E8)` → `remainingMs()==totalMs()-2024`).
- [ ] **Step 2: Write failing tests (`test_accel.cpp`):** first step returns ±1; second step same direction within 150 ms returns ±5; a gap over 150 ms returns ±1; a direction change returns ±1.
- [ ] **Step 3: Run `pio test -e native`; expect FAIL.**
- [ ] **Step 4: Implement** both. Elapsed time is `uint32_t(nowMs - startMs)` so rollover works; pause stores remaining.
- [ ] **Step 5: Run `pio test -e native`; expect PASS.**
- [ ] **Step 6: Commit** (`feat: timer model and rotary acceleration`).

---

### Task 3: Timer page rendering

**Files:**
- Create: `lib/render/TimerPage.h`, `lib/render/TimerPage.cpp`
- Modify: `tools/preview/main.cpp` (add timer samples)
- Test: `test/test_render/test_timerpage.cpp`

**Interfaces:**
- Consumes: `drawBigNumber`, `Canvas` (Task 1).
- Produces: `struct TimerView { enum class Mode { Setting, Running, Paused, Done } mode; int setMinutes; int minutesLeft; int secondsLeft; uint32_t remainingMs; uint32_t totalMs; uint8_t endHour, endMinute; }`; `void renderTimer(Canvas&, const TimerView&)`; `int timerBarFilledPx(uint32_t remainingMs, uint32_t totalMs, int barWidthPx)`; `constexpr int kBarX = 12, kBarY = 214, kBarW = 768, kBarH = 46;`.

- [ ] **Step 1: Write failing tests:** `test_filled_px_proportional` (`timerBarFilledPx(30000, 60000, 768)==384`; full → 768; 0 → 0); `test_running_bar_drains_left` (at 25% remaining, pixel `(kBarX+10, kBarY+10)` black and `(kBarX+kBarW-10, kBarY+10)` white); `test_segmented_under_60` (25-min total: the gap column between segments, at `kBarX + kBarW/25`, is white inside the bar when remaining ≥ that segment); `test_continuous_at_60` (60-min total: no white gap columns inside the filled region); `test_last_minute_inverts` (mode Running, remaining 30000: pixel `(2,2)` black); `test_done_inverted` (pixel `(2,2)` black); `test_setting_previews_bar` (setMinutes 90 vs 30: black pixel count for the bar differs, with 90 filling the whole bar).
- [ ] **Step 2: Run `pio test -e native`; expect FAIL.**
- [ ] **Step 3: Implement `timerBarFilledPx` and `renderTimer`** per the spec's Timer section: giant minute digits (height ~150) left, "MIN LEFT" caption, end time top-right, bar along the bottom; last minute draws seconds and fills the canvas black with white digits; Done draws white "TIME'S UP" on black; Paused draws a "PAUSED" label; Setting draws "SET TIMER" with a hint "turn = minutes · click = start".
- [ ] **Step 4: Run `pio test -e native`; expect PASS.** Render previews for each mode and view the PNGs; adjust sizes until the digits and bar read well.
- [ ] **Step 5: Commit** (`feat: timer page rendering`).

---

### Task 4: Stand-up scheduler, app state machine, refresh policy

**Files:**
- Create: `lib/core/Types.h`, `lib/core/StandupScheduler.h/.cpp`, `lib/core/App.h/.cpp`, `lib/core/RefreshPolicy.h/.cpp`
- Test: `test/test_core/test_standup.cpp`, `test/test_core/test_app.cpp`, `test/test_core/test_refresh.cpp`

**Interfaces:**
- Consumes: `TimerModel`, `RotaryAccel` (Task 2).
- Produces:
  `struct LocalTime { bool valid; uint32_t epoch; uint8_t wday; uint8_t hour, minute, second; };` (wday 0 = Sunday)
  `struct StandupConfig { uint16_t intervalMin = 60; uint8_t startHour = 9, endHour = 17; uint8_t weekdayMask = 0b0111110; };`
  `class StandupScheduler { explicit StandupScheduler(StandupConfig); bool due(const LocalTime&); void acknowledge(const LocalTime&); };`
  `enum class Page { Timeline, Timer, Clock };` `enum class Button { Menu, Exit, Click };`
  `struct InputEvent { enum class Type { Press, Turn } type; Button button; int dir; };`
  `class App { App(StandupConfig); void onInput(const InputEvent&, uint32_t nowMs, const LocalTime&); void tick(uint32_t nowMs, const LocalTime&); Page page() const; bool overlayActive() const; const TimerModel& timer() const; };`
  `enum class RefreshKind { None, Partial, Full };` `class RefreshPolicy { RefreshKind next(uint32_t contentKey, uint32_t layoutKey); };` (layoutKey changes on page or overlay change → Full; contentKey change → Partial; every 30th consecutive Partial → Full).

- [ ] **Step 1: Write failing stand-up tests** using a helper `mk(wday, hour, minute, second)`: `test_first_due_one_interval_after_first_valid_time` (first seen Mon 09:30:00; `due` false at 10:29:59, true at 10:30:00); `test_ack_restarts_countdown` (ack at 10:30 → false at 11:29:59, true at 11:30:00); `test_not_due_on_weekend`; `test_not_due_after_end_hour` (false at Mon 17:00:00 even with a stale ack); `test_window_open_resets_countdown` (ack Mon 16:30; Tue 09:00 → false, Tue 10:00 → true); `test_invalid_time_never_due` (`valid=false` → false and does not start the countdown).
- [ ] **Step 2: Write failing app tests:** `test_boots_on_clock`; `test_menu_cycles_pages_in_order` (Clock → Timeline → Timer → Clock); `test_overlay_appears_when_due`; `test_menu_dismisses_overlay_without_changing_page`; `test_exit_and_click_also_dismiss`; `test_turn_does_not_dismiss_or_change_timer`; `test_dismiss_does_not_start_timer_on_click` (page Timer, Setting; overlay up; click dismisses and `timer().state()==Setting`); `test_timer_keeps_running_under_overlay`; `test_done_state_survives_overlay`; `test_dismiss_acknowledges_scheduler` (no new overlay for the next interval); `test_turn_on_timer_page_changes_minutes`; `test_click_on_timer_page_starts`; `test_exit_cancels_timer`; `test_standup_never_fires_without_valid_time`.
- [ ] **Step 3: Write failing refresh tests:** unchanged keys → None; contentKey change → Partial; layoutKey change → Full; 30 consecutive Partials then the next change → Full and the counter resets.
- [ ] **Step 4: Run `pio test -e native`; expect FAIL.**
- [ ] **Step 5: Implement** the three units. App routes events by page (MENU always switches; Turn/Click/Exit go to the timer only on Page::Timer; Timeline scroll is added in Task 8). The overlay swallows the first Press only.
- [ ] **Step 6: Run `pio test -e native`; expect PASS.**
- [ ] **Step 7: Commit** (`feat: stand-up scheduler, app state machine, refresh policy`).

---

### Task 5: Device bring-up — panel, inputs, NTP, Clock page, refresh timing

**Files:**
- Create: `src/PanelDisplay.h/.cpp`, `src/Inputs.h/.cpp`, `src/Net.h/.cpp`, `src/config.example.h`
- Modify: `src/main.cpp` (replace hello-world), `platformio.ini` (no change to pins)
- Create: `docs/refresh-timing.md`

**Interfaces:**
- Consumes: `Canvas`, `renderClock` (Task 1), `App`, `InputEvent`, `LocalTime`, `RefreshPolicy` (Task 4).
- Produces:
  `class PanelDisplay { void begin(); void show(const Canvas&, RefreshKind); };` (copies canvas pixels into the 800×272 driver buffer with the 4-px offset used by `EPD_ShowPicture`, then full: `EPD_Display + EPD_Update`; partial: the partial sequence from Elecrow's `5.79_partial_refresh` example, then `EPD_PartUpdate`);
  `class Inputs { void begin(); bool poll(InputEvent& out, uint32_t nowMs); };` (20 ms debounce on each pin; MENU/EXIT/click → Press; IO6 pulse → Turn dir +1, IO4 pulse → Turn dir −1);
  `class Net { void begin(); LocalTime now(); bool timeValid(); };` (Wi-Fi from `config.h`, NTP via `configTzTime(TZ_STRING, "pool.ntp.org")`, `now()` maps `localtime_r` into `LocalTime`).
  `config.example.h` fields: `WIFI_SSID`, `WIFI_PASS`, `TZ_STRING` (e.g. `"EST5EDT,M3.2.0,M11.1.0"`), `LATITUDE`, `LONGITUDE`, `DAY_START_MIN` (default 480), `DAY_END_MIN` (default 1140), standup interval/hours/weekday mask, `MEETINGS[]` array of `Meeting` (defined in Task 7; the field is added there).

- [ ] **Step 1: Copy `config.example.h` to `src/config.h`** (gitignored) and fill in the user's Wi-Fi and timezone (ask the user for these; never commit them).
- [ ] **Step 2: Implement `PanelDisplay`, `Inputs`, `Net`.** The Elecrow reference for partial refresh is the repo `Elecrow-RD/CrowPanel-ESP32-5.79-E-paper-HMI-Display-with-272-792`, `example/arduino/Examples/5.79_partial_refresh/5.79_partial_refresh.ino`.
- [ ] **Step 3: Rewrite `main.cpp`:** `setup()` powers the screen (IO7 HIGH), starts display, inputs, Net; `loop()` polls inputs into `App`, ticks `App`, renders the current page into the canvas, and calls `show()` when `RefreshPolicy` says so. For now only Clock renders; Timeline and Timer show a placeholder text.
- [ ] **Step 4: Build and flash;** expect the clock to appear after Wi-Fi connects, and `--:--` before then. Verify by serial monitor that pressing MENU, EXIT, the rotary click, and turning the rotary each print one event per action; if turn direction is reversed, swap the IO4/IO6 assignment in `Inputs`.
- [ ] **Step 5: Measure refresh timing:** log `millis()` around a partial and a full refresh, plus ten consecutive partials on the clock page. Record the numbers and observed ghosting in `docs/refresh-timing.md`; set `kPartialsBeforeFull` in `RefreshPolicy` and the page update cadence from the measurement.
- [ ] **Step 6: Unplug Wi-Fi (or set a wrong SSID), reboot, and verify** the clock shows `--:--` and does not crash.
- [ ] **Step 7: Commit** (`feat: device bring-up with clock page and input handling`; verify `git status` shows `src/config.h` untracked-ignored).

---

### Task 6: Timer and stand-up on device

**Files:**
- Modify: `src/main.cpp`
- Create: `lib/render/StandupOverlay.h/.cpp`, `lib/render/TimerChip.h/.cpp`
- Test: `test/test_render/test_overlay.cpp`

**Interfaces:**
- Consumes: `renderTimer`, `TimerView` (Task 3), `App` (Task 4).
- Produces: `void renderStandup(Canvas&)` (full inverted, white "STAND UP", smaller "Press any button"); `void drawTimerChip(Canvas&, int minutesLeft)` (small bordered chip, top-right corner, "⏱ N min" drawn as text `N MIN`); `TimerView makeTimerView(const TimerModel&, const LocalTime&)` in `lib/core` or `lib/render` (pick `lib/render/TimerView.cpp`).

- [ ] **Step 1: Write failing tests:** `test_standup_overlay_is_mostly_black` (black pixel ratio > 0.8); `test_standup_has_white_text` (some white pixels inside the central band); `test_chip_draws_in_corner_only` (pixels outside the chip rect unchanged on a white canvas).
- [ ] **Step 2: Run `pio test -e native`; expect FAIL.** Implement; run again; expect PASS.
- [ ] **Step 3: Wire `main.cpp`:** render Timer page via `renderTimer(makeTimerView(...))`; draw the chip on Clock and Timeline pages while the timer is Running or Paused; render the overlay instead of the page whenever `overlayActive()`; compute `contentKey` from the timer bar column and minute so the bar updates at the measured cadence; `layoutKey` from page and overlay state.
- [ ] **Step 4: Flash and verify on hardware** with a checklist: set 2 minutes with the rotary and see the bar preview resize; click to start and watch the bar drain and digits change by minute; last minute inverts and shows seconds; Done shows "TIME'S UP", any press returns to Setting; pause/resume and EXIT-cancel work; temporarily set the stand-up interval to 1 minute in `config.h`, confirm the overlay appears on each page, a rotary turn does not dismiss it, any button does, and the previous page returns; a running timer survives the overlay. Restore the interval to 60.
- [ ] **Step 5: Commit** (`feat: timer and stand-up overlay on device`).

---

### Task 7: Calendar core — recurring meetings, day events, layout, daylight

**Files:**
- Create: `lib/core/Calendar.h/.cpp`, `lib/core/Daylight.h/.cpp`
- Test: `test/test_core/test_calendar.cpp`, `test/test_core/test_daylight.cpp`

**Interfaces:**
- Produces:
  `struct Meeting { const char* title; uint8_t weekdayMask; uint16_t startMin, endMin; };` (minutes since midnight; mask bit 0 = Sunday)
  `struct DayEvent { char title[32]; uint16_t startMin, endMin; };`
  `size_t eventsForDay(const Meeting* meetings, size_t n, uint8_t wday, DayEvent* out, size_t cap);` (sorted by start; drops `end <= start` after clipping `end` to 1440; truncates to `cap`)
  `struct NextInfo { int index; bool inProgress; int minutesUntil; };` `NextInfo nextEvent(const DayEvent*, size_t n, uint16_t nowMin);` (index −1 if none)
  `struct DayWindow { uint16_t startMin, endMin; };` `int minuteToX(uint16_t minute, DayWindow w, int widthPx);` (clamped to [0, widthPx])
  `struct SunTimes { bool polarDay, polarNight; uint16_t sunriseMin, sunsetMin; };` `SunTimes sunTimes(int dayOfYear, double latDeg, double lonDeg, double tzHours);`

- [ ] **Step 1: Write failing calendar tests:** `test_weekday_filter` (Monday-only meeting absent on Tuesday); `test_sorted_by_start`; `test_zero_length_dropped`; `test_crosses_midnight_is_clipped` (22:00–02:00 becomes 22:00–24:00); `test_cap_respected` (cap 2 with 3 meetings returns 2); `test_overlap_kept_both`; `test_next_event_before_first` (now 8:00, first at 9:00 → index 0, minutesUntil 60, not inProgress); `test_next_event_in_progress` (now inside a meeting → `inProgress`, minutesUntil 0); `test_next_event_none_after_last` (index −1); `test_minute_to_x` (window 480–1140, width 660: 600 → 120, 400 → 0, 1200 → 660).
- [ ] **Step 2: Write failing daylight tests:** `test_equator_equinox` (lat 0, lon 0, tz 0, day 80: sunrise within 355–365 min, sunset within 1075–1085); `test_polar_day` (lat 80, day 172 → `polarDay`); `test_polar_night` (lat 80, day 355 → `polarNight`).
- [ ] **Step 3: Run `pio test -e native`; expect FAIL.** Implement (NOAA sunrise equation, minutes since local midnight); run again; expect PASS.
- [ ] **Step 4: Commit** (`feat: calendar and daylight core`).

---

### Task 8: Timeline page

**Files:**
- Create: `lib/render/TimelinePage.h/.cpp`
- Modify: `lib/core/App.h/.cpp` (timeline scroll), `src/main.cpp`, `src/config.example.h`, `tools/preview/main.cpp`
- Test: `test/test_render/test_timeline.cpp`, `test/test_core/test_app.cpp` (add cases)

**Interfaces:**
- Consumes: `DayEvent`, `NextInfo`, `DayWindow`, `minuteToX`, `SunTimes` (Task 7).
- Produces: `struct TimelineView { bool timeValid; uint8_t hour, minute, wday, mday, month; const DayEvent* events; size_t n; DayWindow window; SunTimes sun; int dayOffset; bool feedStale; }`; `void renderTimeline(Canvas&, const TimelineView&)`; `App::dayOffset() const` (rotary turns on the Timeline page move ±1 day, clamped to ±7; rotary click resets to 0).

- [ ] **Step 1: Write failing tests:** render: `test_meeting_block_spans_expected_x` (a 10:00–11:00 meeting with window 480–1140 on a 660-px bar fills the bar row from x0+120 to x0+180); `test_past_meeting_is_hatched` (alternating pixels in a block that ended before now, solid for a future one); `test_now_marker_present` (a black pixel column above the bar at the now position); `test_nothing_drawn_outside_bar` (meetings outside the window leave bar pixels at the edges white); `test_overlap_and_zero_length_do_not_crash`; `test_next_line_text_present` (some ink in the NEXT line band); `test_stale_marker` (`feedStale` adds ink in the corner marker area). App: `test_timeline_turn_changes_day_offset`, `test_timeline_click_resets_offset`, `test_offset_clamped_to_week`.
- [ ] **Step 2: Run `pio test -e native`; expect FAIL.** Implement and run again; expect PASS.
- [ ] **Step 3: Add `MEETINGS[]` to `config.example.h` and `src/config.h`** with two sample weekday meetings; wire Timeline rendering in `main.cpp` (events recomputed on day change; refresh cadence once per minute).
- [ ] **Step 4: Preview PNG for a busy day;** adjust layout; flash and verify on the device that the now-marker moves, and scrolling days works.
- [ ] **Step 5: Commit** (`feat: timeline page`).

---

### Task 9: Personal calendar feed

**Files:**
- Create: `tools/calfeed/calfeed.py`, `tools/calfeed/test_calfeed.py`, `tools/calfeed/README.md`
- Create: `lib/core/Merge.h/.cpp`, `src/Feed.h/.cpp`
- Test: `test/test_core/test_merge.cpp`, `tools/calfeed/test_calfeed.py`
- Modify: `src/main.cpp`, `src/config.example.h` (add `FEED_URL`, `FEED_REFRESH_MIN`)

**Interfaces:**
- Produces: converter CLI `uv run tools/calfeed/calfeed.py --ics <url|file> --date YYYY-MM-DD --tz <IANA> [--serve PORT]` printing `{"date": "...", "events": [{"title": "...", "start": "HH:MM", "end": "HH:MM"}]}` (timed events only, titles truncated to 31 chars, sorted); `size_t mergeEvents(const DayEvent* a, size_t n, const DayEvent* b, size_t m, DayEvent* out, size_t cap)` (sorted by start, exact duplicates removed); `class Feed { void begin(); void poll(uint32_t nowMs); bool fetched() const; bool stale() const; size_t events(DayEvent* out, size_t cap); }` (stale = last successful fetch older than 60 min or never).

- [ ] **Step 1: Write failing Python tests** with a small inline `.ics` fixture: `test_weekly_recurring_expands_on_matching_date`; `test_other_weekday_excluded`; `test_all_day_event_excluded`; `test_utc_times_converted_to_tz`; `test_long_title_truncated_to_31`; `test_output_sorted`. Run `uv run --with pytest --with icalendar --with recurring-ical-events pytest tools/calfeed -v`; expect FAIL.
- [ ] **Step 2: Implement `calfeed.py`** (`icalendar` + `recurring_ical_events`; `--serve` runs a tiny `http.server` returning today's JSON so the board can poll a LAN URL); run pytest; expect PASS.
- [ ] **Step 3: Write failing merge tests:** `test_merge_sorted`; `test_duplicates_removed`; `test_cap_respected`; `test_empty_inputs`. Run; expect FAIL; implement; expect PASS.
- [ ] **Step 4: Implement `Feed`** with `HTTPClient` + ArduinoJson (reject documents whose `date` is not today, ignore malformed entries, keep the last good events on any failure). Wire into `main.cpp`: timeline events = `mergeEvents(config meetings, feed events)`; `TimelineView.feedStale` from `Feed::stale()` when `FEED_URL` is set.
- [ ] **Step 5: Hardware check:** run `calfeed.py --serve` against a personal calendar's iCal URL, point `FEED_URL` at it, confirm events appear; stop the server and confirm config meetings remain with the stale marker after an hour (shorten the threshold temporarily to verify, then restore); feed malformed JSON and confirm no crash.
- [ ] **Step 6: Write `tools/calfeed/README.md`** (how to get a personal iCal URL, run the converter, and the hosting options: laptop, Raspberry Pi, hosted function; never point it at work data) and commit (`feat: personal calendar feed`).

---

## Self-Review

- **Spec coverage:** controls (Tasks 4, 5), Timer set/run/done/last-minute/bar/chip (Tasks 2, 3, 6), Timeline with config meetings, daylight, scroll (Tasks 7, 8), Clock (Task 1), stand-up overlay and scheduler (Tasks 4, 6), NTP and invalid-time handling (Task 5), refresh policy and measurement (Tasks 4, 5), feed and stale marker (Task 9), `config.h`/`config.example.h` split (Task 5), desktop preview (Task 1). Spec's "check Elecrow license" stays an open item and is not a build task.
- **Types:** `LocalTime`, `InputEvent`, `TimerModel::State`, `TimerView`, `DayEvent`, `Canvas`, and `RefreshKind` are defined once and consumed by later tasks with the same names.
- **Proportion:** code appears only as signatures and test assertions.
