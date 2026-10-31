import datetime as dt

import calfeed

TZ = "America/New_York"


def ics(*events: str) -> str:
    return "BEGIN:VCALENDAR\nVERSION:2.0\nPRODID:-//test//EN\n" + "\n".join(events) + "\nEND:VCALENDAR\n"


def vevent(summary, start, end=None, extra=""):
    lines = ["BEGIN:VEVENT", f"UID:{summary}-{start}", "DTSTAMP:20260101T000000Z", f"SUMMARY:{summary}", start]
    if end:
        lines.append(end)
    if extra:
        lines.append(extra)
    lines.append("END:VEVENT")
    return "\n".join(lines)


def titles(feed):
    return [e["title"] for e in feed["events"]]


def test_weekly_recurring_expands_on_matching_date():
    cal = ics(vevent("Weekly", f"DTSTART;TZID={TZ}:20261026T140000", f"DTEND;TZID={TZ}:20261026T150000",
                     "RRULE:FREQ=WEEKLY;BYDAY=MO"))
    feed = calfeed.events_for_date(cal, dt.date(2026, 11, 2), TZ)  # a Monday
    assert feed["date"] == "2026-11-02"
    assert feed["events"] == [{"title": "Weekly", "start": "14:00", "end": "15:00"}]


def test_other_weekday_excluded():
    cal = ics(vevent("Weekly", f"DTSTART;TZID={TZ}:20261026T140000", f"DTEND;TZID={TZ}:20261026T150000",
                     "RRULE:FREQ=WEEKLY;BYDAY=MO"))
    assert calfeed.events_for_date(cal, dt.date(2026, 11, 3), TZ)["events"] == []


def test_all_day_event_excluded():
    cal = ics(vevent("Holiday", "DTSTART;VALUE=DATE:20261030", "DTEND;VALUE=DATE:20261031"))
    assert calfeed.events_for_date(cal, dt.date(2026, 10, 30), TZ)["events"] == []


def test_utc_times_converted_to_tz():
    cal = ics(vevent("Call", "DTSTART:20261030T180000Z", "DTEND:20261030T190000Z"))  # EDT is UTC-4
    assert calfeed.events_for_date(cal, dt.date(2026, 10, 30), TZ)["events"] == [
        {"title": "Call", "start": "14:00", "end": "15:00"}]


def test_long_title_truncated_to_31():
    cal = ics(vevent("x" * 60, f"DTSTART;TZID={TZ}:20261030T100000", f"DTEND;TZID={TZ}:20261030T110000"))
    assert len(calfeed.events_for_date(cal, dt.date(2026, 10, 30), TZ)["events"][0]["title"]) == 31


def test_output_sorted():
    cal = ics(
        vevent("Late", f"DTSTART;TZID={TZ}:20261030T150000", f"DTEND;TZID={TZ}:20261030T160000"),
        vevent("Early", f"DTSTART;TZID={TZ}:20261030T090000", f"DTEND;TZID={TZ}:20261030T100000"),
    )
    assert titles(calfeed.events_for_date(cal, dt.date(2026, 10, 30), TZ)) == ["Early", "Late"]


def test_event_spanning_midnight_is_clipped_to_the_day():
    cal = ics(vevent("Overnight", f"DTSTART;TZID={TZ}:20261030T220000", f"DTEND;TZID={TZ}:20261031T020000"))
    day1 = calfeed.events_for_date(cal, dt.date(2026, 10, 30), TZ)["events"]
    day2 = calfeed.events_for_date(cal, dt.date(2026, 10, 31), TZ)["events"]
    assert day1 == [{"title": "Overnight", "start": "22:00", "end": "24:00"}]
    assert day2 == [{"title": "Overnight", "start": "00:00", "end": "02:00"}]


def test_zero_length_and_missing_end_events_skipped():
    cal = ics(vevent("Zero", f"DTSTART;TZID={TZ}:20261030T100000", f"DTEND;TZID={TZ}:20261030T100000"),
              vevent("NoEnd", f"DTSTART;TZID={TZ}:20261030T110000"))
    assert calfeed.events_for_date(cal, dt.date(2026, 10, 30), TZ)["events"] == []


def test_malformed_ics_raises_value_error():
    import pytest
    with pytest.raises(ValueError):
        calfeed.events_for_date("this is not a calendar", dt.date(2026, 10, 30), TZ)
