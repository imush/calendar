#include "zmanim.h"
#include "noaa.h"
#include "hc_jewish_dates.h"
#include "hc_internal.h"
#include <math.h>
#include <string.h>

hc_cal_impl *get_calendar(hc_calendar_type t);

/* ── Solar angles (degrees, negative = below horizon) ────────────────────── */

#define VISUAL_ANGLE                (-0.8333)
#define TRUE_HORIZON_ANGLE          (-1.583)
#define MISHEYAKIR_ANGLE            (-10.2)
#define MISHEYAKIR_SBH_ANGLE        (-11.5)   /* Sefer Bein Hashmashot */
#define MISHEYAKIR_NIVRESHET_ANGLE  (-11.8)   /* Nivreshet */
#define DAWN_ANGLE                  (-16.9)   /* Chabad Default */
#define DAWN_GRA_ANGLE              (-16.1)   /* GR"A; also MA sha'ah anchor */
#define DAWN_SBH_ANGLE              (-19.8)   /* Sefer Bein Hashmashot */
#define DAWN_RAV_NAEH_ANGLE         (-26.0)   /* Rav Avrohom Chaim Naeh */
#define NIGHTFALL_ANGLE             (-6.0)
#define NIGHTFALL_MELAMED_ANGLE     (-7.083)  /* Melamed Lehoil */
#define HAVDALAH_ANGLE              (-8.5)    /* Alter Rebbe / Igrot Moshe / SBH */

/* ── zmanim computation ───────────────────────────────────────────────────── */

static int is_rest_day(int year, int month, int day, int in_israel)
{
    hc_date gd;
    gd.calendar_type = GREGORIAN;
    gd.year = year; gd.month = month; gd.day = day;
    /* Check Shabbat */
    int dow = (int)(hc_get_day_of_week(&gd)); /* 0=Sat,1=Sun..6=Fri */
    if (dow == 0) return 1; /* Saturday */
    /* Check Yom Tov */
    hc_date hd = gd;
    hc_convert(&hd, HEBREW);
    hc_special_day sdays[HC_MAX_SPECIAL_DAYS];
    int sc = 0;
    hc_get_special_days(&hd, in_israel, sdays, &sc);
    for (int i = 0; i < sc; i++)
        if (hc_sd_is_yom_tov(sdays[i]) && hc_sd_applies(sdays[i], in_israel)) return 1;
    return 0;
}

