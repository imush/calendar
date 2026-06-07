#include "hc_jewish_dates.h"
#include "hc_internal.h"
#include "tekufot.h"
#include <string.h>

/* from hconverter.c / hebrew.c */
hc_cal_impl *get_calendar(hc_calendar_type t);
heb_year_type hc_get_heb_year_type(int year);

/* ── name table ──────────────────────────────────────────────────────────── */

static const char *SD_NAMES[HC_SD_COUNT] = {
    [HC_SD_NONE]                    = "",
    [HC_SD_NISAN_11]                = "11 Nisan",
    [HC_SD_EREV_PESACH]             = "Erev Pesach",
    [HC_SD_PESACH_1]                = "Pesach",
    [HC_SD_PESACH_2_C]              = "2nd day Pesach",
    [HC_SD_PESACH_7]                = "7th day Pesach",
    [HC_SD_PESACH_LAST_C]           = "Last day Pesach",
    [HC_SD_CHOL_HAMOED_PESACH_1_I]  = "Chol Hamoed Pesach",
    [HC_SD_CHOL_HAMOED_PESACH_2_I]  = "Chol Hamoed Pesach",
    [HC_SD_CHOL_HAMOED_PESACH_3_I]  = "Chol Hamoed Pesach",
    [HC_SD_CHOL_HAMOED_PESACH_4_I]  = "Chol Hamoed Pesach",
    [HC_SD_CHOL_HAMOED_PESACH_5_I]  = "Chol Hamoed Pesach",
    [HC_SD_CHOL_HAMOED_PESACH_1_C]  = "Chol Hamoed Pesach",
    [HC_SD_CHOL_HAMOED_PESACH_2_C]  = "Chol Hamoed Pesach",
    [HC_SD_CHOL_HAMOED_PESACH_3_C]  = "Chol Hamoed Pesach",
    [HC_SD_CHOL_HAMOED_PESACH_4_C]  = "Chol Hamoed Pesach",
    [HC_SD_PESACH_SHENI]            = "Pesach Sheni",
    [HC_SD_LAG_BAOMER]              = "Lag Baomer",
    [HC_SD_SHAVUOT]                 = "Shavuot",
    [HC_SD_SHAVUOT_2_C]             = "2nd day Shavuot",
    [HC_SD_TAMUZ_3]                 = "3 Tamuz",
    [HC_SD_TAMUZ_12]                = "12 Tamuz",
    [HC_SD_TAMUZ_13]                = "13 Tamuz",
    [HC_SD_FAST_17_TAMUZ]           = "Fast of 17 Tamuz",
    [HC_SD_FAST_9_AV]               = "Fast of 9 Av",
    [HC_SD_CHAI_ELUL]               = "Chai Elul",
    [HC_SD_ROSH_HASHANA_1]          = "Rosh Hashana",
    [HC_SD_ROSH_HASHANA_2]          = "2nd day Rosh Hashana",
    [HC_SD_ROSH_CHODESH]            = "Rosh Chodesh",
    [HC_SD_TZOM_GEDALIA]            = "Tzom Gedalia",
    [HC_SD_EREV_YOM_KIPPUR]         = "Erev Yom Kippur",
    [HC_SD_YOM_KIPPUR]              = "Yom Kippur",
    [HC_SD_SUKKOT_1]                = "1st day Sukkot",
    [HC_SD_SUKKOT_2_C]              = "2nd day Sukkot",
    [HC_SD_CHOL_HAMOED_SUKKOT_1_I]  = "Chol Hamoed Sukkot",
    [HC_SD_CHOL_HAMOED_SUKKOT_2_I]  = "Chol Hamoed Sukkot",
    [HC_SD_CHOL_HAMOED_SUKKOT_3_I]  = "Chol Hamoed Sukkot",
    [HC_SD_CHOL_HAMOED_SUKKOT_4_I]  = "Chol Hamoed Sukkot",
    [HC_SD_CHOL_HAMOED_SUKKOT_5_I]  = "Chol Hamoed Sukkot",
    [HC_SD_CHOL_HAMOED_SUKKOT_1_C]  = "Chol Hamoed Sukkot",
    [HC_SD_CHOL_HAMOED_SUKKOT_2_C]  = "Chol Hamoed Sukkot",
    [HC_SD_CHOL_HAMOED_SUKKOT_3_C]  = "Chol Hamoed Sukkot",
    [HC_SD_CHOL_HAMOED_SUKKOT_4_C]  = "Chol Hamoed Sukkot",
    [HC_SD_HOSHANA_RABBA]           = "Hoshana Rabba",
    [HC_SD_SHMINI_ATZERET_C]        = "Shemini Atzeret",
    [HC_SD_SIMCHAT_TORAH_C]         = "Simchat Torah",
    [HC_SD_SIMCHAT_TORAH_I]         = "Simchat Torah",
    [HC_SD_NINETEENTH_KISLEV]       = "19 Kislev",
    [HC_SD_CHANUKAH_1]              = "1st day Chanukah",
    [HC_SD_CHANUKAH_2]              = "2nd day Chanukah",
    [HC_SD_CHANUKAH_3]              = "3rd day Chanukah",
    [HC_SD_CHANUKAH_4]              = "4th day Chanukah",
    [HC_SD_CHANUKAH_5]              = "5th day Chanukah",
    [HC_SD_CHANUKAH_6]              = "6th day Chanukah",
    [HC_SD_CHANUKAH_7]              = "7th day Chanukah",
    [HC_SD_CHANUKAH_8]              = "8th day Chanukah",
    [HC_SD_TENTH_TEVET]             = "10 Tevet",
    [HC_SD_YUD_SHVAT]               = "10 Shvat",
    [HC_SD_TU_BESHVAT]              = "Tu Bishvat",
    [HC_SD_PURIM_KATAN]             = "Purim Katan",
    [HC_SD_TAANIT_ESTHER]           = "Fast of Esther",
    [HC_SD_PURIM]                   = "Purim",
    [HC_SD_SHUSHAN_PURIM]           = "Shushan Purim",
    [HC_SD_SHABBAT_SHEKALIM]        = "Shabbat Shekalim",
    [HC_SD_SHABBAT_ZACHOR]          = "Shabbat Zachor",
    [HC_SD_SHABBAT_PARA]            = "Shabbat Para",
    [HC_SD_SHABBAT_HACHODESH]       = "Shabbat Hachodesh",
    [HC_SD_SHABBAT_HAGADOL]         = "Shabbat Hagadol",
    [HC_SD_SHABBAT_CHAZON]          = "Shabbat Chazon",
    [HC_SD_ERUV_TAVSHILIN_I]        = "Eruv Tavshilin",
    [HC_SD_ERUV_TAVSHILIN_C]        = "Eruv Tavshilin",
    [HC_SD_TAL_UMATAR_I]            = "Tal Umatar",
    [HC_SD_TAL_UMATAR_C]            = "Tal Umatar",
    [HC_SD_BIRKAT_HACHAMA]          = "Birkat HaChama",
};

