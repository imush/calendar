/* Port of ParshiotTest.java */
#include "hc_test.h"
#include "../src/parshiot.h"
#include "../src/hconverter.h"

/* ── helpers ─────────────────────────────────────────────────────────────── */

/* 1=Sun..7=Sat */
static int dow_greg(int y, int m, int d)
{
    hc_date dt; dt.calendar_type = GREGORIAN;
    dt.year = y; dt.month = m; dt.day = d;
    int raw = (int)hc_get_day_of_week(&dt);
    return raw == 0 ? 7 : raw;
}

static int get_parsha(int gy, int gm, int gd, int in_israel, hc_reading *r)
{
    hc_date d; d.calendar_type = GREGORIAN;
    d.year = gy; d.month = gm; d.day = gd;
    return hc_get_parsha(&d, in_israel, r);
}

/* Find the first Saturday on or after a given Gregorian date */
static void next_saturday(int *y, int *m, int *d)
{
    while (dow_greg(*y, *m, *d) != 7) {
        (*d)++;
        int ml[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
        int ly = (*y%4==0 && (*y%100!=0 || *y%400==0));
        int max = (*m==2 && ly) ? 29 : ml[*m];
        if (*d > max) { *d=1; (*m)++; if (*m>12){*m=1;(*y)++;} }
    }
}

/* Hebrew → Gregorian Saturday on or after Hebrew date */
static void first_shabbat_after(int hy, int hm, int hd, int *gy, int *gm, int *gd)
{
    hc_date dt; dt.calendar_type = HEBREW;
    dt.year = hy; dt.month = hm; dt.day = hd;
    hc_convert(&dt, GREGORIAN);
    *gy = dt.year; *gm = dt.month; *gd = dt.day;
    next_saturday(gy, gm, gd);
}

/* Last Saturday before or on a Hebrew date */
static void last_shabbat_before(int hy, int hm, int hd, int *gy, int *gm, int *gd)
{
    hc_date dt; dt.calendar_type = HEBREW;
    dt.year = hy; dt.month = hm; dt.day = hd;
    hc_convert(&dt, GREGORIAN);
    *gy = dt.year; *gm = dt.month; *gd = dt.day;
    /* go back until Saturday (or stay if already Saturday) */
    while (dow_greg(*gy, *gm, *gd) != 7) {
        (*gd)--;
        if (*gd < 1) {
            (*gm)--;
            if (*gm < 1) { *gm = 12; (*gy)--; }
            int ml[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
            int ly = (*gy%4==0 && (*gy%100!=0 || *gy%400==0));
            *gd = (*gm==2 && ly) ? 29 : ml[*gm];
        }
    }
}

/* ── tests ───────────────────────────────────────────────────────────────── */

void test_parshiot(void)
{
    /* Fixed known date: May 30, 2026 = Saturday
       Diaspora: Naso  |  Israel: Behaalotecha */
    {
        hc_reading rd, ri;
        HC_ASSERT_EQ_INT(0, get_parsha(2026, 5, 30, 0, &rd));
        HC_ASSERT_EQ_INT(0, get_parsha(2026, 5, 30, 1, &ri));
        HC_ASSERT_EQ_INT(7, dow_greg(2026, 5, 30)); /* must be Saturday */
        HC_ASSERT_EQ_INT(HC_NASO,         (int)rd.p1);
        HC_ASSERT_EQ_INT(HC_PARSHA_NONE,  (int)rd.p2);
        HC_ASSERT_EQ_INT(HC_BEHAALOTECHA, (int)ri.p1);
    }

    /* First Shabbat after Simchat Torah (Tishrei 22 Israel / 23 Diaspora)
       should be Bereishit for years 5750-5820 */
    for (int y = 5750; y <= 5820; y++) {
        /* Israel: Simchat Torah = Tishrei 22; first Shabbat on/after Tishrei 23 = Shabbat Bereishit */
        int gy, gm, gd;
        first_shabbat_after(y, 7, 23, &gy, &gm, &gd);
        hc_reading r;
        int rc = get_parsha(gy, gm, gd, 0, &r);
        if (rc == 0) {
            HC_ASSERT_EQ_INT(HC_BEREISHIT, (int)r.p1);
        }
    }

    /* Shabbat before Pesach must be Tzav or Metzora (non-leap) or Acharei (leap Thu RH) */
    for (int y = 5750; y <= 5820; y++) {
        int gy, gm, gd;
        last_shabbat_before(y, 1, 14, &gy, &gm, &gd); /* before Erev Pesach */
        hc_reading r;
        if (get_parsha(gy, gm, gd, 0, &r) == 0 && r.p1 != HC_PARSHA_NONE) {
            /* must be Tzav, Metzora, or Acharei Mot */
            int ok = (r.p1 == HC_TZAV || r.p1 == HC_METZORA || r.p1 == HC_ACHAREI_MOT);
            HC_ASSERT(ok, "Shabbat before Pesach should be Tzav/Metzora/Acharei");
        }
    }

    /* Nitzavim: standalone iff next year's RH is Mon or Tue */
    for (int y = 5750; y <= 5820; y++) {
        hc_date rh_next; rh_next.calendar_type = HEBREW;
        rh_next.year = y+1; rh_next.month = 7; rh_next.day = 1;
        int next_rh_dow = (int)hc_get_day_of_week(&rh_next);
        /* 2=Mon, 3=Tue in 0=Sat,1=Sun..6=Fri encoding from hc_get_day_of_week */
        int expect_standalone = (next_rh_dow == 2 || next_rh_dow == 3);

        /* find Nitzavim in both schedules */
        /* scan all Shabbatot of year y */
        int gy, gm, gd;
        first_shabbat_after(y, 7, 1, &gy, &gm, &gd);
        int found_combined = 0;
        for (int w = 0; w < 60; w++) {
            hc_reading r;
            if (get_parsha(gy, gm, gd, 0, &r) == 0) {
                if (r.p1 == HC_NITZAVIM && r.p2 == HC_VAYEILECH)
                    found_combined = 1;
            }
            /* advance 7 days */
            gd += 7;
            int ml[]={0,31,28,31,30,31,30,31,31,30,31,30,31};
            int ly=(gy%4==0&&(gy%100!=0||gy%400==0));
            int max=(gm==2&&ly)?29:ml[gm];
            while (gd > max) {
                gd -= max; gm++;
                if (gm > 12) { gm = 1; gy++; }
                ly=(gy%4==0&&(gy%100!=0||gy%400==0));
                max=(gm==2&&ly)?29:ml[gm];
            }
        }
        if (expect_standalone) {
            HC_ASSERT_FALSE(found_combined);
        } else {
            HC_ASSERT_TRUE(found_combined);
        }
    }

    /* Non-Saturday should return error */
    {
        hc_reading r;
        HC_ASSERT_EQ_INT(-1, get_parsha(2026, 5, 29, 0, &r)); /* Friday */
    }
}
