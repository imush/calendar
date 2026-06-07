#include "noaa.h"
#include <math.h>

#define PI      3.14159265358979323846
#define DEG2RAD (PI / 180.0)
#define RAD2DEG (180.0 / PI)

double noaa_julian_day(int year, int month, int day)
{
    if (month <= 2) { year--; month += 12; }
    int A = year / 100;
    int B = 2 - A + A / 4;
    return (int)(365.25 * (year + 4716)) + (int)(30.6001 * (month + 1)) + day + B - 1524.5;
}

double noaa_solar_noon_and_decl(int year, int month, int day,
                                double lon, double tz_offset_h,
                                double *decl_rad, double *eqt_min)
{
    double JD = noaa_julian_day(year, month, day);
    double JC = (JD - 2451545.0) / 36525.0;

    double L0 = fmod(280.46646 + JC * (36000.76983 + JC * 0.0003032), 360.0);
    double M  = 357.52911 + JC * (35999.05029 - 0.0001537 * JC);
    double Mrad = M * DEG2RAD;
    double C = sin(Mrad) * (1.914602 - JC * (0.004817 + 0.000014 * JC))
             + sin(2*Mrad) * (0.019993 - 0.000101 * JC)
             + sin(3*Mrad) * 0.000289;
    double sun_lon = L0 + C;
    double omega = 125.04 - 1934.136 * JC;
    double lambda = sun_lon - 0.00569 - 0.00478 * sin(omega * DEG2RAD);
    double eps0 = 23.0 + (26.0 + (21.448 - JC * (46.8150 + JC * (0.00059 - JC * 0.001813))) / 60.0) / 60.0;
    double eps = eps0 + 0.00256 * cos(omega * DEG2RAD);
    *decl_rad = asin(sin(eps * DEG2RAD) * sin(lambda * DEG2RAD));

    double y_var = tan(eps/2.0 * DEG2RAD);
    y_var *= y_var;
    double L0rad = L0 * DEG2RAD;
    double Mrad2 = M * DEG2RAD;
    double eqt = 4.0 * RAD2DEG * (
            y_var * sin(2*L0rad)
          - 2.0 * 0.016708634 * sin(Mrad2)
          + 4.0 * 0.016708634 * y_var * sin(Mrad2) * cos(2*L0rad)
          - 0.5 * y_var * y_var * sin(4*L0rad)
          - 1.25 * 0.016708634 * 0.016708634 * sin(2*Mrad2));
    *eqt_min = eqt;

    return 720.0 - 4.0 * lon - eqt + tz_offset_h * 60.0;
}

double noaa_hour_angle_for_elevation(double lat_rad, double decl_rad, double elev_deg)
{
    double elev_rad = elev_deg * DEG2RAD;
    double cos_ha = (sin(elev_rad) - sin(lat_rad) * sin(decl_rad))
                  / (cos(lat_rad) * cos(decl_rad));
    if (cos_ha < -1.0 || cos_ha > 1.0) return HC_ZMAN_UNAVAILABLE;
    return acos(cos_ha) * RAD2DEG;
}

double noaa_sun_event(int year, int month, int day,
                      double lat, double lon, double tz_offset_h,
                      double elev_deg, int is_rise, double observer_elev_m)
{
    double effective_elev = elev_deg;
    if (observer_elev_m > 0.0) {
        /* horizon dip (degrees) due to observer elevation */
        double dip = RAD2DEG * sqrt(2.0 * observer_elev_m / 6371000.0);
        effective_elev -= dip;
    }
    double decl, eqt;
    double noon = noaa_solar_noon_and_decl(year, month, day, lon, tz_offset_h, &decl, &eqt);
    double ha = noaa_hour_angle_for_elevation(lat * DEG2RAD, decl, effective_elev);
    if (ha == HC_ZMAN_UNAVAILABLE) return HC_ZMAN_UNAVAILABLE;
    return is_rise ? noon - ha * 4.0 : noon + ha * 4.0;
}