const char *hc_sd_name(hc_special_day d)
{
    if (d <= HC_SD_NONE || d >= HC_SD_COUNT) return NULL;
    return SD_NAMES[d];
}

/* ── classification tables ───────────────────────────────────────────────── */

static const hc_special_day YOM_TOV_DAYS[] = {
    HC_SD_PESACH_1, HC_SD_PESACH_2_C, HC_SD_PESACH_7, HC_SD_PESACH_LAST_C,
    HC_SD_SHAVUOT, HC_SD_SHAVUOT_2_C,
    HC_SD_ROSH_HASHANA_1, HC_SD_ROSH_HASHANA_2, HC_SD_YOM_KIPPUR,
    HC_SD_SUKKOT_1, HC_SD_SUKKOT_2_C,
    HC_SD_SIMCHAT_TORAH_I, HC_SD_SIMCHAT_TORAH_C, HC_SD_SHMINI_ATZERET_C,
    HC_SD_NONE
};
static const hc_special_day CHOL_HAMOED_DAYS[] = {
    HC_SD_CHOL_HAMOED_PESACH_1_I, HC_SD_CHOL_HAMOED_PESACH_2_I,
    HC_SD_CHOL_HAMOED_PESACH_3_I, HC_SD_CHOL_HAMOED_PESACH_4_I,
    HC_SD_CHOL_HAMOED_PESACH_5_I,
    HC_SD_CHOL_HAMOED_PESACH_1_C, HC_SD_CHOL_HAMOED_PESACH_2_C,
    HC_SD_CHOL_HAMOED_PESACH_3_C, HC_SD_CHOL_HAMOED_PESACH_4_C,
    HC_SD_CHOL_HAMOED_SUKKOT_1_I, HC_SD_CHOL_HAMOED_SUKKOT_2_I,
    HC_SD_CHOL_HAMOED_SUKKOT_3_I, HC_SD_CHOL_HAMOED_SUKKOT_4_I,
    HC_SD_CHOL_HAMOED_SUKKOT_5_I, HC_SD_HOSHANA_RABBA,
    HC_SD_CHOL_HAMOED_SUKKOT_1_C, HC_SD_CHOL_HAMOED_SUKKOT_2_C,
    HC_SD_CHOL_HAMOED_SUKKOT_3_C, HC_SD_CHOL_HAMOED_SUKKOT_4_C,
    HC_SD_NONE
};
static const hc_special_day FAST_DAYS[] = {
    HC_SD_TAANIT_ESTHER, HC_SD_TZOM_GEDALIA,
    HC_SD_TENTH_TEVET, HC_SD_FAST_17_TAMUZ, HC_SD_FAST_9_AV,
    HC_SD_NONE
};
static const hc_special_day CHABAD_DAYS[] = {
    HC_SD_NINETEENTH_KISLEV, HC_SD_YUD_SHVAT, HC_SD_NISAN_11,
    HC_SD_TAMUZ_3, HC_SD_TAMUZ_12, HC_SD_TAMUZ_13, HC_SD_CHAI_ELUL,
    HC_SD_NONE
};
static const hc_special_day CHANUKAH_DAYS[] = {
    HC_SD_CHANUKAH_1, HC_SD_CHANUKAH_2, HC_SD_CHANUKAH_3, HC_SD_CHANUKAH_4,
    HC_SD_CHANUKAH_5, HC_SD_CHANUKAH_6, HC_SD_CHANUKAH_7, HC_SD_CHANUKAH_8,
    HC_SD_NONE
};
static const hc_special_day ARBA_PARSHIYOT_DAYS[] = {
    HC_SD_SHABBAT_SHEKALIM, HC_SD_SHABBAT_ZACHOR,
    HC_SD_SHABBAT_PARA, HC_SD_SHABBAT_HACHODESH,
    HC_SD_NONE
};
static const hc_special_day ERUV_TAVSHILIN_DAYS[] = {
    HC_SD_ERUV_TAVSHILIN_I, HC_SD_ERUV_TAVSHILIN_C, HC_SD_NONE
};
static const hc_special_day ROSH_CHODESH_DAYS[] = {
    HC_SD_ROSH_CHODESH, HC_SD_NONE
};

