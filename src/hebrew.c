#include "hconverter.h"
#include "hc_internal.h"
#include <stdio.h>
#include <string.h>

typedef struct abs_heb_time {
    long abs_date;
    int hour;
    int part;
} hc_abs_heb_time;

int set_hc_heb_time(hc_heb_time* htime, int hours, int parts)
{
	htime->hour = hours;
	htime->part = parts;
	return hours >= 0 && hours < 24 && parts >= 0 && parts < 1080;
}

int get_hour(hc_heb_time* dt) { return dt->hour; }
int get_parts(hc_heb_time* dt) {return dt->part; }

/**
    Enum of months for easier coding.
    We start here with NONE so that Nisan==1. 
*/
typedef enum HEB_MONTH {NONE, NISAN, IYAR, SIVAN, TAMUZ, AV, ELUL,
    TISHREI, CHESHVAN, KISLEV, TEVETH, SHVAT, ADAR, ADAR_2} heb_month;


static int heb_is_leap_year(const int year)
{
    /* check for leapness of a Hebrew year */
    static const int leap_map[19] = { 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0 };
	return leap_map[year % 19];
}

static int heb_month_length(const int year, const int month)
{
    if (month < 1 || month > 12 + heb_is_leap_year(year))
        return 0;
    switch (month) {
        case NISAN:
        case SIVAN:
        case AV:
        case TISHREI:
        case SHVAT:
            return 30;
        case ADAR_2:
            return 29;
        case ADAR:
            return heb_is_leap_year ? 30 :29;
        case CHESHVAN:
            return get_heb_year_type(year) == FULL_HEB_YEAR ? 30 : 29;
        case KISLEV:
            return get_heb_year_type(year) == SHORT_HEB_YEAR ? 29 : 30;
        case IYAR:
        case TAMUZ:
        case ELUL:
            return 29;
        default:
            // invalid month here
            return 0;
    }
}



static int heb_check_date(const int year, const int month, const int day)
{
	if (year < 1)
		return 1;
	if (month > 13 || (month == 13 && !heb_is_leap_year(year)))
		return 1;
	return day > 0 && day <= heb_month_length(year, month);
}

/* subroutine to add parts
 * target is the struct to be modified
 * to_add stays constant */
static void add_parts (hc_abs_heb_time *target, const hc_abs_heb_time *to_add)
{
    target->abs_date += to_add->abs_date;
    target->hour += to_add->hour;
    target->part += to_add->part;

    if (target->part >= 1080) {
        target->part -= 1080;
        target->hour++;
    }

    while (target->hour > 23) {
        target->hour -= 24;
        target->abs_date++;
    }
}

/* multiplication of parts here */
static hc_abs_heb_time mult_parts(const long days, const int hours, const int parts, const long int times)
{
  hc_abs_heb_time ret;
  long int D = days * times;
  long int H = hours * times;
  long int P = parts * times;

  H += P/1080;
  P = P % 1080;

  D += H/24;
  H = H % 24;

  ret.abs_date = D;
  ret.hour = H;
  ret.part = P;

  return ret;
}


static int compute_abs_molad_rosh_hashana(const int year, hc_abs_heb_time *abs_time)
{
	hc_abs_heb_time in;

	/* start with first molad */
	abs_time->abs_date = 6;
	abs_time->hour = 5;
	abs_time->part = 4;
	long pre_months;

    /*
        yr is number of years since first molad, in other words the year
        when the molad of Rosh Hashana occurs (in end of Elul).
    */
	int yr = year - 1;
	int cycles = yr / 19;
	int i;

	/* count number of months since beginning to molad of Rosh Hashana
	   There are 19*12+7 months per cycle */
	pre_months = 235 * cycles;
	for (i = 0; i < (yr % 19); i++)
	{
		pre_months += 12 + heb_is_leap_year(i+1);
	}

	/* now call mult_parts */

	in = mult_parts(29, 12, 793, pre_months);
	add_parts(abs_time, &in);
	return 0;
}


/* Calculate absolute day of Rosh Hashanah
   by first taking Molad and applying dehiyot as necessary  */
