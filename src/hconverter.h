/*
 * hconverter.h
 *
 *  Created on: Dec 29, 2015
 *      Author: itz
 */

#ifndef SRC_HCONVERTER_H_
#define SRC_HCONVERTER_H_

/**
 * days of week
 */
typedef enum hc_day_of_week {SUNDAY, MONDAY, TUESDAY, WEDNESDAY,
	THURSDAY, FRIDAY, SATURDAY} hc_day_of_week;
/**
 *  Supported types of calendar.
 */
typedef enum hc_calendar_type { GREGORIAN, JULIAN, HEBREW } hc_calendar_type;

/**
 * Structure representing a date
 */
typedef struct hc_date_s {
	hc_calendar_type calendar_type;
	int year;
	int month;
	int day;
} hc_date;

/**
 * Convenience method to set the entire hc_date in one call.
 * @param year
 * @param month
 * @param day
 * @param calendar_type
 */
int set_hc_date(hc_date*, int year, int month, int day, hc_calendar_type);

int set_calendar_type(hc_date*, hc_calendar_type);
int get_year(hc_date*);
int get_month(hc_date*);
int get_day(hc_date*);
hc_calendar_type get_calendar_type(hc_date *);

/**
 * This function converts a date from one calendar to another.
 * Returns 0 on success, 1 on invalid input date object. The given
 * hc_date structure is modified during this call, no new memory
 * is allocated.
 */
int hc_convert(hc_date *date, hc_calendar_type targetCalendar);

/**
 * Return zero if the date is valid in corresponding calendar
 */
int hc_check(hc_date *date);

/**
 * Check if year is leap in given calendar. For Gregorian or Julian
 * calendar this means that length of the year is 366 days. In Hebrew
 * calendar it means that the year has 13 months instead of 12.
 * @param
 */
int hc_is_leap_year(int year, hc_calendar_type calendarType);

hc_day_of_week hc_get_day_of_week(hc_date *date);

int hc_get_month_length(int year, int month,hc_calendar_type calendarType);

/**
 * This type is specific to Hebrew calendar. Time is measured in
 * hours and chalokim or "parts", which are 1/1080 of an hour.
 */
typedef struct hc_heb_time_s {
	int hour;
	int part;
} hc_heb_time;
int set_hc_heb_time(hc_heb_time*, int hour, int part);
int get_hour(hc_heb_time*);
int get_parts(hc_heb_time*);

/**
 * Compute the date time of molad of Rosh Hashana for given year.
 * hc_date and hc_time objects are passed in to be documented.
 */
int compute_molad_rosh_hashana(int year, hc_calendar_type cal_type,
		hc_date *date, hc_heb_time *time);
/**
 * Compute the date time of molad for given year and month.
 * hc_date and hc_time objects are passed in to be documented.
 */
int compute_molad(const int year, int month, const hc_calendar_type cal_type,
		hc_date *date, hc_heb_time *time);

/** Number of days in excess of 58 in Cheshvan and Kislev combined.
 *  Can take 3 values
 *  SHORT_HEB_YEAR  (0)   - both Cheshvan and Kislev 29 days long
 *  NORMAL_HEB_YEAR (1)   - 29 days in Cheshvan, 30 days in Kislev
 *  FULL _HEB_YEAR  (2)   - 30 days in both Cheshvan and Kislev   */
typedef enum heb_year_type {SHORT_HEB_YEAR, NORMAL_HEB_YEAR, FULL_HEB_YEAR} heb_year_type;

heb_year_type get_heb_year_type(int year);

#endif /* SRC_HCONVERTER_H_ */