/* Israel-only days (do not apply in Diaspora) */
static const hc_special_day ISRAEL_ONLY[] = {
    HC_SD_CHOL_HAMOED_PESACH_1_I, HC_SD_CHOL_HAMOED_PESACH_2_I,
    HC_SD_CHOL_HAMOED_PESACH_3_I, HC_SD_CHOL_HAMOED_PESACH_4_I,
    HC_SD_CHOL_HAMOED_PESACH_5_I,
    HC_SD_CHOL_HAMOED_SUKKOT_1_I, HC_SD_CHOL_HAMOED_SUKKOT_2_I,
    HC_SD_CHOL_HAMOED_SUKKOT_3_I, HC_SD_CHOL_HAMOED_SUKKOT_4_I,
    HC_SD_CHOL_HAMOED_SUKKOT_5_I,
    HC_SD_SIMCHAT_TORAH_I, HC_SD_ERUV_TAVSHILIN_I, HC_SD_TAL_UMATAR_I,
    HC_SD_NONE
};
/* Diaspora-only days */
static const hc_special_day DIASPORA_ONLY[] = {
    HC_SD_PESACH_2_C, HC_SD_PESACH_LAST_C,
    HC_SD_CHOL_HAMOED_PESACH_1_C, HC_SD_CHOL_HAMOED_PESACH_2_C,
    HC_SD_CHOL_HAMOED_PESACH_3_C, HC_SD_CHOL_HAMOED_PESACH_4_C,
    HC_SD_SHAVUOT_2_C, HC_SD_SUKKOT_2_C,
    HC_SD_CHOL_HAMOED_SUKKOT_1_C, HC_SD_CHOL_HAMOED_SUKKOT_2_C,
    HC_SD_CHOL_HAMOED_SUKKOT_3_C, HC_SD_CHOL_HAMOED_SUKKOT_4_C,
    HC_SD_SHMINI_ATZERET_C, HC_SD_SIMCHAT_TORAH_C,
    HC_SD_ERUV_TAVSHILIN_C, HC_SD_TAL_UMATAR_C,
    HC_SD_NONE
};