/* Gregorian date + 1 day */
static void next_day(int *year, int *month, int *day)
{
    (*day)++;
    int ml[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    int y = *year;
    int leap = (y%4==0 && (y%100!=0 || y%400==0));
    int max = (*month == 2 && leap) ? 29 : ml[*month];
    if (*day > max) {
        *day = 1;
        (*month)++;
        if (*month > 12) { *month = 1; (*year)++; }
    }
}

int hc_compute_zmanim(hc_date *date, double lat, double lon,
                      double tz_offset_h, int is_jerusalem, int in_israel,
                      hc_zmanim *out)
{
    if (!date || !out || date->calendar_type != GREGORIAN) return -1;
    memset(out, 0, sizeof(*out));

    int yr = date->year, mo = date->month, dy = date->day;

    /* ── sunrise / sunset at each angle ─────────────────────────────── */
    double hanetz       = noaa_sun_event(yr,mo,dy, lat,lon,tz_offset_h, VISUAL_ANGLE,       1, 0.0);
    double shkiah       = noaa_sun_event(yr,mo,dy, lat,lon,tz_offset_h, VISUAL_ANGLE,       0, 0.0);
    double hanetz_amiti = noaa_sun_event(yr,mo,dy, lat,lon,tz_offset_h, TRUE_HORIZON_ANGLE, 1, 0.0);
    double shkiah_amiti = noaa_sun_event(yr,mo,dy, lat,lon,tz_offset_h, TRUE_HORIZON_ANGLE, 0, 0.0);
    double alot         = noaa_sun_event(yr,mo,dy, lat,lon,tz_offset_h, DAWN_ANGLE,         1, 0.0);
    double alot_rn      = noaa_sun_event(yr,mo,dy, lat,lon,tz_offset_h, DAWN_RAV_NAEH_ANGLE,1, 0.0);
    double alot_sbh     = noaa_sun_event(yr,mo,dy, lat,lon,tz_offset_h, DAWN_SBH_ANGLE,     1, 0.0);
    double alot_gra     = noaa_sun_event(yr,mo,dy, lat,lon,tz_offset_h, DAWN_GRA_ANGLE,     1, 0.0);
    double tzet_gra     = noaa_sun_event(yr,mo,dy, lat,lon,tz_offset_h, DAWN_GRA_ANGLE,     0, 0.0);
    double misheyakir   = noaa_sun_event(yr,mo,dy, lat,lon,tz_offset_h, MISHEYAKIR_ANGLE,   1, 0.0);
    double mish_sbh     = noaa_sun_event(yr,mo,dy, lat,lon,tz_offset_h, MISHEYAKIR_SBH_ANGLE, 1, 0.0);
    double mish_nvs     = noaa_sun_event(yr,mo,dy, lat,lon,tz_offset_h, MISHEYAKIR_NIVRESHET_ANGLE, 1, 0.0);
    double tzait_3      = noaa_sun_event(yr,mo,dy, lat,lon,tz_offset_h, NIGHTFALL_ANGLE,    0, 0.0);
    double tzait_ml     = noaa_sun_event(yr,mo,dy, lat,lon,tz_offset_h, NIGHTFALL_MELAMED_ANGLE, 0, 0.0);
    double tzait_ar     = noaa_sun_event(yr,mo,dy, lat,lon,tz_offset_h, HAVDALAH_ANGLE,     0, 0.0);

    /* Halachic midnight: solar noon + 12h */
    double decl_unused, eqt_unused;
    double noon_min = noaa_solar_noon_and_decl(yr,mo,dy, lon, tz_offset_h, &decl_unused, &eqt_unused);
    double halachic_midnight = noon_min + 720.0; /* +12h (end of today's daytime) */

    out->alot_hashachar        = alot;
    out->alot_rav_naeh         = alot_rn;
    out->alot_sbh              = alot_sbh;
    out->alot_gra              = alot_gra;
    out->misheyakir            = misheyakir;
    out->misheyakir_sbh        = mish_sbh;
    out->misheyakir_nivreshet  = mish_nvs;
    out->hanetz                = hanetz;
    out->hanetz_amiti          = hanetz_amiti;
    out->shkiah_amitis         = shkiah_amiti;
    out->shkiah                = shkiah;
    out->tzait_3_stars         = tzait_3;
    out->tzait_melamed         = tzait_ml;
    out->tzait_alter_rebbe     = tzait_ar;
    out->tzait_rt              = (shkiah != HC_ZMAN_UNAVAILABLE) ? shkiah + 72.0 : HC_ZMAN_UNAVAILABLE;

    /* ── sha'ah zmanit (Chabad default: hanetz amiti → shkiah amitis) ─ */
    double sha_sec = 0;
    if (hanetz_amiti != HC_ZMAN_UNAVAILABLE && shkiah_amiti != HC_ZMAN_UNAVAILABLE) {
        sha_sec = (shkiah_amiti - hanetz_amiti) * 60.0 / 12.0;
    }
    out->sha_ah_zmanit_sec = sha_sec;

    /* ── sha'ah-based zmanim (Chabad default) ────────────────────────── */
    if (sha_sec > 0 && hanetz_amiti != HC_ZMAN_UNAVAILABLE) {
        double ha_min = sha_sec / 60.0;
        out->sof_shma          = hanetz_amiti + 3.0   * ha_min;
        out->sof_tfila         = hanetz_amiti + 4.0   * ha_min;
        out->sof_biur_chometz  = hanetz_amiti + 5.0   * ha_min;
        out->chatzot           = hanetz_amiti + 6.0   * ha_min;
        out->mincha_gedola     = hanetz_amiti + 6.5   * ha_min;
        out->mincha_ketana     = hanetz_amiti + 9.5   * ha_min;
        out->plag_hamincha     = hanetz_amiti + 10.75 * ha_min;
    } else {
        out->sof_shma          = HC_ZMAN_UNAVAILABLE;
        out->sof_tfila         = HC_ZMAN_UNAVAILABLE;
        out->sof_biur_chometz  = HC_ZMAN_UNAVAILABLE;
        out->chatzot           = noon_min;
        out->mincha_gedola     = HC_ZMAN_UNAVAILABLE;
        out->mincha_ketana     = HC_ZMAN_UNAVAILABLE;
        out->plag_hamincha     = HC_ZMAN_UNAVAILABLE;
    }

    /* ── GR"A portion-of-day (visible sunrise → visible sunset) ──────── */
    double sha_sec_gra = 0;
    if (hanetz != HC_ZMAN_UNAVAILABLE && shkiah != HC_ZMAN_UNAVAILABLE) {
        sha_sec_gra = (shkiah - hanetz) * 60.0 / 12.0;
    }
    out->sha_ah_zmanit_sec_gra = sha_sec_gra;
    if (sha_sec_gra > 0 && hanetz != HC_ZMAN_UNAVAILABLE) {
        double ha_min = sha_sec_gra / 60.0;
        out->sof_shma_gra         = hanetz + 3.0   * ha_min;
        out->sof_tfila_gra        = hanetz + 4.0   * ha_min;
        out->sof_biur_chometz_gra = hanetz + 5.0   * ha_min;
        out->mincha_gedola_gra    = hanetz + 6.5   * ha_min;
        out->mincha_ketana_gra    = hanetz + 9.5   * ha_min;
        out->plag_hamincha_gra    = hanetz + 10.75 * ha_min;
    } else {
        out->sof_shma_gra         = HC_ZMAN_UNAVAILABLE;
        out->sof_tfila_gra        = HC_ZMAN_UNAVAILABLE;
        out->sof_biur_chometz_gra = HC_ZMAN_UNAVAILABLE;
        out->mincha_gedola_gra    = HC_ZMAN_UNAVAILABLE;
        out->mincha_ketana_gra    = HC_ZMAN_UNAVAILABLE;
        out->plag_hamincha_gra    = HC_ZMAN_UNAVAILABLE;
    }

    /* ── Magen Avraham portion-of-day (Alot −16.1° → Tzet −16.1°) ───── */
    /* Polar fallback: when Alot −16.1° is unreachable at this latitude/
       date, treat the halachic day as full night-to-night (chatzot
       halaila → next chatzot halaila) → sha'ah = 2h, anchor at chatzot
       halaila at start of today's daytime (halachic_midnight − 24h). */
    double sha_sec_ma;
    double ma_anchor;
    int    ma_fallback = 0;
    if (alot_gra != HC_ZMAN_UNAVAILABLE && tzet_gra != HC_ZMAN_UNAVAILABLE) {
        sha_sec_ma = (tzet_gra - alot_gra) * 60.0 / 12.0;
        ma_anchor  = alot_gra;
    } else {
        sha_sec_ma = 24.0 * 3600.0 / 12.0; /* 2 hours */
        ma_anchor  = halachic_midnight - 1440.0; /* start-of-today's chatzot halaila */
        ma_fallback = 1;
    }
    out->sha_ah_zmanit_sec_ma = sha_sec_ma;
    out->ma_polar_fallback    = ma_fallback;
    {
        double ha_min = sha_sec_ma / 60.0;
        out->sof_shma_ma         = ma_anchor + 3.0   * ha_min;
        out->sof_tfila_ma        = ma_anchor + 4.0   * ha_min;
        out->sof_biur_chometz_ma = ma_anchor + 5.0   * ha_min;
        out->mincha_gedola_ma    = ma_anchor + 6.5   * ha_min;
        out->mincha_ketana_ma    = ma_anchor + 9.5   * ha_min;
        out->plag_hamincha_ma    = ma_anchor + 10.75 * ha_min;
    }

    /* ── candle lighting ─────────────────────────────────────────────── */
    int tom_yr = yr, tom_mo = mo, tom_dy = dy;
    next_day(&tom_yr, &tom_mo, &tom_dy);
    if (is_rest_day(tom_yr, tom_mo, tom_dy, in_israel)) {
        int today_is_rest = is_rest_day(yr, mo, dy, in_israel);
        /* hc_get_day_of_week returns 0=Sat,1=Sun..6=Fri */
        int dow_today = (int)hc_get_day_of_week(date);
        int tomorrow_is_shabbat = (dow_today == 6); /* today=Fri(6) → tomorrow=Sat */

        if (today_is_rest && !tomorrow_is_shabbat) {
            /* Candles after nightfall (from existing flame into Yom Tov) —
               Alter Rebbe / Igrot Moshe tzait (−8.5°, three small stars),
               matches End of Shabbat / YT. Was tzait_3 (−6°) before. */
            out->candle_lighting = (tzait_ar != HC_ZMAN_UNAVAILABLE) ? tzait_ar : halachic_midnight;
        } else {
            int cl_min = is_jerusalem ? 40 : 18;
            if (shkiah != HC_ZMAN_UNAVAILABLE)
                out->candle_lighting = shkiah - cl_min;
            else
                out->candle_lighting = halachic_midnight - cl_min;
        }
    } else {
        out->candle_lighting = HC_ZMAN_UNAVAILABLE;
    }

    return 0;
}
