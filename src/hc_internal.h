#ifndef SRC_HCONVERTER_INTERNAL_H_
#define SRC_HCONVERTER_INTERNAL_H_
#include "hconverter.h"

/* Absolute Hebrew moment (days-since-creation + time-of-day) used internally
   and by tekufot.c. Not part of the public API. */
typedef struct hc_abs_heb_time_s {
    long abs_date;
    int  hour;
    int  part;
    int  rega;
} hc_abs_heb_time;

/* Internal arithmetic on hc_abs_heb_time */
void hc_add_heb_time(hc_abs_heb_time *target, const hc_abs_heb_time *to_add);
hc_abs_heb_time hc_mult_heb_time(long days, int hours, int parts, int regas, long times);
void hc_get_molad_tohu(hc_abs_heb_time *out); /* BaHaRaD: day=2, h=5, p=204, r=0 */
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

/** Absolute day of Julian 1 January 1 CE.
 *
 * Two days before the Gregorian epoch: proleptic Gregorian 1 Jan 1 CE falls on
 * Julian 3 Jan 1 CE, so the same absolute day is a different date in each
 * calendar. Sharing one constant between them put every Julian date two days
 * out -- in both directions, since the same constant converts each way. */
extern const long JULIAN_BEGINNING;

/** Standard month lengths for Gregorian and Julian calendars */
extern const int COMMON_MONTH_LENGTH[12];

hc_cal_impl* get_calendar(hc_calendar_type calendar_type);

#endif