static int in_list(hc_special_day d, const hc_special_day *list)
{
    for (int i = 0; list[i] != HC_SD_NONE; i++)
        if (list[i] == d) return 1;
    return 0;
}

int hc_sd_is_yom_tov      (hc_special_day d) { return in_list(d, YOM_TOV_DAYS); }
int hc_sd_is_chol_hamoed  (hc_special_day d) { return in_list(d, CHOL_HAMOED_DAYS); }
int hc_sd_is_fast         (hc_special_day d) { return in_list(d, FAST_DAYS); }
int hc_sd_is_chabad       (hc_special_day d) { return in_list(d, CHABAD_DAYS); }
int hc_sd_is_chanukah     (hc_special_day d) { return in_list(d, CHANUKAH_DAYS); }
int hc_sd_is_arba_parshiyot(hc_special_day d){ return in_list(d, ARBA_PARSHIYOT_DAYS); }
int hc_sd_is_eruv_tavshilin(hc_special_day d){ return in_list(d, ERUV_TAVSHILIN_DAYS); }
int hc_sd_is_rosh_chodesh (hc_special_day d) { return in_list(d, ROSH_CHODESH_DAYS); }

int hc_sd_applies(hc_special_day d, int in_israel)
{
    if (in_israel) return !in_list(d, DIASPORA_ONLY);
    else           return !in_list(d, ISRAEL_ONLY);
}

/* ── helper macros ───────────────────────────────────────────────────────── */

/* absolute day helpers */
static long heb_abs(int year, int month, int day)
{
    return get_calendar(HEBREW)->abs_date(year, month, day);
}
static int heb_dow(int year, int month, int day)
{
    /* 1=Sun..7=Sat */
    long a = heb_abs(year, month, day);
    int d = (int)(a % 7);
    return d == 0 ? 7 : d;
}

static void push(hc_special_day days[HC_MAX_SPECIAL_DAYS], int *count, hc_special_day d)
{
    if (*count < HC_MAX_SPECIAL_DAYS)
        days[(*count)++] = d;
}

/* ── special-day matching logic ──────────────────────────────────────────── */

/*
 * Chanukah: 25 Kislev + 7 more days (may spill into Tevet if Kislev is short).
 * Returns 1-8 if this is a Chanukah day, 0 otherwise.
 */
static int chanukah_day(int year, int month, int day)
{
    long abs_this  = heb_abs(year, month, day);
    long abs_start = heb_abs(year, 9, 25); /* Kislev 25 */
    long diff = abs_this - abs_start;
    if (diff >= 0 && diff < 8) return (int)(diff + 1);
    return 0;
}

/*
 * Taanit Esther: 13 Adar (or 13 Adar I if leap), shifted to Thursday when it
 * would fall on Shabbat.
 */
static int is_taanit_esther(int year, int month, int day)
{
    int adar = hc_is_leap_year(year, HEBREW) ? 13 : 12; /* last Adar */
    if (month != adar || (day != 11 && day != 13)) return 0;
    if (day == 13 && heb_dow(year, adar, 13) != 7) return 1; /* not Shabbat */
    if (day == 11 && heb_dow(year, adar, 13) == 7) return 1; /* nidche to Thu */
    return 0;
}

/*
 * Fast of 17 Tamuz: 17 Tamuz, or 18 Tamuz on Sunday if 17 is Shabbat.
 */
static int is_fast_17_tamuz(int year, int month, int day)
{
    if (month != 4) return 0;
    if (day == 17 && heb_dow(year, 4, 17) != 7) return 1;
    if (day == 18 && heb_dow(year, 4, 17) == 7 && heb_dow(year, 4, 18) == 1) return 1;
    return 0;
}

/*
 * Fast of 9 Av: 9 Av, or 10 Av on Sunday if 9 is Shabbat.
 */
