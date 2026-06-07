/* Port of TekufaCalc tests.
   Known value: Tekufat Nisan of Shmuel in year 5769 falls on Wednesday April 8, 2009. */
#include "hc_test.h"
#include "../src/tekufot.h"
#include "../src/hconverter.h"
#include "../src/hc_internal.h"

void test_tekufot(void)
{
    /* Shmuel Tekufat Nisan 5769:
       Gregorian April 8, 2009 = Wednesday.
       abs day of April 8 2009: convert and check. */
    {
        long abs_day;
        heb_time t;
        int rc = hc_get_tekufa(HC_SHMUEL, 5769, HC_SEASON_NISAN, &abs_day, &t);
        HC_ASSERT_EQ_INT(0, rc);

        hc_date d;
        d.calendar_type = HEBREW;
        get_calendar(HEBREW)->compute_date(abs_day, &d);
        hc_convert(&d, GREGORIAN);
        HC_ASSERT_EQ_INT(2009, d.year);
        HC_ASSERT_EQ_INT(4,    d.month);
        HC_ASSERT_EQ_INT(8,    d.day);

        /* Must be Wednesday (day_of_week of April 8, 2009):
           hc_get_day_of_week returns 0=Sun..6=Sat. April 8, 2009 = Wednesday (4). */
        hc_date greg; set_hc_date(&greg, 2009, 4, 8, GREGORIAN);
        HC_ASSERT_EQ_INT(4, (int)hc_get_day_of_week(&greg)); /* Wednesday */
    }

    /* Shmuel Tekufat Nisan 5797 should also be Wednesday April 8, 2037 (28 years later) */
    {
        long abs_day;
        heb_time t;
        hc_get_tekufa(HC_SHMUEL, 5797, HC_SEASON_NISAN, &abs_day, &t);
        hc_date d;
        d.calendar_type = HEBREW;
        get_calendar(HEBREW)->compute_date(abs_day, &d);
        hc_convert(&d, GREGORIAN);
        HC_ASSERT_EQ_INT(2037, d.year);
        HC_ASSERT_EQ_INT(4,    d.month);
        HC_ASSERT_EQ_INT(8,    d.day);
    }

    /* Shmuel: 4 seasons per year, spacing = 91d 7h 540p (no rega) */
    {
        long d0, d1;
        heb_time t0, t1;
        hc_get_tekufa(HC_SHMUEL, 5780, HC_SEASON_NISAN,   &d0, &t0);
        hc_get_tekufa(HC_SHMUEL, 5780, HC_SEASON_TAMMUZ,  &d1, &t1);
        long diff_days = d1 - d0;
        int  diff_hrs  = t1.hour - t0.hour;
        int  diff_part = t1.part - t0.part;
        /* carry */
        if (diff_part < 0) { diff_part += 1080; diff_hrs--; }
        if (diff_hrs < 0)  { diff_hrs  += 24;   diff_days--; }
        HC_ASSERT_EQ_LONG(91, diff_days);
        HC_ASSERT_EQ_INT (7,  diff_hrs);
        HC_ASSERT_EQ_INT (540,diff_part);
    }

    /* Rav Ada: season length carries non-zero rega (31) */
    {
        long d0, d1;
        heb_time t0, t1;
        hc_get_tekufa(HC_RAV_ADA, 5780, HC_SEASON_NISAN,  &d0, &t0);
        hc_get_tekufa(HC_RAV_ADA, 5780, HC_SEASON_TAMMUZ, &d1, &t1);
        long diff_days = d1 - d0;
        int  diff_hrs  = t1.hour - t0.hour;
        int  diff_part = t1.part - t0.part;
        int  diff_rega = t1.rega - t0.rega;
        if (diff_rega < 0) { diff_rega += 76; diff_part--; }
        if (diff_part < 0) { diff_part += 1080; diff_hrs--; }
        if (diff_hrs < 0)  { diff_hrs  += 24;   diff_days--; }
        HC_ASSERT_EQ_LONG(91,  diff_days);
        HC_ASSERT_EQ_INT (7,   diff_hrs);
        HC_ASSERT_EQ_INT (519, diff_part);
        HC_ASSERT_EQ_INT (31,  diff_rega);
    }

    /* Invalid input */
    {
        long d; heb_time t;
        HC_ASSERT_EQ_INT(-1, hc_get_tekufa(HC_SHMUEL, 0, HC_SEASON_NISAN, &d, &t));
        HC_ASSERT_EQ_INT(-1, hc_get_tekufa(HC_SHMUEL, 1, (hc_season)4,    &d, &t));
    }
}
