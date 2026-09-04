/*
 * C oracle for cross-validation against the Java library.
 * Produces the same output format as CrossValidate.java so the two
 * can be diffed directly.
 *
 * Format:
 *   SD YYYY-MM-DD {D|I} NAME1,NAME2,...|-     (sorted by Java enum name)
 *   ZM YYYY-MM-DD LOCNAME ha sa al tz ch ss t3
 *
 * All zmanim use tz_offset=0 (UTC); "N" for HC_ZMAN_UNAVAILABLE.
 */

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "../src/hc_jewish_dates.h"
#include "../src/hconverter.h"
#include "../src/zmanim.h"

/* ── C-to-Java enum name mapping ─────────────────────────────────────────── */
static const char *JAVA_NAME[HC_SD_COUNT] = {
    [HC_SD_NONE]                    = NULL,
    [HC_SD_NISAN_11]                = "NISAN_11",
    [HC_SD_EREV_PESACH]             = "EREV_PESACH",
    [HC_SD_PESACH_1]                = "FIRST_DAY_PESACH",
    [HC_SD_PESACH_2_C]              = "SECOND_DAY_PESACH_C",
    [HC_SD_PESACH_7]                = "SEVENTH_DAY_PESACH",
    [HC_SD_PESACH_LAST_C]           = "LAST_DAY_PESACH_C",
    [HC_SD_CHOL_HAMOED_PESACH_1_I]  = "CHOL_HAMOED_PESACH_1I",
    [HC_SD_CHOL_HAMOED_PESACH_2_I]  = "CHOL_HAMOED_PESACH_2I",
    [HC_SD_CHOL_HAMOED_PESACH_3_I]  = "CHOL_HAMOED_PESACH_3I",
    [HC_SD_CHOL_HAMOED_PESACH_4_I]  = "CHOL_HAMOED_PESACH_4I",
    [HC_SD_CHOL_HAMOED_PESACH_5_I]  = "CHOL_HAMOED_PESACH_5I",
    [HC_SD_CHOL_HAMOED_PESACH_1_C]  = "CHOL_HAMOED_PESACH_1C",
    [HC_SD_CHOL_HAMOED_PESACH_2_C]  = "CHOL_HAMOED_PESACH_2C",
    [HC_SD_CHOL_HAMOED_PESACH_3_C]  = "CHOL_HAMOED_PESACH_3C",
    [HC_SD_CHOL_HAMOED_PESACH_4_C]  = "CHOL_HAMOED_PESACH_4C",
    [HC_SD_PESACH_SHENI]            = "PESACH_SHENI",
    [HC_SD_LAG_BAOMER]              = "LAG_BAOMER",
    [HC_SD_SHAVUOT]                 = "SHAVUOT",
    [HC_SD_SHAVUOT_2_C]             = "SHAVUOT_2C",
    [HC_SD_TAMUZ_3]                 = "TAMUZ_3",
    [HC_SD_TAMUZ_12]                = "TAMUZ_12",
    [HC_SD_TAMUZ_13]                = "TAMUZ_13",
    [HC_SD_FAST_17_TAMUZ]           = "FAST_TAMUZ_17",
    [HC_SD_FAST_9_AV]               = "FAST_AV_9",
    [HC_SD_CHAI_ELUL]               = "CHAI_ELUL",
    [HC_SD_ROSH_HASHANA_1]          = "ROSH_HASHANA_1",
    [HC_SD_ROSH_HASHANA_2]          = "ROSH_HASHANA_2",
    [HC_SD_ROSH_CHODESH]            = "ROSH_CHODESH",
    [HC_SD_TZOM_GEDALIA]            = "TZOM_GEDALIA",
    [HC_SD_EREV_YOM_KIPPUR]         = "EREV_YOM_KIPPUR",
    [HC_SD_YOM_KIPPUR]              = "YOM_KIPPUR",
    [HC_SD_SUKKOT_1]                = "FIRST_DAY_SUKKOT",
    [HC_SD_SUKKOT_2_C]              = "SECOND_DAY_SUKKOT_C",
    [HC_SD_CHOL_HAMOED_SUKKOT_1_I]  = "CHOL_HAMOED_SUKKOT_1I",
    [HC_SD_CHOL_HAMOED_SUKKOT_2_I]  = "CHOL_HAMOED_SUKKOT_2I",
    [HC_SD_CHOL_HAMOED_SUKKOT_3_I]  = "CHOL_HAMOED_SUKKOT_3I",
    [HC_SD_CHOL_HAMOED_SUKKOT_4_I]  = "CHOL_HAMOED_SUKKOT_4I",
    [HC_SD_CHOL_HAMOED_SUKKOT_5_I]  = "CHOL_HAMOED_SUKKOT_5I",
    [HC_SD_CHOL_HAMOED_SUKKOT_1_C]  = "CHOL_HAMOED_SUKKOT_1C",
    [HC_SD_CHOL_HAMOED_SUKKOT_2_C]  = "CHOL_HAMOED_SUKKOT_2C",
    [HC_SD_CHOL_HAMOED_SUKKOT_3_C]  = "CHOL_HAMOED_SUKKOT_3C",
    [HC_SD_CHOL_HAMOED_SUKKOT_4_C]  = "CHOL_HAMOED_SUKKOT_4C",
    [HC_SD_HOSHANA_RABBA]           = "HOSHANA_RABBA",
    [HC_SD_SHMINI_ATZERET_C]        = "SHMINI_ATZERES_C",
    [HC_SD_SIMCHAT_TORAH_C]         = "SIMCHAT_TORAH_C",
    [HC_SD_SIMCHAT_TORAH_I]         = "SIMCHAT_TORAH_I",
    [HC_SD_NINETEENTH_KISLEV]       = "NINETEENTH_KISLEV",
    [HC_SD_CHANUKAH_1]              = "FIRST_DAY_CHANUKAH",
    [HC_SD_CHANUKAH_2]              = "SECOND_DAY_CHANUKAH",
    [HC_SD_CHANUKAH_3]              = "THIRD_DAY_CHANUKAH",
    [HC_SD_CHANUKAH_4]              = "FOURTH_DAY_CHANUKAH",
    [HC_SD_CHANUKAH_5]              = "FIFTH_DAY_CHANUKAH",
    [HC_SD_CHANUKAH_6]              = "SIXTH_DAY_CHANUKAH",
    [HC_SD_CHANUKAH_7]              = "SEVENTH_DAY_CHANUKAH",
    [HC_SD_CHANUKAH_8]              = "EIGHTH_DAY_CHANUKAH",
    [HC_SD_TENTH_TEVET]             = "TENTH_TEVET",
    [HC_SD_YUD_SHVAT]               = "YUD_SHVAT",
    [HC_SD_TU_BESHVAT]              = "TU_BESHVAT",
    [HC_SD_PURIM_KATAN]             = "PURIM_KATAN",
    [HC_SD_TAANIT_ESTHER]           = "TAANIT_ESTHER",
    [HC_SD_PURIM]                   = "PURIM",
    [HC_SD_SHUSHAN_PURIM]           = "SHUSHAN_PURIM",
    [HC_SD_SHABBAT_SHEKALIM]        = "SHABBAT_SHEKALIM",
    [HC_SD_SHABBAT_ZACHOR]          = "SHABBAT_ZACHOR",
    [HC_SD_SHABBAT_PARA]            = "SHABBAT_PARA",
    [HC_SD_SHABBAT_HACHODESH]       = "SHABBAT_HACHODESH",
    [HC_SD_SHABBAT_HAGADOL]         = "SHABBAT_HAGADOL",
    [HC_SD_SHABBAT_CHAZON]          = "SHABBAT_CHAZON",
    [HC_SD_SHABBAT_NACHAMU]         = "SHABBAT_NACHAMU",
    [HC_SD_SHABBAT_SHUVAH]          = "SHABBAT_SHUVAH",
    [HC_SD_SHABBAT_SHIRAH]          = "SHABBAT_SHIRAH",
    [HC_SD_ERUV_TAVSHILIN_I]        = "ERUV_TAVSHILIN_I",
    [HC_SD_ERUV_TAVSHILIN_C]        = "ERUV_TAVSHILIN_C",
    [HC_SD_TAL_UMATAR_I]            = "TAL_UMATAR_I",
    [HC_SD_TAL_UMATAR_C]            = "TAL_UMATAR_C",
    [HC_SD_BIRKAT_HACHAMA]          = "BIRKAT_HACHAMA",
};