static int is_fast_9_av(int year, int month, int day)
{
    if (month != 5) return 0;
    if (day == 9  && heb_dow(year, 5, 9) != 7) return 1;
    if (day == 10 && heb_dow(year, 5, 9) == 7 && heb_dow(year, 5, 10) == 1) return 1;
    return 0;
}

/*
 * Tzom Gedalia: 3 Tishrei (or 4 if 3 is Shabbat).
 */
static int is_tzom_gedalia(int year, int month, int day)
{
    if (month != 7) return 0;
    if (day == 3 && heb_dow(year, 7, 3) != 7) return 1;
    if (day == 4 && heb_dow(year, 7, 3) == 7 && heb_dow(year, 7, 4) == 1) return 1;
    return 0;
}

/*
 * Shabbat Hagadol: last Shabbat before Pesach (Nisan 15).
 */
static int is_shabbat_hagadol(int year, int month, int day)
{
    if (heb_dow(year, month, day) != 7) return 0;
    long abs_this   = heb_abs(year, month, day);
    long abs_pesach = heb_abs(year, 1, 15);
    return abs_pesach - abs_this > 0 && abs_pesach - abs_this <= 7;
}

/*
 * Shabbat Chazon: Shabbat on or before 9 Av.
 */
static int is_shabbat_chazon(int year, int month, int day)
{
    if (heb_dow(year, month, day) != 7) return 0;
    long abs_this = heb_abs(year, month, day);
    long abs_9av  = heb_abs(year, 5, 9);
    return abs_9av - abs_this >= 0 && abs_9av - abs_this < 7;
}

/*
 * Shabbat Shekalim: Shabbat on or before 1 Adar (last Adar in leap year).
 */
static int is_shabbat_shekalim(int year, int month, int day)
{
    if (heb_dow(year, month, day) != 7) return 0;
    int adar = hc_is_leap_year(year, HEBREW) ? 13 : 12; /* last Adar */
    long abs_this = heb_abs(year, month, day);
    long abs_1adar = heb_abs(year, adar, 1);
    return abs_1adar - abs_this >= 0 && abs_1adar - abs_this < 7;
}

/*
 * Shabbat Zachor: Shabbat before Purim (13 Adar / 13 Adar II).
 */
static int is_shabbat_zachor(int year, int month, int day)
{
    if (heb_dow(year, month, day) != 7) return 0;
    int adar_last = hc_is_leap_year(year, HEBREW) ? 13 : 12;
    long abs_this   = heb_abs(year, month, day);
    long abs_purim  = heb_abs(year, adar_last, 13);
    return abs_purim - abs_this >= 0 && abs_purim - abs_this < 7;
}

/*
 * Shabbat Para: 2nd Shabbat before 1 Nisan.
 */
static int is_shabbat_para(int year, int month, int day)
{
    if (heb_dow(year, month, day) != 7) return 0;
    long abs_this  = heb_abs(year, month, day);
    long abs_1nis  = heb_abs(year, 1, 1);
    long diff = abs_1nis - abs_this;
    return diff >= 7 && diff < 14;
}

/*
 * Shabbat Hachodesh: Shabbat on or before 1 Nisan.
 */
static int is_shabbat_hachodesh(int year, int month, int day)
{
    if (heb_dow(year, month, day) != 7) return 0;
    long abs_this  = heb_abs(year, month, day);
    long abs_1nis  = heb_abs(year, 1, 1);
    return abs_1nis - abs_this >= 0 && abs_1nis - abs_this < 7;
}

/*
 * Rosh Chodesh: day 1 of any month (not Tishrei), or day 30 of any month.
 * Not Rosh Hashana.
 */
static int is_rosh_chodesh(int year, int month, int day)
{
    if (month == 7 && (day == 1 || day == 2)) return 0; /* Rosh Hashana */
    if (day == 1) return 1;
    /* 30th day of previous month (only when that month has 30 days) */
    if (day == 30) {
        int ml = get_calendar(HEBREW)->month_length(year, month);
        return ml == 30;
    }
    return 0;
}

