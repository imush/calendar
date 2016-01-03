#include "hconverter.h"
#include "hc_internal.h"

static int jul_month_length(const int year, const int month)
{
    if (month < 1 || month > 12)
        return -1;
    if (month == 2)
		return year % 4 == 0 ? 29 : 28;
    else
        return COMMON_MONTH_LENGTH[month-1];
}

static int jul_is_leap_year(const int year)
{
    return (year % 4 == 0) ? 1 : 0;
}

static int jul_check_date(const int year, const int month, const int day)
{
    if (month < 1 || month > 12)
        return 0;
    if (day < 1 || (day > 28 && (day > jul_month_length(year, month))))
        return 0;
    return 1;
}

static long jul_to_abs_date(const int year, const int month, const int day)
{
	long int ret = COMMON_BEGINNING;
	int m;
	int passed_years = year-1;

	ret += 365 * passed_years;
	ret += passed_years/4; 

	for (m = 1; m < month; m++) {
		ret += jul_month_length (year, m);
	}
	ret += day;
	return ret;
}

static int jul_compute_date(const long abs_date, hc_date *target)
{
	const long dy = abs_date - COMMON_BEGINNING;
	/* compute approximate lower bound for year */
	int yr = dy/366;
	int mh, next_eom;
	long dcount, next_dec31;

	// error - calendar does not exist yet
    if (abs_date <= COMMON_BEGINNING)
        return -1;

	/* set dcount to 31DEC of yr-1 */
	dcount = 365 * (yr-1); /* 365 times number of years */
	dcount += (yr-1)/4;    /* one extra day for each leap year */

	while ((next_dec31 = dcount + 365 + jul_is_leap_year(yr)) < dy) {
		dcount = next_dec31;
		yr++;
	}

    mh = 1;
	while ((next_eom = dcount + jul_month_length(yr, mh)) < dy) {
		dcount = next_eom;
		mh++;
	}

    target->year = yr;
    target->month = mh;
    target->day = dy - dcount;
	return 0;
}

static hc_cal_impl hc_jul_impl = {
    jul_to_abs_date,
    jul_compute_date,
	jul_check_date,
	jul_is_leap_year,
	jul_month_length
};

hc_cal_impl *jul_impl = &hc_jul_impl;
