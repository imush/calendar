/*!
 * \file tekufot.h
 * \brief Tekufa (Jewish solar season) calculations.
 *
 * Two opinions are provided:
 *   - HC_RAV_ADA : year = 235/19 lunar months; season = 91d 7h 519p 31r
 *   - HC_SHMUEL  : year = 365.25 Julian days;  season = 91d 7h 540p
 *
 * Units: 1 hour = 1080 chalakim; 1 chelek = 76 regaim.
 * Regaim are non-zero only for Rav Ada.
 */

#ifndef SRC_TEKUFOT_H_
#define SRC_TEKUFOT_H_

#include "hconverter.h"

/*!
 * Which tekufa season is requested.
 *   HC_SEASON_NISAN   = 0  (spring)
 *   HC_SEASON_TAMMUZ  = 1  (summer)
 *   HC_SEASON_TISHREI = 2  (autumn)
 *   HC_SEASON_TEVET   = 3  (winter)
 */
typedef enum hc_season {
    HC_SEASON_NISAN   = 0,
    HC_SEASON_TAMMUZ  = 1,
    HC_SEASON_TISHREI = 2,
    HC_SEASON_TEVET   = 3
} hc_season;

/*!
 * Which halachic opinion to use for the tekufa calculation.
 */
typedef enum hc_tekufa_opinion {
    HC_RAV_ADA,
    HC_SHMUEL
} hc_tekufa_opinion;

/*!
 * \brief Compute the tekufa time for a given Hebrew year and season.
 *
 * The result is an absolute halachic moment expressed as days-since-creation,
 * hours, chalakim, and regaim, stored in a \c heb_time together with an
 * absolute day number returned via \p abs_day_out.
 *
 * \param[in]  opinion      HC_RAV_ADA or HC_SHMUEL
 * \param[in]  hebrew_year  Hebrew year (1 = Shnat Tohu)
 * \param[in]  season       one of the four \c hc_season values
 * \param[out] abs_day_out  absolute day (days since creation) of the tekufa
 * \param[out] time_out     hour/part/rega within that day
 * \return 0 on success, -1 on invalid input
 */
int hc_get_tekufa(hc_tekufa_opinion opinion, int hebrew_year, hc_season season,
                  long *abs_day_out, heb_time *time_out);

#endif /* SRC_TEKUFOT_H_ */