static long rosh_hashana_abs_date(const int year)
{
	hc_abs_heb_time molad;
	compute_abs_molad_rosh_hashana(year, &molad);
	long int day = molad.abs_date;
	hc_day_of_week dw = (day+2) % 7;

	/* Now come the 3 dehiyot. These are as follows:
         Molad zoken - R"H postponed 1 day if the molad occurs after 18 hrs in the day (i.e. after noon)
         gatra"d - if molad of R"H of nonleap year happens on Tuesday after 9 hrs 204 hl;
         bttkp"t - molad on Monday following a leap year after 15 hrs 589 hlkim
    */
	if (    molad.hour >= 18
		/* ^^^ molad zoken ^^^ */
		|| ( ! heb_is_leap_year(year) && dw == TUESDAY &&
			( molad.hour> 9 || (molad.hour == 9 && molad.part >= 204)))
		/* ^^^ gatra"d b'shanah pdhutah b'rosh ^^^ */
		|| ( heb_is_leap_year(year-1)  && dw == MONDAY &&
			( molad.hour > 15  || (molad.hour == 15 && molad.part >= 589) ))
		/* ^^^ Dehiyyah BeTU'TeKaPoT ^^^ */) {
		day++;
		dw++;
		dw %= 7;
	}

	/* Lo Ad"u - never to have R"H on sun/wed/fri */
	if ( dw == SUNDAY || dw == WEDNESDAY || dw == FRIDAY)
	{
		day++;
		dw++;
	}
	return day;
}

heb_year_type get_heb_year_type(const int year)
{
	const long r0 = rosh_hashana_abs_date(year);
	const long r1 = rosh_hashana_abs_date(year+1);
	long year_length = r1 -r0;
	heb_year_type t;
	if (year_length < 360)
		t = year_length - 353;
	else
		t = year_length - 383;
	return t;
}

/* convert Hebrew day to absolute */
static long heb_to_abs_date (const int year, const int month, const int day)
{
	// Hebrew months are numbered from Tishrei, which is month 7. This is
	// because and additional month on leap year is inserted in Adar, and it
	// is more convenient to have Adar 12th in order. Thus we start with 7, and
	// then wrap up.
	int mon = 7;

	long ret = rosh_hashana_abs_date(year) - 1;
	int months_in_year = 13 + heb_is_leap_year(year);
	while ( mon != month ) {
		ret += heb_month_length (year, mon);
		mon = (mon + 1) % months_in_year;
	}
	ret += day;
	return ret;
}


static int heb_compute_date(const long abs_date, hc_date *target)
{
    target->calendar_type = HEBREW;

	long int absday = abs_date - 1;
	long int premon;
	int dy, mh=7, yr, ml;


	/* first find out what is the current Hebrew year.
     get a reasonable lower bound for the year first so we won't
     have to count through thousands of years...
     Yes, this is not most efficient, I'll make a better approximation later. */
	premon = absday/29.7 - 1;
	yr = (premon/235)*19;

    /** molad is faster to compute until we get to the last year */
    while (rosh_hashana_abs_date(yr+1) <= abs_date)
        yr++;

	/* Ok, seems we got the right year now */
	target->year = yr;
	dy = abs_date - rosh_hashana_abs_date(yr) + 1;

	/* count as we go through the year; start with Tishrei and save rewinding the cycle */
	while ((ml = heb_month_length(yr, mh)) < dy) {
		dy -= ml;
		mh++;
		mh = ((mh-1) % (12+heb_is_leap_year(yr)))+1;
	}

	target->month = mh;
	target->day = dy;
	target->calendar_type = HEBREW;
	return 0;
}

int compute_molad(const int year, const int month, const hc_calendar_type cal_type,
		hc_date *date, hc_heb_time *time)
{
	hc_abs_heb_time molad;
	compute_abs_molad_rosh_hashana(year, &molad);
	hc_abs_heb_time to_add = mult_parts(29, 12, 793, month-1);
	add_parts(&molad, &to_add);
	heb_compute_date(molad.abs_date, date);
	if (cal_type != HEBREW)
		hc_convert(date, cal_type);

	set_hc_heb_time(time, molad.hour, molad.part);
	return 0;
}

int compute_molad_rosh_hashana(const int year, const hc_calendar_type cal_type,
		hc_date *date, hc_heb_time *time)
{
	return compute_molad(year, 1, cal_type, date, time);
}

/* set handles */
static hc_cal_impl hc_heb_impl_s =
{
    .abs_date = heb_to_abs_date,
    .compute_date = heb_compute_date,
	.check_date = heb_check_date,
	.is_leap_year = heb_is_leap_year,
	.month_length = heb_month_length
};

hc_cal_impl *heb_impl = &hc_heb_impl_s;
