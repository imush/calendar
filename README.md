# Hebrew Calendar C Library

[![License](https://img.shields.io/badge/license-BSD--3--Clause-blue)](LICENSE)

Jewish calendar computations in plain C — dates, festivals, zmanim, readings and
haftarot. C11, no dependencies beyond the standard library.

**[Project page and API documentation](https://imush.github.io/calendar/)**

## It powers the Hebrew Calendar app

The dates, zmanim and readings in the Hebrew Calendar Zmanim app —
[App Store](https://apps.apple.com/us/app/hebrew-calendar-zmanim/id6780720472),
[Google Play](https://play.google.com/store/apps/details?id=net.hebrewcalendar.hebrew_calendar_app)
— are computed by this library, on the phone and without a network. It is small,
allocation-free in its calculations, and easy to call from Swift, Kotlin or Dart.

## What it does

- **Dates** — Jewish, Gregorian and Julian calendars, converted in any direction; leap
  years, month lengths, the day of the week and the keviut of a year.
- **Festivals and fasts** — Yom Tov, chol hamoed, rosh chodesh, fasts and their
  deferrals, the named Shabbatot, eruv tavshilin and the arba parshiyot, for Israel and
  for the diaspora. Chametz deadlines and fast times come with the day they belong to.
- **Zmanim** — one call fills a struct with the day's times, from the NOAA solar
  calculations: sunrise and sunset, dawn and nightfall in several opinions, candle
  lighting, and the halachic hours between them.
- **Tekufot** — the four solar seasons according to Shmuel or Rav Ada, the arithmetic
  behind Tal uMatar and Birkat HaChama.
- **Torah readings** — the weekly parsha and its aliyot, festival and special readings,
  and haftarot across twenty-six customs, with the sizes and offsets a caller needs to
  walk the results from another language.

## Using it

```c
#include "hconverter.h"
#include "zmanim.h"

/* A Gregorian date as a Jewish one */
hc_date date = { GREGORIAN, 2026, 9, 18 };
hc_convert(&date, HEBREW);          /* 7 Tishrei 5787: year 5787, month 7, day 7 */

/* The day's zmanim in Montreal */
hc_zmanim z;
hc_date   day = { GREGORIAN, 2026, 9, 18 };
hc_compute_zmanim(&day, 45.5017, -73.5673, -4.0, 0, 0, &z);
double sunset = z.shkiah;            /* minutes from local midnight */
```

## Building

```bash
git clone --recurse-submodules https://github.com/imush/calendar.git
cd calendar
cmake -S . -B build && cmake --build build
./build/hconverter_tests
```

Or from another CMake project, against a released tag:

```cmake
include(FetchContent)
FetchContent_Declare(hconverter
  GIT_REPOSITORY https://github.com/imush/calendar.git
  GIT_TAG        v2.0.5)
FetchContent_MakeAvailable(hconverter)
target_link_libraries(your_target PRIVATE hconverter)
```

Calendar names and reading schedules come from the shared
[hebrewcalendar-data](https://github.com/imush/hebrewcalendar-data) repository, included
as a submodule and generated into C headers.

## Checked against its sibling

A [Java library](https://imush.github.io/hebrewcalendar/) computes the same calendar for
the web. The two are diffed day by day, and both are checked against
[opentorah](https://www.opentorah.org), the source of the reading data.

## License

BSD 3-Clause. See [LICENSE](LICENSE).
