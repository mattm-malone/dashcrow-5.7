# calfeed: personal calendar -> desk display

Turns a **personal** iCal calendar into today's timed events as tiny JSON that the display polls.
Recurring events are expanded here so the ESP32 never parses iCal.

> Do not point this at work data. The display deliberately has no work-calendar or work-email access.

## Get an iCal URL
- Google Calendar: Settings -> your calendar -> "Secret address in iCal format". Anyone with the link can read the
  calendar, so keep it private.
- iCloud: share the calendar publicly and use the `webcal://` link, changing `webcal` to `https`.
- Or pass a local `.ics` file path.

## Run
```bash
# print today's events once
uv run --with icalendar --with recurring-ical-events tools/calfeed/calfeed.py \
    --ics "https://calendar.google.com/calendar/ical/.../basic.ics" --tz America/New_York

# serve them to the display (re-fetches the calendar at most every 5 minutes)
uv run --with icalendar --with recurring-ical-events tools/calfeed/calfeed.py \
    --ics "<url>" --tz America/New_York --serve 8765 --host 192.168.1.50
```
`--host` defaults to `127.0.0.1` (only this machine). The display needs to reach the server, so bind to the
machine's LAN address; the endpoint has no authentication, so use it only on a network you trust.

Then set `FEED_URL "http://192.168.1.50:8765/"` in `src/config.h`.

## Hosting
Anything that can run this command all day: your laptop (stops when it sleeps), a Raspberry Pi or home server
(simplest), or a small hosted function that returns the same JSON.

## Output
`{"date": "2026-10-30", "events": [{"title": "Dentist", "start": "10:00", "end": "11:00"}]}`
All-day events are omitted; events crossing midnight are clipped to the day (`"end": "24:00"`).

## Test
`uv run --with pytest --with icalendar --with recurring-ical-events pytest tools/calfeed`
