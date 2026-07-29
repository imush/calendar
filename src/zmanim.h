/*!
 * \file zmanim.h
 * \brief Halachic time (zmanim) calculations.
 *
 * All zmanim follow Chabad / Alter Rebbe standards (sha'ah zmanit based on
 * hanetz amiti to shkiah amitis, i.e. true sunrise/sunset at −1.583°).
 *
 * Times are returned as minutes from local midnight (double).
 * A value of HC_ZMAN_UNAVAILABLE indicates a polar condition where the
 * sun does not cross the requested angle.
 */

#ifndef SRC_ZMANIM_H_
#define SRC_ZMANIM_H_

#include "hconverter.h"

#define HC_ZMAN_UNAVAILABLE (-1e300)

/*!
 * All zmanim for one day and location.
 * Times are minutes from local midnight (00:00 local standard time).
 * Candle lighting is set only when applicable (next day is Shabbat/YomTov);
 * otherwise HC_ZMAN_UNAVAILABLE.
 *
 * Portion-of-day zmanim are provided for all three common opinions —
 * the default fields (sof_shma, sof_tfila, mincha_gedola, mincha_ketana,
 * plag_hamincha, sof_biur_chometz) follow Chabad / Alter Rebbe (hanetz
 * amiti → shkiah amitis); the *_gra and *_ma variants use GR"A (visible
 * sunrise → visible sunset) and Magen Avraham (Alot −16.1° → Tzeit −16.1°)
 * respectively.
 *
 * In high-latitude summer where MA's Alot at −16.1° is unreachable,
 * ma_polar_fallback is set to 1 and MA zmanim are computed from Chatzot
 * HaLailah with a 2-hour sha'ah (the full night-to-night span / 12).
 */
typedef struct hc_zmanim_s {
    double alot_hashachar;    /*!< Dawn: sun 16.9° below horizon (Chabad Default) */
    double misheyakir;        /*!< Sun 10.2° below horizon (Chabad / Zmanei Halacha) */
    double hanetz;            /*!< Visible sunrise */
    double hanetz_amiti;      /*!< True sunrise: −1.583° */
    double sof_shma;          /*!< Latest Shma: hanetz_amiti + 3 sha'ot (Chabad) */
    double sof_tfila;         /*!< Latest Shacharit: + 4 sha'ot (Chabad) */
    double sof_biur_chometz;  /*!< Burning chometz: + 5 sha'ot (Chabad) */
    double chatzot;           /*!< Halachic noon */
    double mincha_gedola;     /*!< + 6.5 sha'ot (Chabad) */
    double mincha_ketana;     /*!< + 9.5 sha'ot (Chabad) */
    double plag_hamincha;     /*!< + 10.75 sha'ot (Chabad) */
    double shkiah_amitis;     /*!< True sunset: −1.583° */
    double shkiah;            /*!< Visible sunset */
    double tzait_3_stars;     /*!< Nightfall 6° (3 medium stars) */
    double tzait_alter_rebbe; /*!< End of Shabbat 8.5° (Alter Rebbe / Igrot Moshe / Sefer Bein Hashmashot) */
    double tzait_rt;          /*!< Rabbeinu Tam: 72 min after shkiah */
    double candle_lighting;   /*!< Before sunset (18 or 40 min for Jerusalem) */
    double sha_ah_zmanit_sec; /*!< Length of one sha'ah zmanit in seconds (Chabad) */

    /* Additional Alot / Misheyakir opinions */
    double alot_rav_naeh;         /*!< Alot per Rav Avrohom Chaim Naeh: −26° */
    double alot_sbh;              /*!< Alot per Sefer Bein Hashmashot: −19.8° */
    double alot_gra;              /*!< Alot per GR"A: −16.1° (also anchors MA sha'ah) */
    double misheyakir_sbh;        /*!< Misheyakir per Sefer Bein Hashmashot: −11.5° */
    double misheyakir_nivreshet;  /*!< Misheyakir per Nivreshet: −11.8° */

    /* Additional Tzeit opinions */
    double tzait_melamed;         /*!< Tzeit per Melamed Lehoil: −7.083° */

    /* GR"A portion-of-day zmanim (visible sunrise → visible sunset) */
    double sha_ah_zmanit_sec_gra;  /*!< Length of one sha'ah zmanit in seconds (GR"A) */
    double sof_shma_gra;           /*!< Latest Shma: hanetz + 3 sha'ot (GR"A) */
    double sof_tfila_gra;          /*!< Latest Shacharit: + 4 sha'ot (GR"A) */
    double sof_biur_chometz_gra;   /*!< Burning chometz: + 5 sha'ot (GR"A) */
    double mincha_gedola_gra;      /*!< + 6.5 sha'ot (GR"A) */
    double mincha_ketana_gra;      /*!< + 9.5 sha'ot (GR"A) */
    double plag_hamincha_gra;      /*!< + 10.75 sha'ot (GR"A) */

    /* Magen Avraham portion-of-day zmanim (Alot −16.1° → Tzeit −16.1°) */
    double sha_ah_zmanit_sec_ma;   /*!< Length of one sha'ah zmanit in seconds (Magen Avraham) */
    double sof_shma_ma;            /*!< Latest Shma: alot_gra + 3 sha'ot (Magen Avraham) */
    double sof_tfila_ma;           /*!< Latest Shacharit: + 4 sha'ot (Magen Avraham) */
    double sof_biur_chometz_ma;    /*!< Burning chometz: + 5 sha'ot (Magen Avraham) */
    double mincha_gedola_ma;       /*!< + 6.5 sha'ot (Magen Avraham) */
    double mincha_ketana_ma;       /*!< + 9.5 sha'ot (Magen Avraham) */
    double plag_hamincha_ma;       /*!< + 10.75 sha'ot (Magen Avraham) */
    int    ma_polar_fallback;      /*!< 1 if MA falls back to Chatzot-HaLailah 2h sha'ah (Alot −16.1° unreachable) */
} hc_zmanim;

/*!
 * \brief Compute all zmanim for a Gregorian date and geographic location.
 *
 * \param[in]  date         Gregorian date (calendar_type must be GREGORIAN)
 * \param[in]  lat          latitude in decimal degrees (north positive)
 * \param[in]  lon          longitude in decimal degrees (east positive)
 * \param[in]  tz_offset_h  UTC offset in hours (e.g. -5.0 for EST, +2.0 for IST)
 * \param[in]  is_jerusalem 1 to apply Jerusalem 40-minute candle lighting rule
 * \param[in]  in_israel    used to determine candle-lighting eligibility via hc_jewish_dates
 * \param[out] out          populated hc_zmanim struct
 * \return 0 on success, -1 on invalid input
 */
int hc_compute_zmanim(hc_date *date, double lat, double lon,
                      double tz_offset_h, int is_jerusalem, int in_israel,
                      hc_zmanim *out);

#endif /* SRC_ZMANIM_H_ */