/*
 * Tal Umatar start day — exact tekufa calculation matching Java TalUMatar.computeDate().
 *
 * Both Israel and Diaspora: if the computed day is Shabbat, postpone to Sunday.
 * Israel:  7 Cheshvan of the current Hebrew year.
 * Diaspora: 60th day (inclusive) after Tekufat Tishrei per Shmuel.
 *           The tekufa for Hebrew year Y is getTekufatShmuel(Y-1, TISHREI)
 *           (same year-offset convention as the Java library).
 */
static long tal_umatar_israel_abs(int year)
{
    long abs = heb_abs(year, 8, 7);
    if ((int)(abs % 7) == 0) abs++; /* postpone from Shabbat */
    return abs;
}

static long tal_umatar_diaspora_abs(int year)
{
    long tekufa_abs; heb_time t;
    hc_get_tekufa(HC_SHMUEL, year - 1, HC_SEASON_TISHREI, &tekufa_abs, &t);
    long abs = tekufa_abs + 59;
    if ((int)(abs % 7) == 0) abs++; /* postpone from Shabbat */
    return abs;
}

static int is_tal_umatar_israel(int year, int month, int day)
{
    return heb_abs(year, month, day) == tal_umatar_israel_abs(year);
}

static int is_tal_umatar_diaspora(int year, int month, int day)
{
    return heb_abs(year, month, day) == tal_umatar_diaspora_abs(year);
}

/*
 * Eruv Tavshilin: matches Java's EruvTavshilin.matches() exactly.
 * YT day lists mirror Java's ERUV_TAVSHILIN_I / _C arrays.
 */

static int yt_check(int m, int d, int in_israel)
{
    if (in_israel)
        return ((m==1&&d==21)||(m==3&&d==6)||(m==7&&(d==1||d==2)));
    return ((m==1&&(d==15||d==16||d==21||d==22))||
            (m==3&&(d==6||d==7))||
            (m==7&&(d==1||d==2||d==15||d==16||d==22||d==23)));
}

static int is_eruv_tavshilin(int year, int month, int day, int in_israel)
{
    /* Guard: ET cannot be made on Shabbat or Yom Tov */
    if (heb_dow(year, month, day) == 7) return 0;
    if (yt_check(month, day, in_israel)) return 0;

    long abs_tom = heb_abs(year, month, day) + 1;
    int dow_tom = (int)(abs_tom % 7);
    if (dow_tom == 0) dow_tom = 7;

    hc_date tom;
    tom.calendar_type = HEBREW;
    get_calendar(HEBREW)->compute_date(abs_tom, &tom);
    int tm = tom.month, td = tom.day;

    if (!yt_check(tm, td, in_israel)) return 0;

    if (dow_tom == 6) return 1; /* YT on Friday */

    if (dow_tom == 5) { /* YT on Thursday — check if Friday is also YT */
        long abs_dat = abs_tom + 1;
        hc_date dat;
        dat.calendar_type = HEBREW;
        get_calendar(HEBREW)->compute_date(abs_dat, &dat);
        if (yt_check(dat.month, dat.day, in_israel)) return 1;
    }

    return 0;
}

/*
 * Birkat HaChama: when Tekufat Nisan (Shmuel) falls on Wednesday at 18:00.
 * Occurs every 28 years (year % 28 == 1). The actual calendar date is the
 * day on which the tekufa falls, computed from the tekufa engine — matching
 * Java's BirkatHachama.tekufaDateForYear(year).
 */
static int is_birkat_hachama(int year, int month, int day)
{
    if ((year % 28) != 1) return 0;
    long tekufa_abs; heb_time t;
    hc_get_tekufa(HC_SHMUEL, year, HC_SEASON_NISAN, &tekufa_abs, &t);
    return heb_abs(year, month, day) == tekufa_abs;
}

/* ── main function ───────────────────────────────────────────────────────── */

