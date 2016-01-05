#include "hconverter.h"
#include "hc_internal.h"
#include <math.h>
#include <stdio.h>

extern hc_cal_impl *greg_impl;
extern hc_cal_impl *heb_impl;
extern hc_cal_impl *jul_impl;

int set_hc_date(hc_date* date, const int year, const int month,
		const int day, const hc_calendar_type type)
{
	date->year = year;
	date->month = month;
	date->day = day;
	date->calendar_type = type;
	return hc_check(date);
}

int get_year(hc_date *date) { return date->year; }
int get_month(hc_date *date) { return date->month; }
int get_day(hc_date *date) { return date->day; }

hc_calendar_type get_calendar_type(hc_date *date) { return date->calendar_type; }

hc_cal_impl *get_calendar(hc_calendar_type type)
{
	switch (type) {
		case GREGORIAN: return greg_impl;
		case JULIAN: return jul_impl;
		case HEBREW: return heb_impl;
		default: return NULL;
	}
}

int hc_convert(hc_date *date, hc_calendar_type target_calendar)
{
	hc_cal_impl *impl0 = get_calendar(date->calendar_type);
	long abs_date = impl0->abs_date(date->year, date->month, date->day);
	if (abs_date < 0) return -1;
	hc_cal_impl *impl1 = get_calendar(target_calendar);
	return impl1->compute_date(abs_date, date);
}

int hc_check(hc_date *date)
{
	hc_cal_impl *impl = get_calendar(date->calendar_type);
	return impl->check_date(date->year, date->month, date->day);
}

int hc_is_leap_year(int year, hc_calendar_type cal_type)
{
	hc_cal_impl *impl = get_calendar(cal_type);
	return impl->is_leap_year(year);
}

hc_day_of_week hc_get_day_of_week(hc_date *date)
{
	hc_cal_impl *impl = get_calendar(date->calendar_type);
	long abs_date = impl->abs_date(date->year, date->month, date->day);
	return (hc_day_of_week)(abs_date+1)%7;
}

int hc_get_month_length(int year, int month, hc_calendar_type calendar_type)
{
	hc_cal_impl *impl = get_calendar(calendar_type);
	long abs_date = impl->month_length(year, month);
	return (hc_day_of_week)(abs_date+1)%7;
}
