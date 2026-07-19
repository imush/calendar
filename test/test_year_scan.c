/*
 * Year-scan test — for a range of Hebrew years, enumerate every day and
 * check that each singleton special day appears exactly once per year
 * (or with the specific expected count for multi-day categories).
 *
 * Runs for both Diaspora and Israel modes so location-specific enums
 * are covered.
 */
#include "hc_test.h"
#include "../src/hc_jewish_dates.h"
#include "../src/hconverter.h"

#define FIRST_YEAR 5730
#define LAST_YEAR  5830

typedef enum {
    KIND_ALWAYS_ONCE,      /* Exactly 1 per year (in applicable location) */
    KIND_LEAP_ONLY,        /* 1 in leap year, 0 in non-leap */
    KIND_NON_LEAP_ONLY,    /* 1 in non-leap year, 0 in leap */
    KIND_RANGE,            /* min..max per year */
    KIND_SKIP,             /* Not covered by this test */
} kind_t;

typedef struct {
    hc_special_day sd;
    kind_t         kind;
    int            min;    /* only for KIND_RANGE */
    int            max;
} expect_t;

/* Expectation table. Location filtering is applied via hc_sd_applies(): if
 * the day doesn't apply in the current location, it must appear 0 times. */
static const expect_t EXPECT[] = {
    /* Simple singletons — one occurrence per year in applicable location */
    { HC_SD_NISAN_11,                KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_EREV_PESACH,             KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_PESACH_1,                KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_PESACH_2_C,              KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_PESACH_7,                KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_PESACH_LAST_C,           KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_PESACH_SHENI,            KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_LAG_BAOMER,              KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_SHAVUOT,                 KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_SHAVUOT_2_C,             KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_TAMUZ_3,                 KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_TAMUZ_12,                KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_TAMUZ_13,                KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_FAST_17_TAMUZ,           KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_FAST_9_AV,               KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_CHAI_ELUL,               KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_ROSH_HASHANA_1,          KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_ROSH_HASHANA_2,          KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_TZOM_GEDALIA,            KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_EREV_YOM_KIPPUR,         KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_YOM_KIPPUR,              KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_SUKKOT_1,                KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_SUKKOT_2_C,              KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_HOSHANA_RABBA,           KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_SHMINI_ATZERET_C,        KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_SIMCHAT_TORAH_C,         KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_SIMCHAT_TORAH_I,         KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_NINETEENTH_KISLEV,       KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_CHANUKAH_1,              KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_CHANUKAH_2,              KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_CHANUKAH_3,              KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_CHANUKAH_4,              KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_CHANUKAH_5,              KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_CHANUKAH_6,              KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_CHANUKAH_7,              KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_CHANUKAH_8,              KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_TENTH_TEVET,             KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_YUD_SHVAT,               KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_TU_BESHVAT,              KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_TAANIT_ESTHER,           KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_PURIM,                   KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_SHUSHAN_PURIM,           KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_SHABBAT_SHEKALIM,        KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_SHABBAT_ZACHOR,          KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_SHABBAT_PARA,            KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_SHABBAT_HACHODESH,       KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_SHABBAT_HAGADOL,         KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_SHABBAT_CHAZON,          KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_SHABBAT_NACHAMU,         KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_SHABBAT_SHUVAH,          KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_SHABBAT_SHIRAH,          KIND_ALWAYS_ONCE, 0, 0 },

    /* Chol Hamoed — Diaspora 4 days each, Israel 5 days each */
    { HC_SD_CHOL_HAMOED_PESACH_1_C,  KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_CHOL_HAMOED_PESACH_2_C,  KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_CHOL_HAMOED_PESACH_3_C,  KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_CHOL_HAMOED_PESACH_4_C,  KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_CHOL_HAMOED_PESACH_1_I,  KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_CHOL_HAMOED_PESACH_2_I,  KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_CHOL_HAMOED_PESACH_3_I,  KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_CHOL_HAMOED_PESACH_4_I,  KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_CHOL_HAMOED_PESACH_5_I,  KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_CHOL_HAMOED_SUKKOT_1_C,  KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_CHOL_HAMOED_SUKKOT_2_C,  KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_CHOL_HAMOED_SUKKOT_3_C,  KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_CHOL_HAMOED_SUKKOT_4_C,  KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_CHOL_HAMOED_SUKKOT_1_I,  KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_CHOL_HAMOED_SUKKOT_2_I,  KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_CHOL_HAMOED_SUKKOT_3_I,  KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_CHOL_HAMOED_SUKKOT_4_I,  KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_CHOL_HAMOED_SUKKOT_5_I,  KIND_ALWAYS_ONCE, 0, 0 },

    /* Leap-year only */
    { HC_SD_PURIM_KATAN,             KIND_LEAP_ONLY,   0, 0 },

    /* Tal Umatar: 1 start day per Hebrew year in applicable location */
    { HC_SD_TAL_UMATAR_I,            KIND_ALWAYS_ONCE, 0, 0 },
    { HC_SD_TAL_UMATAR_C,            KIND_ALWAYS_ONCE, 0, 0 },

    /* Rosh Chodesh: one or two days for each of 12 (13 in leap) months,
     * excluding Tishrei — bounded, not a singleton. */
    { HC_SD_ROSH_CHODESH,            KIND_RANGE,      12, 20 },

    /* Shabbat Mevarchim: one per non-Tishrei month = 11 (or 12 leap). */
    { HC_SD_SHABBAT_MEVARCHIM,       KIND_RANGE,      11, 12 },

    /* Eruv Tavshilin: 0 to a handful per year, depending on YT placement. */
    { HC_SD_ERUV_TAVSHILIN_C,        KIND_RANGE,       0,  6 },
    { HC_SD_ERUV_TAVSHILIN_I,        KIND_RANGE,       0,  6 },

    /* Birkat HaChama recurs every 28 civil years; a Hebrew-year scan may
     * or may not include one — not asserted here. */
    { HC_SD_BIRKAT_HACHAMA,          KIND_SKIP,        0,  0 },
};
#define N_EXPECT (int)(sizeof(EXPECT)/sizeof(EXPECT[0]))

