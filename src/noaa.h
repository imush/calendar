/*!
 * \file noaa.h
 * \brief Solar position calculations using the NOAA algorithm.
 *
 * Provides Julian Day conversion and sunrise/sunset event computation.
 * Times are minutes from local midnight (local standard time).
 * HC_ZMAN_UNAVAILABLE is returned for polar conditions.
 */

#ifndef SRC_NOAA_H_
#define SRC_NOAA_H_

#include "zmanim.h"

/*!
 * \brief Julian Day Number for a Gregorian date (noon UT).
 */
double noaa_julian_day(int year, int month, int day);

/*!
 * \brief Compute solar noon and fill declination / equation-of-time.
 *
 * \param[in]  year, month, day  Gregorian date
 * \param[in]  lon               longitude (degrees, east positive)
 * \param[in]  tz_offset_h       UTC offset in hours
 * \param[out] decl_rad          solar declination (radians)
 * \param[out] eqt_min           equation of time (minutes)
 * \return solar noon in minutes from local midnight
 */
double noaa_solar_noon_and_decl(int year, int month, int day,
                                double lon, double tz_offset_h,
                                double *decl_rad, double *eqt_min);

/*!
 * \brief Hour angle (degrees) for a given elevation above/below horizon.
 *
 * \param lat_rad   latitude (radians)
 * \param decl_rad  solar declination (radians)
 * \param elev_deg  target elevation (degrees; negative = below horizon)
 * \return hour angle in degrees, or HC_ZMAN_UNAVAILABLE if unreachable
 */
double noaa_hour_angle_for_elevation(double lat_rad, double decl_rad, double elev_deg);

/*!
 * \brief Time (minutes from local midnight) when the sun crosses elev_deg.
 *
 * An observer above sea level sees a depressed horizon; the depression
 * \f$\delta = \sqrt{2h/R_\oplus}\f$ (radians) is subtracted from \p elev_deg
 * so that all event times are shifted accordingly (sunrise earlier, sunset later).
 * Pass \p observer_elev_m = 0 to disable the correction.
 *
 * \param year, month, day  Gregorian date
 * \param lat               latitude (degrees, north positive)
 * \param lon               longitude (degrees, east positive)
 * \param tz_offset_h       UTC offset in hours
 * \param elev_deg          target elevation (degrees; use negative for below-horizon)
 * \param is_rise           1 for sunrise side, 0 for sunset side
 * \param observer_elev_m   observer elevation above sea level in metres (≥ 0)
 * \return minutes from local midnight, or HC_ZMAN_UNAVAILABLE
 */
double noaa_sun_event(int year, int month, int day,
                      double lat, double lon, double tz_offset_h,
                      double elev_deg, int is_rise, double observer_elev_m);

#endif /* SRC_NOAA_H_ */
