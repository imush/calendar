/*!
  \brief Public header file for hconverter library.
===================================================


  \file hconverter.h
  \date Dec 29, 1998
  \author Isaac Mushinsky
  

  Contains all functional offerings of this library: convert dates between
  Hebrew, Gregorian and Julian calendars as well as compute certain specific
  times for the new moon, types of Jewish year.

 */

#ifndef SRC_HCONVERTER_H_
#define SRC_HCONVERTER_H_

/*!
 * Convenience enum for days of week: <i>SUNDAY=0, MONDAY=1, ..., FRIDAY=6</i>
 */
typedef enum hc_day_of_week {SUNDAY, MONDAY, TUESDAY, WEDNESDAY,
	THURSDAY, FRIDAY, SATURDAY} hc_day_of_week;
/*!
  \brief Supported types of calendar.

  \li GREGORIAN
  \li JULIAN
  \li HEBREW
 */
typedef enum hc_calendar_type { NONE, GREGORIAN, JULIAN, HEBREW } hc_calendar_type;

/**
 * Structure representing a date.
 */
typedef struct hc_date_s {
    hc_calendar_type calendar_type;
    int year;
    int month;
    int day;
} hc_date;

/*!
 Convenience method to set the entire ::hc_date in one call.
 \param[out] date pointer to ::hc_date structure
 \param[in] year
 \param[in] month
 \param[in] day
 \param[in] calendar_type
 */
int set_hc_date(hc_date* , int year, int month, int day, hc_calendar_type);

/*!
\brief This function converts a date from one calendar to another.

 The given ::hc_date structure is modified during this call, so no new memory
 is allocated here.
 
\param[out] date This object will be modified so that it has the target #hc_calendar_type set
 along with corresponding date.
\param[in] target_calendar see #hc_calendar_type
\return 0 on success, -1 if the input date is invalid.
*/
int hc_convert(hc_date *date, hc_calendar_type target_calendar);

/*!
\brief Check validity of data in ::hc_date
 
 Returns 1 if the date is valid, 0 if the date is invalid in corresponding calendar.
 The following are invalid: nonpositive Hebrew year, month less than 1 or more than number
 of months in given year, day less than one or more than days in month.
 
\param[in] date an ::hc_date
\return 0 if invalid, 1 otherwise.
*/
int hc_check(hc_date *date);

/*!
\brief Check if year is leap in given calendar. 

For Gregorian or Julian calendar this means that length of the
year is 366 days. In Hebrew calendar it means that the year has 13 months
instead of 12.

\param[in] year Hebrew year >=1
\param[in] calendar_type see #hc_calendar_type
\return 1 for leap year, 0 for non-leap year
*/
int hc_is_leap_year(int year, hc_calendar_type calendar_type);

/*!
\brief Function to get day of week out of a ::hc_date.

\param date
\return a #hc_day of week: 0 for Saturday, 1 for Sunday, ..., 6 for Friday
*/
hc_day_of_week hc_get_day_of_week(hc_date *date);

/*!
\brief Function to get the length of a month.

\param[in] year
\param[in] month. For Hebrew, see (\ref hebmonth "note") about the special
month order.
\param[in] calendar_type see #hc_calendar_type
\return length of month in days
*/
int hc_get_month_length(int year, int month, hc_calendar_type calendar_type);

/*!
\brief Compute year "keviut" and put the result into 4 integers.

Keviut determines layout of the Jewish year, including all holidays and
readings of the Torah. See \ref keviut for more information. Here, we output
the "keviut" parameters by setting values of 4 integers whose pointers are supplied
with parameters. For any of these output parameters, null pointers are allowed and
will be ignored, so users can pass NULL rather than allocate integers for output that
they do not need.

\param[in] year Hebrew year
\param[out] rosh_hashana_dow Day of week for Eosh Hashana (1 Tishrei)
\param[out] pesach_dow Day of week for Pesach (0 = saturday)
\param[out] ck returns 0, 1, 2 (SHORT, REGULAR, FULL) for number of days in
excess of 58 in Chesh=van and Kislev combined. (see #hc_heb_year_type)
\param[out] it will return 1 for leap years and 0 otherwise
 */
int hc_compute_keviut(const int year, int *rosh_hashana_dow, int *pesach_dow, int *ck, int *leap);

/*!
\brief Hebrew time: hour and parts

This type is specific to Hebrew calendar. Time is measured in
hours and chalokim ("parts"), which are 1/1080 of an hour.

*/
typedef struct heb_time_s {
	int hour;
	int part;
} heb_time;