int hc_get_special_days(hc_date *date, int in_israel,
                        hc_special_day days[HC_MAX_SPECIAL_DAYS], int *count)
{
    if (!date || !days || !count) return -1;
    for (int i = 0; i < HC_MAX_SPECIAL_DAYS; i++) days[i] = HC_SD_NONE;
    *count = 0;

    hc_date hd = *date;
    if (hd.calendar_type != HEBREW) {
        if (hc_convert(&hd, HEBREW) != 0) return -1;
    }
    int y = hd.year, m = hd.month, d = hd.day;
    int leap = hc_is_leap_year(y, HEBREW);
    int adar_last = leap ? 13 : 12;
    int dow = heb_dow(y, m, d); /* 1=Sun..7=Sat */

    /* ── Nisan ─────────────────────────────────────────────────────────── */
    if (m == 1) {
        if (d == 11)                                    push(days, count, HC_SD_NISAN_11);
        if (d == 14)                                    push(days, count, HC_SD_EREV_PESACH);
        if (d == 15)                                    push(days, count, HC_SD_PESACH_1);
        if (d == 16 && !in_israel)                      push(days, count, HC_SD_PESACH_2_C);
        if (d == 21)                                    push(days, count, HC_SD_PESACH_7);
        if (d == 22 && !in_israel)                      push(days, count, HC_SD_PESACH_LAST_C);
        /* Chol Hamoed Pesach Israel: 16-20 */
        if (in_israel) {
            if (d == 16) push(days, count, HC_SD_CHOL_HAMOED_PESACH_1_I);
            if (d == 17) push(days, count, HC_SD_CHOL_HAMOED_PESACH_2_I);
            if (d == 18) push(days, count, HC_SD_CHOL_HAMOED_PESACH_3_I);
            if (d == 19) push(days, count, HC_SD_CHOL_HAMOED_PESACH_4_I);
            if (d == 20) push(days, count, HC_SD_CHOL_HAMOED_PESACH_5_I);
        } else {
            if (d == 17) push(days, count, HC_SD_CHOL_HAMOED_PESACH_1_C);
            if (d == 18) push(days, count, HC_SD_CHOL_HAMOED_PESACH_2_C);
            if (d == 19) push(days, count, HC_SD_CHOL_HAMOED_PESACH_3_C);
            if (d == 20) push(days, count, HC_SD_CHOL_HAMOED_PESACH_4_C);
        }
        if (is_birkat_hachama(y, m, d))                push(days, count, HC_SD_BIRKAT_HACHAMA);
    }
    /* ── Iyar ──────────────────────────────────────────────────────────── */
    if (m == 2) {
        if (d == 14)                                    push(days, count, HC_SD_PESACH_SHENI);
        if (d == 18)                                    push(days, count, HC_SD_LAG_BAOMER);
    }
    /* ── Sivan ─────────────────────────────────────────────────────────── */
    if (m == 3) {
        if (d == 6)                                     push(days, count, HC_SD_SHAVUOT);
        if (d == 7 && !in_israel)                       push(days, count, HC_SD_SHAVUOT_2_C);
    }
    /* ── Tamuz ─────────────────────────────────────────────────────────── */
    if (m == 4) {
        if (d == 3)                                     push(days, count, HC_SD_TAMUZ_3);
        if (d == 12)                                    push(days, count, HC_SD_TAMUZ_12);
        if (d == 13)                                    push(days, count, HC_SD_TAMUZ_13);
        if (is_fast_17_tamuz(y, m, d))                  push(days, count, HC_SD_FAST_17_TAMUZ);
    }
    /* ── Av ────────────────────────────────────────────────────────────── */
    if (m == 5) {
        if (is_fast_9_av(y, m, d))                      push(days, count, HC_SD_FAST_9_AV);
    }
    /* ── Elul ──────────────────────────────────────────────────────────── */
    if (m == 6) {
        if (d == 18)                                    push(days, count, HC_SD_CHAI_ELUL);
    }
    /* ── Tishrei ───────────────────────────────────────────────────────── */
    if (m == 7) {
        if (d == 1)                                     push(days, count, HC_SD_ROSH_HASHANA_1);
        if (d == 2)                                     push(days, count, HC_SD_ROSH_HASHANA_2);
        if (is_tzom_gedalia(y, m, d))                   push(days, count, HC_SD_TZOM_GEDALIA);
        if (d == 9)                                     push(days, count, HC_SD_EREV_YOM_KIPPUR);
        if (d == 10)                                    push(days, count, HC_SD_YOM_KIPPUR);
        if (d == 15)                                    push(days, count, HC_SD_SUKKOT_1);
        if (d == 16 && !in_israel)                      push(days, count, HC_SD_SUKKOT_2_C);
        if (in_israel) {
            if (d == 16) push(days, count, HC_SD_CHOL_HAMOED_SUKKOT_1_I);
            if (d == 17) push(days, count, HC_SD_CHOL_HAMOED_SUKKOT_2_I);
            if (d == 18) push(days, count, HC_SD_CHOL_HAMOED_SUKKOT_3_I);
            if (d == 19) push(days, count, HC_SD_CHOL_HAMOED_SUKKOT_4_I);
            if (d == 20) push(days, count, HC_SD_CHOL_HAMOED_SUKKOT_5_I);
        } else {
            if (d == 17) push(days, count, HC_SD_CHOL_HAMOED_SUKKOT_1_C);
            if (d == 18) push(days, count, HC_SD_CHOL_HAMOED_SUKKOT_2_C);
            if (d == 19) push(days, count, HC_SD_CHOL_HAMOED_SUKKOT_3_C);
            if (d == 20) push(days, count, HC_SD_CHOL_HAMOED_SUKKOT_4_C);
        }
        if (d == 21)                                    push(days, count, HC_SD_HOSHANA_RABBA);
        if (d == 22 && !in_israel)                      push(days, count, HC_SD_SHMINI_ATZERET_C);
        if (d == 23 && !in_israel)                      push(days, count, HC_SD_SIMCHAT_TORAH_C);
        if (d == 22 &&  in_israel)                      push(days, count, HC_SD_SIMCHAT_TORAH_I);
    }
    /* ── Tal Umatar ────────────────────────────────────────────────────── */
    if (in_israel && is_tal_umatar_israel(y, m, d))    push(days, count, HC_SD_TAL_UMATAR_I);
    if (!in_israel && is_tal_umatar_diaspora(y, m, d)) push(days, count, HC_SD_TAL_UMATAR_C);

    /* ── Kislev / Tevet ────────────────────────────────────────────────── */
    if (m == 9) {
        if (d == 19)                                    push(days, count, HC_SD_NINETEENTH_KISLEV);
    }
    {
        int ch = chanukah_day(y, m, d);
        if (ch > 0) push(days, count, (hc_special_day)(HC_SD_CHANUKAH_1 + ch - 1));
    }
    if (m == 10 && d == 10)                             push(days, count, HC_SD_TENTH_TEVET);

    /* ── Shvat ─────────────────────────────────────────────────────────── */
    if (m == 11) {
        if (d == 10)                                    push(days, count, HC_SD_YUD_SHVAT);
        if (d == 15)                                    push(days, count, HC_SD_TU_BESHVAT);
    }
    /* ── Adar / Adar II ────────────────────────────────────────────────── */
    if (m == adar_last) {
        if (d == 14)                                    push(days, count, HC_SD_PURIM);
        if (d == 15)                                    push(days, count, HC_SD_SHUSHAN_PURIM);
        if (is_taanit_esther(y, m, d))                  push(days, count, HC_SD_TAANIT_ESTHER);
    }
    /* Purim Katan (14 Adar I in leap year) */
    if (leap && m == 12 && d == 14)                     push(days, count, HC_SD_PURIM_KATAN);

    /* ── Rosh Chodesh (any month) ──────────────────────────────────────── */
    if (is_rosh_chodesh(y, m, d))                       push(days, count, HC_SD_ROSH_CHODESH);

    /* ── Shabbat-based special days ────────────────────────────────────── */
    if (dow == 7) {
        if (is_shabbat_hagadol(y, m, d))               push(days, count, HC_SD_SHABBAT_HAGADOL);
        if (is_shabbat_chazon(y, m, d))                push(days, count, HC_SD_SHABBAT_CHAZON);
        if (is_shabbat_shekalim(y, m, d))              push(days, count, HC_SD_SHABBAT_SHEKALIM);
        if (is_shabbat_zachor(y, m, d))                push(days, count, HC_SD_SHABBAT_ZACHOR);
        if (is_shabbat_para(y, m, d))                  push(days, count, HC_SD_SHABBAT_PARA);
        if (is_shabbat_hachodesh(y, m, d))             push(days, count, HC_SD_SHABBAT_HACHODESH);
    }

    /* ── Eruv Tavshilin ────────────────────────────────────────────────── */
    if (is_eruv_tavshilin(y, m, d, in_israel)) {
        push(days, count, in_israel ? HC_SD_ERUV_TAVSHILIN_I : HC_SD_ERUV_TAVSHILIN_C);
    }

    return 0;
}
