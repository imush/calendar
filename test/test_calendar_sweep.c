/*!
 * \file test_calendar_sweep.c
 * \brief Every day of 200 years, checked against arithmetic the library does
 *        not share.
 *
 * The conversions were compared day by day against the Java library and agreed
 * on the Hebrew dates and disagreed on every Julian one -- the two calendars
 * had been sharing an epoch constant, though Gregorian 1 January 1 CE is
 * Julian 3 January 1 CE. That is the kind of error a handful of spot dates
 * misses entirely: it was wrong everywhere, by the same two days, so no single
 * date looked odd beside its neighbours.
 *
 * Rather than keep a diff against another library, the expected Julian date is
 * computed here from Julian Day Number arithmetic (Fliegel-Van Flandern),
 * which shares no code with the library under test. The Hebrew side is checked
 * by round trip, there being no independent formula to hand.
 */
#include "hc_test.h"
#include "hconverter.h"

static long g_to_jdn(int y, int m, int d)
{
    long a = (14 - m) / 12, yy = y + 4800 - a, mm = m + 12 * a - 3;
    return d + (153 * mm + 2) / 5 + 365 * yy + yy / 4 - yy / 100 + yy / 400 - 32045;
}

static void jdn_to_julian(long jdn, int *y, int *m, int *d)
{
    long c = jdn + 32082, dq = (4 * c + 3) / 1461, e = c - 1461 * dq / 4;
    long mm = (5 * e + 2) / 153;
    *d = (int)(e - (153 * mm + 2) / 5 + 1);
    *m = (int)(mm + 3 - 12 * (mm / 10));
    *y = (int)(dq - 4800 + mm / 10);
}

void test_calendar_sweep(void)
{
    int julian_bad = 0, hebrew_bad = 0, days = 0;
    char first_j[128] = "", first_h[128] = "";

    for (int y = 1900; y <= 2099; y++) {
        for (int m = 1; m <= 12; m++) {
            int len = hc_get_month_length(y, m, GREGORIAN);
            for (int d = 1; d <= len; d++) {
                days++;

                hc_date j; j.calendar_type = GREGORIAN; j.year = y; j.month = m; j.day = d;
                int ey, em, ed;
                jdn_to_julian(g_to_jdn(y, m, d), &ey, &em, &ed);
                if (hc_convert(&j, JULIAN) != 0 ||
                    j.year != ey || j.month != em || j.day != ed) {
                    if (!julian_bad++)
                        snprintf(first_j, sizeof(first_j),
                                 "%04d-%02d-%02d: expected Julian %04d-%02d-%02d, got %04d-%02d-%02d",
                                 y, m, d, ey, em, ed, j.year, j.month, j.day);
                }

                /* Hebrew there and back: a shifted epoch would land the round
                 * trip on a different Gregorian day. */
                hc_date h; h.calendar_type = GREGORIAN; h.year = y; h.month = m; h.day = d;
                if (hc_convert(&h, HEBREW) != 0 || hc_convert(&h, GREGORIAN) != 0 ||
                    h.year != y || h.month != m || h.day != d) {
                    if (!hebrew_bad++)
                        snprintf(first_h, sizeof(first_h),
                                 "%04d-%02d-%02d round-tripped to %04d-%02d-%02d",
                                 y, m, d, h.year, h.month, h.day);
                }
            }
        }
    }

    HC_ASSERT(days > 70000, "swept two centuries of days");
    if (julian_bad) fprintf(stderr, "    %d Julian days wrong; first: %s\n", julian_bad, first_j);
    if (hebrew_bad) fprintf(stderr, "    %d Hebrew round trips wrong; first: %s\n", hebrew_bad, first_h);
    HC_ASSERT(julian_bad == 0, "every Julian date matches Julian Day Number arithmetic");
    HC_ASSERT(hebrew_bad == 0, "every Hebrew date round-trips");
}
