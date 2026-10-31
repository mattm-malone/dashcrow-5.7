#!/usr/bin/env python3
"""Turn a personal iCal calendar into the day's timed events as small JSON for the desk display.

  uv run --with icalendar --with recurring-ical-events tools/calfeed/calfeed.py \\
      --ics <url-or-file> --tz America/New_York [--date YYYY-MM-DD] [--serve PORT [--host ADDR]]

Output: {"date": "2026-10-30", "events": [{"title": "...", "start": "14:00", "end": "15:00"}]}
All-day events are left out; events crossing midnight are clipped to the day ("24:00" end).
"""
import argparse
import datetime as dt
import json
import sys
import time
import urllib.request
from http.server import BaseHTTPRequestHandler, HTTPServer
from zoneinfo import ZoneInfo

import icalendar
import recurring_ical_events

TITLE_MAX = 31


def _hhmm(minutes: int) -> str:
    return f"{minutes // 60:02d}:{minutes % 60:02d}"


def _local(value, tz: ZoneInfo):
    """Datetime in tz, or None for all-day (date) values. Floating times are taken as tz-local."""
    if not isinstance(value, dt.datetime):
        return None
    return value.replace(tzinfo=tz) if value.tzinfo is None else value.astimezone(tz)


def events_for_date(ics_text: str, date: dt.date, tz_name: str) -> dict:
    tz = ZoneInfo(tz_name)
    try:
        calendar = icalendar.Calendar.from_ical(ics_text)
    except Exception as exc:  # icalendar raises assorted errors on junk input
        raise ValueError(f"not a valid iCalendar document: {exc}") from exc

    day_start = dt.datetime.combine(date, dt.time.min, tzinfo=tz)
    day_end = day_start + dt.timedelta(days=1)
    events = []
    for comp in recurring_ical_events.of(calendar).between(day_start, day_end):
        start = _local(comp.decoded("DTSTART"), tz)
        if start is None:
            continue  # all-day
        if "DTEND" in comp:
            end = _local(comp.decoded("DTEND"), tz)
        elif "DURATION" in comp:
            end = start + comp.decoded("DURATION")
        else:
            end = None
        if end is None or end <= start:
            continue
        start, end = max(start, day_start), min(end, day_end)
        s = int((start - day_start).total_seconds() // 60)
        e = int((end - day_start).total_seconds() // 60)
        if e <= s:
            continue
        events.append({"title": str(comp.get("SUMMARY", "")).strip()[:TITLE_MAX], "start": _hhmm(s), "end": _hhmm(e)})
    events.sort(key=lambda ev: (ev["start"], ev["end"], ev["title"]))
    return {"date": date.isoformat(), "events": events}


def fetch(source: str) -> str:
    if source.startswith(("http://", "https://")):
        with urllib.request.urlopen(source, timeout=20) as resp:
            return resp.read().decode("utf-8", errors="replace")
    with open(source, encoding="utf-8") as fh:
        return fh.read()


class _Cache:
    """Re-fetches the calendar at most every `ttl` seconds."""

    def __init__(self, source: str, ttl: float = 300.0):
        self.source, self.ttl, self.text, self.at = source, ttl, "", 0.0

    def get(self) -> str:
        if not self.text or time.time() - self.at > self.ttl:
            self.text, self.at = fetch(self.source), time.time()
        return self.text


def serve(source: str, tz_name: str, host: str, port: int) -> None:
    cache = _Cache(source)

    class Handler(BaseHTTPRequestHandler):
        def do_GET(self):
            try:
                today = dt.datetime.now(ZoneInfo(tz_name)).date()
                body = json.dumps(events_for_date(cache.get(), today, tz_name)).encode()
                self.send_response(200)
                self.send_header("Content-Type", "application/json")
            except Exception as exc:  # keep serving; the device keeps its last good data
                body = json.dumps({"error": str(exc)}).encode()
                self.send_response(502)
                self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)

        def log_message(self, fmt, *args):  # never log URLs/calendar data
            pass

    print(f"serving today's events on http://{host}:{port}/", file=sys.stderr)
    HTTPServer((host, port), Handler).serve_forever()


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--ics", required=True, help="iCal URL or file path (a personal calendar)")
    ap.add_argument("--tz", required=True, help="IANA time zone, e.g. America/New_York")
    ap.add_argument("--date", help="YYYY-MM-DD (default: today in --tz)")
    ap.add_argument("--serve", type=int, metavar="PORT", help="serve today's JSON over HTTP instead of printing")
    ap.add_argument("--host", default="127.0.0.1", help="bind address for --serve (default: localhost only)")
    args = ap.parse_args()
    if args.serve:
        serve(args.ics, args.tz, args.host, args.serve)
        return
    date = dt.date.fromisoformat(args.date) if args.date else dt.datetime.now(ZoneInfo(args.tz)).date()
    print(json.dumps(events_for_date(fetch(args.ics), date, args.tz)))


if __name__ == "__main__":
    main()