/* Scan every day of a Hebrew year, filling counts[sd] with occurrences. */
static void scan_year(int year, int in_israel, int counts[HC_SD_COUNT])
{
    for (int i = 0; i < HC_SD_COUNT; i++) counts[i] = 0;

    /* Hebrew year runs Tishrei (7) through Elul (6). */
    static const int MONTH_ORDER[] = {
        7, 8, 9, 10, 11, 12, 13, 1, 2, 3, 4, 5, 6
    };
    int leap = hc_is_leap_year(year, HEBREW);
    for (int mi = 0; mi < 13; mi++) {
        int m = MONTH_ORDER[mi];
        if (m == 13 && !leap) continue;
        int mlen = hc_get_month_length(year, m, HEBREW);
        for (int d = 1; d <= mlen; d++) {
            hc_date dt; dt.calendar_type = HEBREW;
            dt.year = year; dt.month = m; dt.day = d;
            hc_special_day days[HC_MAX_SPECIAL_DAYS];
            int cnt = 0;
            if (hc_get_special_days(&dt, in_israel, days, &cnt) != 0) continue;
            for (int k = 0; k < cnt; k++) {
                if (days[k] > HC_SD_NONE && days[k] < HC_SD_COUNT)
                    counts[days[k]]++;
            }
        }
    }
}

static void check_expectations(int year, int in_israel, const int counts[HC_SD_COUNT])
{
    int leap = hc_is_leap_year(year, HEBREW);
    for (int e = 0; e < N_EXPECT; e++) {
        hc_special_day sd = EXPECT[e].sd;
        int got = counts[sd];
        int applies = hc_sd_applies(sd, in_israel);
        int expected_min = -1, expected_max = -1;

        switch (EXPECT[e].kind) {
            case KIND_ALWAYS_ONCE:
                expected_min = expected_max = applies ? 1 : 0;
                break;
            case KIND_LEAP_ONLY:
                expected_min = expected_max = (applies && leap) ? 1 : 0;
                break;
            case KIND_NON_LEAP_ONLY:
                expected_min = expected_max = (applies && !leap) ? 1 : 0;
                break;
            case KIND_RANGE:
                expected_min = applies ? EXPECT[e].min : 0;
                expected_max = applies ? EXPECT[e].max : 0;
                break;
            case KIND_SKIP:
                continue;
        }

        int ok = (got >= expected_min && got <= expected_max);
        hc_tests_run++;
        if (ok) {
            hc_tests_passed++;
        } else {
            hc_tests_failed++;
            fprintf(stderr,
                "FAIL year_scan year=%d %s sd=%s(%d): got %d, expected [%d..%d]\n",
                year, in_israel ? "IL" : "CH",
                hc_sd_name(sd) ? hc_sd_name(sd) : "?", (int)sd,
                got, expected_min, expected_max);
        }
    }
}

/* All non-Mevarchim named Shabbats must occur on Shabbat. Spot-check by
 * asserting the total count of enumerated day-of-week matches expectation. */
static void check_named_shabbats_on_shabbat(int year)
{
    static const int MONTH_ORDER[] = {
        7, 8, 9, 10, 11, 12, 13, 1, 2, 3, 4, 5, 6
    };
    int leap = hc_is_leap_year(year, HEBREW);
    for (int mi = 0; mi < 13; mi++) {
        int m = MONTH_ORDER[mi];
        if (m == 13 && !leap) continue;
        int mlen = hc_get_month_length(year, m, HEBREW);
        for (int d = 1; d <= mlen; d++) {
            hc_date dt; dt.calendar_type = HEBREW;
            dt.year = year; dt.month = m; dt.day = d;
            hc_special_day days[HC_MAX_SPECIAL_DAYS];
            int cnt = 0;
            hc_get_special_days(&dt, 0, days, &cnt);
            for (int k = 0; k < cnt; k++) {
                if (!hc_sd_is_named_shabbat(days[k])) continue;
                /* hc_get_day_of_week returns 0=Sat,1=Sun..6=Fri (see zmanim.c). */
                int dow = (int)hc_get_day_of_week(&dt);
                hc_tests_run++;
                if (dow == 0) {
                    hc_tests_passed++;
                } else {
                    hc_tests_failed++;
                    fprintf(stderr,
                        "FAIL year_scan year=%d %d-%d-%d dow=%d: named shabbat %s on non-Shabbat\n",
                        year, dt.year, dt.month, dt.day, dow,
                        hc_sd_name(days[k]) ? hc_sd_name(days[k]) : "?");
                }
            }
        }
    }
}

void test_year_scan(void)
{
    int counts[HC_SD_COUNT];
    for (int y = FIRST_YEAR; y <= LAST_YEAR; y++) {
        scan_year(y, 0, counts);
        check_expectations(y, 0, counts);
        scan_year(y, 1, counts);
        check_expectations(y, 1, counts);
        check_named_shabbats_on_shabbat(y);
    }
}