/* ── Locations ───────────────────────────────────────────────────────────── */
typedef struct { const char *name; double lat, lon, tz_h; int in_israel, in_jerusalem; } Loc;

static const Loc LOCS[] = {
    { "JER",  31.7767,   35.2345,  2.0, 1, 1 },
    { "NYC",  40.7128,  -74.0060, -5.0, 0, 0 },
    { "LAX",  34.0522, -118.2437, -8.0, 0, 0 },
    { "MOW",  55.7558,   37.6173,  3.0, 0, 0 },
    { "LED",  59.9311,   30.3609,  3.0, 0, 0 },
    { "PBY",  70.2553, -148.3375, -9.0, 0, 0 },
};
#define N_LOCS (int)(sizeof(LOCS)/sizeof(LOCS[0]))

/* ── Helpers ─────────────────────────────────────────────────────────────── */

static int cmp_str(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

static void fmt_zman(char *buf, double v) {
    if (v < -1e200) { buf[0]='N'; buf[1]='\0'; return; }
    sprintf(buf, "%d", (int)round(v));
}

/* ── Gregorian date iteration ────────────────────────────────────────────── */
static int greg_month_len(int y, int m) {
    static const int ml[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    int leap = (y%4==0 && (y%100!=0 || y%400==0));
    return (m==2 && leap) ? 29 : ml[m];
}
static void next_date(int *y, int *m, int *d) {
    (*d)++;
    if (*d > greg_month_len(*y, *m)) { *d=1; (*m)++; }
    if (*m > 12) { *m=1; (*y)++; }
}

/* ── Main ────────────────────────────────────────────────────────────────── */
int main(void) {
    int y=1950, m=1, d=1;
    const int end_y=2051;

    while (y < end_y) {
        char dateStr[12];
        sprintf(dateStr, "%04d-%02d-%02d", y, m, d);

        hc_date dt;
        dt.calendar_type = GREGORIAN;
        dt.year = y; dt.month = m; dt.day = d;

        /* ── Special days ──────────────────────────────────────────────── */
        for (int isr = 0; isr <= 1; isr++) {
            hc_special_day days[HC_MAX_SPECIAL_DAYS];
            int count = 0;
            hc_get_special_days(&dt, isr, days, &count);

            /* Map to Java names and collect non-null ones */
            const char *jnames[HC_MAX_SPECIAL_DAYS];
            int jcount = 0;
            for (int i = 0; i < count; i++) {
                const char *jn = JAVA_NAME[days[i]];
                if (jn) jnames[jcount++] = jn;
            }
            qsort(jnames, jcount, sizeof(const char *), cmp_str);

            printf("SD %s %c ", dateStr, isr ? 'I' : 'D');
            if (jcount == 0) {
                printf("-");
            } else {
                for (int i = 0; i < jcount; i++) {
                    if (i) putchar(',');
                    printf("%s", jnames[i]);
                }
            }
            putchar('\n');
        }

        /* ── Zmanim ────────────────────────────────────────────────────── */
        for (int li = 0; li < N_LOCS; li++) {
            const Loc *loc = &LOCS[li];
            hc_zmanim zm;
            hc_compute_zmanim(&dt, loc->lat, loc->lon, loc->tz_h,
                              loc->in_jerusalem, loc->in_israel, &zm);

            char ha[16], sa[16], al[16], tz[16], ch[16], ss[16], t3[16];
            fmt_zman(ha, zm.hanetz);
            fmt_zman(sa, zm.shkiah);
            fmt_zman(al, zm.alot_hashachar);
            fmt_zman(tz, zm.tzait_alter_rebbe);
            fmt_zman(ch, zm.chatzot);
            fmt_zman(ss, zm.sof_shma);
            fmt_zman(t3, zm.tzait_3_stars);

            printf("ZM %s %s %s %s %s %s %s %s %s\n",
                   dateStr, loc->name, ha, sa, al, tz, ch, ss, t3);
        }

        next_date(&y, &m, &d);
    }
    return 0;
}
