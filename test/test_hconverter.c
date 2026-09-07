/* Port of JewishCalendarImplTest + GregorianCalendarTest + JulianCalendarTest */
#include "hc_test.h"
#include "../src/hconverter.h"
#include "../src/hc_internal.h"

/* ── helpers ─────────────────────────────────────────────────────────────── */

static long abs_day(hc_calendar_type ct, int y, int m, int d)
{
    hc_date dt;
    dt.calendar_type = ct;
    dt.year = y; dt.month = m; dt.day = d;
    hc_cal_impl *impl = get_calendar(ct);
    return impl->abs_date(y, m, d);
}

/* 1=Sun..7=Sat */
static int dow_heb(int y, int m, int d)
{
    hc_date dt; dt.calendar_type = HEBREW;
    dt.year = y; dt.month = m; dt.day = d;
    int raw = (int)hc_get_day_of_week(&dt); /* 0=Sat,1=Sun..6=Fri in public API */
    /* convert to 1=Sun..7=Sat */
    return raw == 0 ? 7 : raw;
}

/* ── tests ───────────────────────────────────────────────────────────────── */

void test_hconverter(void)
{
    /* isLeap */
    HC_ASSERT_FALSE(hc_is_leap_year(5777, HEBREW));
    HC_ASSERT_FALSE(hc_is_leap_year(5778, HEBREW));
    HC_ASSERT_TRUE (hc_is_leap_year(5779, HEBREW));
    HC_ASSERT_TRUE (hc_is_leap_year(19,   HEBREW));

    /* Rosh Hashana abs day (Java: absDayRoshHashana(5777) == 2109669) */
    long rh5777 = abs_day(HEBREW, 5777, 7, 1);
    long rh5778 = abs_day(HEBREW, 5778, 7, 1);
    HC_ASSERT_EQ_LONG(2109669, rh5777);
    HC_ASSERT_EQ_LONG(2110022, rh5778);

    /* absDayRoshHashana(1) == 2 */
    HC_ASSERT_EQ_LONG(2, abs_day(HEBREW, 1, 7, 1));

    /* year type */
    HC_ASSERT_EQ_INT(0, (int)hc_get_heb_year_type(5777)); /* SHORT */
    HC_ASSERT_EQ_INT(1, (int)hc_get_heb_year_type(5778)); /* NORMAL */
    HC_ASSERT_EQ_INT(2, (int)hc_get_heb_year_type(5779)); /* FULL */
    HC_ASSERT_EQ_INT(2, (int)hc_get_heb_year_type(5780)); /* FULL */
    HC_ASSERT_EQ_INT(0, (int)hc_get_heb_year_type(5781)); /* SHORT */

    /* year length 5775→5776 = 354 days */
    long rh5775 = abs_day(HEBREW, 5775, 7, 1);
    long rh5776 = abs_day(HEBREW, 5776, 7, 1);
    HC_ASSERT_EQ_LONG(354, rh5776 - rh5775);

    /* day of week */
    HC_ASSERT_EQ_INT(2, dow_heb(5777, 7, 1)); /* Mon */
    HC_ASSERT_EQ_INT(4, dow_heb(5777, 8, 1)); /* Wed */
    HC_ASSERT_EQ_INT(5, dow_heb(5777, 9, 1)); /* Thu */

    /* Gregorian → Hebrew conversion: Oct 3, 2016 = 5777 Tishrei 1 */
    {
        hc_date d;
        set_hc_date(&d, 2016, 10, 3, GREGORIAN);
        hc_convert(&d, HEBREW);
        HC_ASSERT_EQ_INT(5777, d.year);
        HC_ASSERT_EQ_INT(7,    d.month);
        HC_ASSERT_EQ_INT(1,    d.day);
    }

    /* Hebrew → Gregorian */
    {
        hc_date d;
        set_hc_date(&d, 5777, 7, 1, HEBREW);
        hc_convert(&d, GREGORIAN);
        HC_ASSERT_EQ_INT(2016, d.year);
        HC_ASSERT_EQ_INT(10,   d.month);
        HC_ASSERT_EQ_INT(3,    d.day);
    }

    /* Molad of Tishrei year 1 = BaHaRaD = day 2, 5h, 204p */
    {
        hc_date md; heb_time mt;
        hc_compute_molad(1, 7, HEBREW, &md, &mt);
        HC_ASSERT_EQ_INT(5,   mt.hour);
        HC_ASSERT_EQ_INT(204, mt.part);
        HC_ASSERT_EQ_INT(0,   mt.rega);
    }

    /* Keviut 5777: RH=Monday(2), Pesach=Thu(5), SHORT(0), not leap */
    {
        int rh, pesach, ck, leap;
        hc_compute_keviut(5777, &rh, &pesach, &ck, &leap);
        HC_ASSERT_EQ_INT(2, rh);
        HC_ASSERT_EQ_INT(3, pesach); /* Tue: abs%7=3 in 0=Sat convention */
        HC_ASSERT_EQ_INT(0, ck);    /* SHORT */
        HC_ASSERT_EQ_INT(0, leap);
    }

    /* Sefira: Nisan 16 (1 day after Pesach) = 1st day of Omer */
    {
        long abs_pesach = abs_day(HEBREW, 5777, 1, 15);
        long abs_n16    = abs_day(HEBREW, 5777, 1, 16);
        HC_ASSERT_EQ_LONG(1, abs_n16 - abs_pesach);
    }

    /* Gregorian year type boundary: Dec 31 → Jan 1 */
    {
        hc_date d;
        set_hc_date(&d, 2016, 12, 31, GREGORIAN);
        hc_convert(&d, HEBREW);
        HC_ASSERT_EQ_INT(5777, d.year);

        set_hc_date(&d, 2017, 1, 1, GREGORIAN);
        hc_convert(&d, HEBREW);
        HC_ASSERT_EQ_INT(5777, d.year);
    }

    /* Julian → Gregorian */
    {
        hc_date d;
        set_hc_date(&d, 100, 3, 1, JULIAN);
        hc_convert(&d, GREGORIAN);
        /* Julian 1 March 100 = proleptic Gregorian 28 February 100. By the
           second century the Julian calendar has drifted a day ahead of the
           Gregorian, and the epochs differ by two more: Gregorian 1 Jan 1 CE
           is Julian 3 Jan 1 CE. Checked against Julian Day Number arithmetic;
           this expected 2 March while the two calendars shared one epoch. */
        HC_ASSERT_EQ_INT(100, d.year);
        HC_ASSERT_EQ_INT(2,   d.month);
        HC_ASSERT_EQ_INT(28,  d.day);
    }

    /* hc_check */
    {
        hc_date valid; set_hc_date(&valid, 5777, 7, 1, HEBREW);
        HC_ASSERT_TRUE(hc_check(&valid));
        hc_date bad;   set_hc_date(&bad,   5777, 13, 1, HEBREW); /* no Adar II */
        HC_ASSERT_FALSE(hc_check(&bad));
    }

    /* hc_yahrzeit_for: 30 Cheshvan / 30 Kislev fixed-date rule
     * 5779=FULL, 5780=FULL, 5781=SHORT (verified above via hc_get_heb_year_type) */

    /* Died 30 Cheshvan 5779 — first year 5780 is FULL: 30 Cheshvan when it exists, else 1 Kislev */
    {
        hc_date out;
        /* FULL target year: 30 Cheshvan exists → 30 Cheshvan */
        HC_ASSERT_EQ_INT(0, hc_yahrzeit_for(5779, 8, 30, 5780, &out));
        HC_ASSERT_EQ_INT(8,    out.month); /* Cheshvan */
        HC_ASSERT_EQ_INT(30,   out.day);
        HC_ASSERT_EQ_INT(5780, out.year);
        /* SHORT target year: 30 Cheshvan absent → 1 Kislev */
        HC_ASSERT_EQ_INT(0, hc_yahrzeit_for(5779, 8, 30, 5781, &out));
        HC_ASSERT_EQ_INT(9,    out.month); /* Kislev */
        HC_ASSERT_EQ_INT(1,    out.day);
        HC_ASSERT_EQ_INT(5781, out.year);
    }

    /* Died 30 Cheshvan 5780 — first year 5781 is SHORT → last day of Cheshvan in target */
    {
        hc_date out;
        /* SHORT target year (5781): Cheshvan has 29 days → 29 Cheshvan */
        HC_ASSERT_EQ_INT(0, hc_yahrzeit_for(5780, 8, 30, 5781, &out));
        HC_ASSERT_EQ_INT(8,    out.month); /* Cheshvan */
        HC_ASSERT_EQ_INT(29,   out.day);
        HC_ASSERT_EQ_INT(5781, out.year);
        /* NORMAL target year (5778): Cheshvan has 29 days → 29 Cheshvan */
        HC_ASSERT_EQ_INT(0, hc_yahrzeit_for(5780, 8, 30, 5778, &out));
        HC_ASSERT_EQ_INT(8,    out.month);
        HC_ASSERT_EQ_INT(29,   out.day);
        HC_ASSERT_EQ_INT(5778, out.year);
        /* FULL target year (5779): Cheshvan has 30 days → 30 Cheshvan */
        HC_ASSERT_EQ_INT(0, hc_yahrzeit_for(5780, 8, 30, 5779, &out));
        HC_ASSERT_EQ_INT(8,    out.month);
        HC_ASSERT_EQ_INT(30,   out.day);
        HC_ASSERT_EQ_INT(5779, out.year);
    }

    /* Died 30 Kislev 5779 — first year 5780 is FULL: 30 Kislev when it exists, else 1 Tevet */
    {
        hc_date out;
        /* FULL target year: 30 Kislev exists → 30 Kislev */
        HC_ASSERT_EQ_INT(0, hc_yahrzeit_for(5779, 9, 30, 5780, &out));
        HC_ASSERT_EQ_INT(9,    out.month); /* Kislev */
        HC_ASSERT_EQ_INT(30,   out.day);
        HC_ASSERT_EQ_INT(5780, out.year);
        /* SHORT target year: 30 Kislev absent → 1 Tevet */
        HC_ASSERT_EQ_INT(0, hc_yahrzeit_for(5779, 9, 30, 5781, &out));
        HC_ASSERT_EQ_INT(10,   out.month); /* Tevet */
        HC_ASSERT_EQ_INT(1,    out.day);
        HC_ASSERT_EQ_INT(5781, out.year);
    }

    /* Died 30 Kislev 5780 — first year 5781 is SHORT → last day of Kislev in target */
    {
        hc_date out;
        /* SHORT target year (5781): Kislev has 29 days → 29 Kislev */
        HC_ASSERT_EQ_INT(0, hc_yahrzeit_for(5780, 9, 30, 5781, &out));
        HC_ASSERT_EQ_INT(9,    out.month); /* Kislev */
        HC_ASSERT_EQ_INT(29,   out.day);
        HC_ASSERT_EQ_INT(5781, out.year);
        /* NORMAL target year (5778): Kislev has 30 days → 30 Kislev */
        HC_ASSERT_EQ_INT(0, hc_yahrzeit_for(5780, 9, 30, 5778, &out));
        HC_ASSERT_EQ_INT(9,    out.month);
        HC_ASSERT_EQ_INT(30,   out.day);
        HC_ASSERT_EQ_INT(5778, out.year);
        /* FULL target year (5779): Kislev has 30 days → 30 Kislev */
        HC_ASSERT_EQ_INT(0, hc_yahrzeit_for(5780, 9, 30, 5779, &out));
        HC_ASSERT_EQ_INT(9,    out.month);
        HC_ASSERT_EQ_INT(30,   out.day);
        HC_ASSERT_EQ_INT(5779, out.year);
    }

    /* Adar: yahrzeit vs anniversary differ when orig is non-leap Adar and target is leap.
     * 5778=non-leap (month 12 = Adar), 5779=leap (month 12=Adar I, 13=Adar II). */

    /* Died non-leap Adar 5778 → target leap year 5779:
       yahrzeit stays Adar I (month 12); anniversary moves to Adar II (month 13). */
    {
        hc_date out;
        HC_ASSERT_EQ_INT(0, hc_yahrzeit_for(5778, 12, 15, 5779, &out));
        HC_ASSERT_EQ_INT(12,   out.month); /* Adar I — yahrzeit stays */
        HC_ASSERT_EQ_INT(15,   out.day);
        HC_ASSERT_EQ_INT(5779, out.year);

        HC_ASSERT_EQ_INT(0, hc_anniversary_for(5778, 12, 15, 5779, &out));
        HC_ASSERT_EQ_INT(13,   out.month); /* Adar II — anniversary moves */
        HC_ASSERT_EQ_INT(15,   out.day);
        HC_ASSERT_EQ_INT(5779, out.year);
    }

    /* Died Adar II 5779 (month 13) → target non-leap 5781: both fall back to Adar (month 12). */
    {
        hc_date out;
        HC_ASSERT_EQ_INT(0, hc_yahrzeit_for(5779, 13, 15, 5781, &out));
        HC_ASSERT_EQ_INT(12,   out.month); /* Adar */
        HC_ASSERT_EQ_INT(15,   out.day);
        HC_ASSERT_EQ_INT(5781, out.year);

        HC_ASSERT_EQ_INT(0, hc_anniversary_for(5779, 13, 15, 5781, &out));
        HC_ASSERT_EQ_INT(12,   out.month);
        HC_ASSERT_EQ_INT(15,   out.day);
        HC_ASSERT_EQ_INT(5781, out.year);
    }
}
