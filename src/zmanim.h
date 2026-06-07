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
 */
typedef struct hc_zmanim_s {
    double alot_hashachar;    /*!< Dawn: sun 16.9° below horizon */
    double misheyakir;        /*!< Sun 10.2° below horizon */
    double hanetz;            /*!< Visible sunrise */
    double hanetz_amiti;      /*!< True sunrise: −1.583° */
    double sof_shma;          /*!< Latest Shma: hanetz_amiti + 3 sha'ot */
    double sof_tfila;         /*!< Latest Shacharit: + 4 sha'ot */
    double sof_biur_chometz;  /*!< Burning chometz: + 5 sha'ot */
    double chatzot;           /*!< Halachic noon */
    double mincha_gedola;     /*!< + 6.5 sha'ot */
    double mincha_ketana;     /*!< + 9.5 sha'ot */
    double plag_hamincha;     /*!< + 10.75 sha'ot */
    double shkiah_amitis;     /*!< True sunset: −1.583° */
    double shkiah;            /*!< Visible sunset */
    double tzait_3_stars;     /*!< Nightfall 6° (3 medium stars) */
    double tzait_alter_rebbe; /*!< End of Shabbat 8.5° (Alter Rebbe) */
    double tzait_rt;          /*!< Rabbeinu Tam: 72 min after shkiah */
    double candle_lighting;   /*!< Before sunset (18 or 40 min for Jerusalem) */
    double sha_ah_zmanit_sec; /*!< Length of one sha'ah zmanit in seconds */
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
