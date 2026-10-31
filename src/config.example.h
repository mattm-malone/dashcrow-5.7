// Copy to src/config.h (gitignored) and edit. Never commit credentials.
#pragma once

// Wi-Fi. Leave WIFI_SSID empty to run without network (clock shows --:--).
#define WIFI_SSID ""
#define WIFI_PASS ""

// POSIX TZ string, e.g. "EST5EDT,M3.2.0,M11.1.0" (US Eastern), "PST8PDT,M3.2.0,M11.1.0",
// "GMT0BST,M3.5.0/1,M10.5.0", "UTC0".
#define TZ_STRING "UTC0"

// Used for the daylight strip on the Timeline page.
#define LATITUDE 40.7
#define LONGITUDE -74.0

// Timeline window, minutes since midnight (08:00 to 19:00).
#define DAY_START_MIN 480
#define DAY_END_MIN 1140

// Stand-up reminder: interval, active hours [start, end), weekday mask (bit 0 = Sunday).
#define STANDUP_INTERVAL_MIN 60
#define STANDUP_START_HOUR 9
#define STANDUP_END_HOUR 17
#define STANDUP_WEEKDAY_MASK 0b0111110

// Personal calendar feed from tools/calfeed (see its README). Empty disables it. Never point
// this at work data.
#define FEED_URL ""
#define FEED_REFRESH_MIN 15

// Recurring meetings, typed in by hand (no work-calendar data is read). Times are minutes since
// midnight; weekday mask bit 0 = Sunday (0b0111110 = Mon-Fri, 1 << 3 = Wednesday only).
// Optional last four fields make a meeting repeat every N weeks: N, then the year, month and day of any date
// in a week where it really happens. Weeks run Sunday-Saturday. Leave them out for a weekly meeting.
#include "Calendar.h"
static const Meeting MEETINGS[] = {
    {"Standup", 0b0111110, 9 * 60 + 30, 9 * 60 + 45},
    {"Team sync", 1 << 3, 14 * 60, 15 * 60},
    // Every other Tuesday, counting from the week of Tue 3 Nov 2026:
    // {"Sprint review", 1 << 2, 14 * 60, 15 * 60, 2, 2026, 11, 3},
};
