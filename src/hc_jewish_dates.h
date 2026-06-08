/*!
 * \file hc_jewish_dates.h
 * \brief Jewish special days (holidays, fasts, Chabad days, etc.)
 */

#ifndef SRC_HC_JEWISH_DATES_H_
#define SRC_HC_JEWISH_DATES_H_

#include "hconverter.h"

/*!
 * All recognised special days.  Days ending in _I are Israel-only;
 * those ending in _C are Chutz LaAretz (Diaspora) only.
 * HC_SD_NONE means the date has no special designation.
 */
typedef enum hc_special_day {
    HC_SD_NONE = 0,

    /* Nisan */
    HC_SD_NISAN_11,
    HC_SD_EREV_PESACH,
    HC_SD_PESACH_1,
    HC_SD_PESACH_2_C,
    HC_SD_PESACH_7,
    HC_SD_PESACH_LAST_C,
    HC_SD_CHOL_HAMOED_PESACH_1_I,
    HC_SD_CHOL_HAMOED_PESACH_2_I,
    HC_SD_CHOL_HAMOED_PESACH_3_I,
    HC_SD_CHOL_HAMOED_PESACH_4_I,
    HC_SD_CHOL_HAMOED_PESACH_5_I,
    HC_SD_CHOL_HAMOED_PESACH_1_C,
    HC_SD_CHOL_HAMOED_PESACH_2_C,
    HC_SD_CHOL_HAMOED_PESACH_3_C,
    HC_SD_CHOL_HAMOED_PESACH_4_C,
    HC_SD_PESACH_SHENI,

    /* Iyar / Sivan */
    HC_SD_LAG_BAOMER,
    HC_SD_SHAVUOT,
    HC_SD_SHAVUOT_2_C,

    /* Tamuz / Av */
    HC_SD_TAMUZ_3,
    HC_SD_TAMUZ_12,
    HC_SD_TAMUZ_13,
    HC_SD_FAST_17_TAMUZ,
    HC_SD_FAST_9_AV,

    /* Elul */
    HC_SD_CHAI_ELUL,

    /* Tishrei */
    HC_SD_ROSH_HASHANA_1,
    HC_SD_ROSH_HASHANA_2,
    HC_SD_ROSH_CHODESH,          /* any Rosh Chodesh except Rosh Hashana */
    HC_SD_TZOM_GEDALIA,
    HC_SD_EREV_YOM_KIPPUR,
    HC_SD_YOM_KIPPUR,
    HC_SD_SUKKOT_1,
    HC_SD_SUKKOT_2_C,
    HC_SD_CHOL_HAMOED_SUKKOT_1_I,
    HC_SD_CHOL_HAMOED_SUKKOT_2_I,
    HC_SD_CHOL_HAMOED_SUKKOT_3_I,
    HC_SD_CHOL_HAMOED_SUKKOT_4_I,
    HC_SD_CHOL_HAMOED_SUKKOT_5_I,
    HC_SD_CHOL_HAMOED_SUKKOT_1_C,
    HC_SD_CHOL_HAMOED_SUKKOT_2_C,
    HC_SD_CHOL_HAMOED_SUKKOT_3_C,
    HC_SD_CHOL_HAMOED_SUKKOT_4_C,
    HC_SD_HOSHANA_RABBA,
    HC_SD_SHMINI_ATZERET_C,
    HC_SD_SIMCHAT_TORAH_C,
    HC_SD_SIMCHAT_TORAH_I,       /* = Shmini Atzeret in Israel */

    /* Kislev / Tevet */
    HC_SD_NINETEENTH_KISLEV,
    HC_SD_CHANUKAH_1,
    HC_SD_CHANUKAH_2,
    HC_SD_CHANUKAH_3,
    HC_SD_CHANUKAH_4,
    HC_SD_CHANUKAH_5,
    HC_SD_CHANUKAH_6,
    HC_SD_CHANUKAH_7,
    HC_SD_CHANUKAH_8,
    HC_SD_TENTH_TEVET,

    /* Shvat */
    HC_SD_YUD_SHVAT,
    HC_SD_TU_BESHVAT,

    /* Adar / Adar II */
    HC_SD_PURIM_KATAN,
    HC_SD_TAANIT_ESTHER,
    HC_SD_PURIM,
    HC_SD_SHUSHAN_PURIM,

    /* Arba Parshiyot */
    HC_SD_SHABBAT_SHEKALIM,
    HC_SD_SHABBAT_ZACHOR,
    HC_SD_SHABBAT_PARA,
    HC_SD_SHABBAT_HACHODESH,

    /* Other Shabbatot */
    HC_SD_SHABBAT_HAGADOL,
    HC_SD_SHABBAT_CHAZON,
    HC_SD_SHABBAT_MEVARCHIM,  /* Shabbat before Rosh Chodesh (not Tishrei) */

    /* Eruv Tavshilin */
    HC_SD_ERUV_TAVSHILIN_I,
    HC_SD_ERUV_TAVSHILIN_C,

    /* Tal UMatar */
    HC_SD_TAL_UMATAR_I,
    HC_SD_TAL_UMATAR_C,

    /* Birkat Hachama */
    HC_SD_BIRKAT_HACHAMA,

    HC_SD_COUNT
} hc_special_day;

/*! Maximum number of special days that can apply to one date */
#define HC_MAX_SPECIAL_DAYS 4

/*!
 * \brief Get special days for a given date.
 *
 * \param[in]  date       any calendar type; converted internally to Hebrew
 * \param[in]  in_israel  1 for Eretz Israel, 0 for Diaspora
 * \param[out] days       array of at least HC_MAX_SPECIAL_DAYS elements, filled
 *                        with applicable special days; remaining entries set to HC_SD_NONE
 * \param[out] count      number of special days found (may be 0)
 * \return 0 on success, -1 on invalid input
 */
int hc_get_special_days(hc_date *date, int in_israel,
                        hc_special_day days[HC_MAX_SPECIAL_DAYS], int *count);

/* Classification predicates */
int hc_sd_is_yom_tov      (hc_special_day d);
int hc_sd_is_chol_hamoed  (hc_special_day d);
int hc_sd_is_fast         (hc_special_day d);
int hc_sd_is_chabad       (hc_special_day d);
int hc_sd_is_chanukah     (hc_special_day d);
int hc_sd_is_arba_parshiyot(hc_special_day d);
int hc_sd_is_eruv_tavshilin(hc_special_day d);
int hc_sd_is_rosh_chodesh  (hc_special_day d);
int hc_sd_is_shabbat_mevarchim(hc_special_day d);

/*! Returns 1 if the special day applies in the given location */
int hc_sd_applies(hc_special_day d, int in_israel);

/*! English name of a special day (NULL for HC_SD_NONE) */
const char *hc_sd_name(hc_special_day d);

#endif /* SRC_HC_JEWISH_DATES_H_ */