/*!
\brief Convenience method to set ::heb_time struct value.

\param param[out] time
\param param[in] hour
\param param[in] part
*/
int hc_set_hc_heb_time(heb_time* time, int hour, int part);

/*!
\brief Compute the date time of molad of Rosh Hashana for given year.

This is equivalent to <hc_compute_molad> with month set to 7 (Tishrei).

The output params ::hc_date and ::heb_time objects are passed in to be populated.
\param[in] year
\param[in] hc_calendar_type GREGORINA, JULIAN or HEBREW
\param[out] date pointer to ::hc_date struct to store result
\param[out] time pointer to ::heb_time to store time result
*/
int hc_compute_molad_rosh_hashana(int year, hc_calendar_type cal_type,
		hc_date *date, heb_time *time);

/*!
 \brief Compute the date and time of molad (new moon according to the Hebrew calendar)
 for given year and month.
 
 Output parameters are ::hc_date and ::heb_time objects, passed in to be populated. No
 New memory is allocated.
 
 \param[in] year
 \param[in] month
 \param[in] hc_calendar_type GREGORIAN, JULIAN or HEBREW
 \param[out] date pointer to ::hc_date struct to store result
 \param[out] time pointer to ::heb_time to store time result
 */
int hc_compute_molad(const int year, int month, const hc_calendar_type cal_type,
		hc_date *date, heb_time *time);

/*!
  \brief enum of possible layouts of the variable length Hebrew months Cheshvan and Kislev
  in a given year.
  
  \li SHORT_HEB_YEAR means that both Cheshvan and Kislev are 29 days long
  \li NORMAL_HEB_YEAR means 29 days in Cheshvan and 30 days in Kislev
  \li FULL_HEB_YEAR means that both months are 30 days long

*/
typedef enum heb_year_type {SHORT_HEB_YEAR, NORMAL_HEB_YEAR, FULL_HEB_YEAR} heb_year_type;


/*!
\brief Compute number of days in excess of 58 in Cheshvan and Kislev combined
  
  \param[in] year Hebrew year >=1
  \return can take 3 values:
  \li <b>SHORT_HEB_YEAR  (0)</b>   - both Cheshvan and Kislev 29 days long
  \li <b>NORMAL_HEB_YEAR (1)</b>   - 29 days in Cheshvan, 30 days in Kislev
  \li <b>FULL _HEB_YEAR  (2)</b>   - 30 days in both Cheshvan and Kislev
*/
heb_year_type hc_get_heb_year_type(int year);

/*!
\file

\anchor hebmonth
Note about Hebrew month ordering.
--------------------------------

Although each Hebrew year starts with the month of Tishrei, Tishrei is not
the first in the order of months. There are a couple of reasons for this. Nisan
is called the "first month" in the Torah for most purposes. Also, on a Hebrew
leap year the 13th month (adar 2) is added before Nisan, not before Tishrei. This makes
it more convenient to enumerate months not in chronological order but rather starting
from Nisan to the end of year (Elul), and then returning to Tishrei through Adar. Thus
the Hebrew months here are represented by integers as follows:

  \li \c Tishrei      7
  \li \c Cheshvan     8
  \li \c Kislev       9
  \li \c Teveth      10
  \li \c Shvat       11
  \li \c Adar        12   (also Adar I on leap years)
  \li \c Adar II     13   (only on Hebrew leap years)
  \li \c Nisan        1
  \li \c Iyar         2
  \li \c Sivan        3
  \li \c Tamuz        4
  \li \c Menachem Av  5
  \li \c Elul         6
  
so despite a lower month index, Nisan follows \b after Adar. For example, the date 5776-01-01
follows 5776-13-29 in yyyy-mm-dd notation.

\anchor keviut
Note about keviut
-----------------

Keviut determines layout of the Jewish year, including all holidays and
readings of the Torah. It is customary to abbreviate it to 4 parameters:

\li \b 1 day of week of Rosh Hashana, i.e. the "new year", Tishrei 1
\li \b 2 day of week of first day of Pesach, i.e. Nisan 15
\li \b 3 combined length of Cheshvan and Kislev (see #hc_heb_year_type)
\li \b 4 whether or not the year is leap

Only 14 combinations of these are actually possible.
*/
#endif /* SRC_HCONVERTER_H_ */
