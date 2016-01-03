#ifndef SRC_HCONVERTER_INTERNAL_H_
#define SRC_HCONVERTER_INTERNAL_H_
#include "hconverter.h"
/**
 * This struct contains pointers to specific functions in a calendar implementation.
 */
typedef struct hc_cal_impl_s {
	long (*abs_date)(int year, int month, int day);
	int (*compute_date)(long abs_date, hc_date *target);
	int (*check_date)(int year, int month, int day);
	int (*is_leap_year)(int year);
	int (*month_length)(int year, int month);
} hc_cal_impl;

extern hc_cal_impl *greg_impl;
extern hc_cal_impl *heb_impl;
extern hc_cal_impl *jul_impl;

/** Days since creation to start of Gregorian and Julian calendars */
extern const long COMMON_BEGINNING;

/** Standard month lengths for Gregorian and Julian calendars */
extern const int COMMON_MONTH_LENGTH[12];

hc_cal_impl* get_calendar(hc_calendar_type calendar_type);

#endif
