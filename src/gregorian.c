#include "hconverter.h"
#include "hc_internal.h"

static int greg_is_leap_year(const int year)
{
    if (year % 4 != 0)
        return 0;
    if (year % 100 != 0)
        return 1;
    return year % 400;
}

static int greg_month_length(const int year, const int month)
{
    if (month < 1 || month > 12)
        return -1;
    if (month == 2)
    	return greg_is_leap_year(year) ? 29 : 28;
    else
        return COMMON_MONTH_LENGTH[month-1];
}

static int greg_check_date(const int year, const int month, const int day)
{
    if (month < 1 || month > 12)
        return 0;
    if (day < 1 || (day > 28 && (day > greg_month_length(year, month))))
        return 0;
    return 1;
}

static long greg_to_abs_date(const int year, const int month, const int day)
{
	long ret = COMMON_BEGINNING;
	int m;
	int passed_years = year-1;

	ret += 365 * passed_years;
	ret += passed_years/4;
	ret -= passed_years/100;
	ret += passed_years/400;

	for (m = 1; m < month; m++) {
		ret += greg_month_length (year, m);
	}
	ret += day;
	return ret;
}

static int greg_compute_date(const long abs_date, hc_date *target)
{
	const long dy = abs_date - COMMON_BEGINNING;
	int yr, mh;
	long dcount, next_dec31, next_eom;

	// error - calendar does not exist yet
    if (dy < 1)
        return -1;
	/* compute approximate lower bound for year */
	yr = dy/366;

	/* set dcount to 31DEC of yr-1 */
	dcount = 365 * (yr-1); /* 365 times number of years */
	dcount += (yr-1)/4;    /* one extra day for each leap year */
	dcount -= (yr-1)/100;  /* every hundredth year is not leap */
	dcount += (yr-1)/400;  /* except once in 400 years */

	while ((next_dec31 = dcount + 365 + greg_is_leap_year(yr)) < dy) {
		dcount = next_dec31;
		yr++;
	}

    mh = 1;
	while ((next_eom = dcount + greg_month_length(yr, mh)) < dy) {
		dcount = next_eom;
		mh++;
	}

    target->year = yr;
    target->month = mh;
    target->day = dy - dcount;
	return 0;
}

static hc_cal_impl hc_greg_impl_s = {
    greg_to_abs_date,
    greg_compute_date,
	greg_check_date,
	greg_is_leap_year,
	greg_month_length
};

hc_cal_impl *greg_impl = &hc_greg_impl_s;
