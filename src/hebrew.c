#include "hconverter.h"
#include "hc_internal.h"
#include <stdio.h>
#include <string.h>


/* hc_abs_heb_time defined in hc_internal.h */

int hc_set_hc_heb_time(heb_time* htime, int hours, int parts, int regas)
{
	htime->hour = hours;
	htime->part = parts;
	htime->rega = regas;
	return hours >= 0 && hours < 24 && parts >= 0 && parts < 1080 && regas >= 0 && regas < 76;
}

int get_hour(heb_time* dt) { return dt->hour; }
int get_parts(heb_time* dt) {return dt->part; }

/*!
\brief Enum of months for easier coding.

We start here with a dummy NULL_MONTH so that Nisan==1. 
*/
typedef enum HEB_MONTH {NULL_MONTH, NISAN, IYAR, SIVAN, TAMUZ, AV, ELUL,
    TISHREI, CHESHVAN, KISLEV, TEVETH, SHVAT, ADAR, ADAR_2} heb_month;


int heb_is_leap_year(const int year)
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

        case TEVETH:
        case IYAR:
        case TAMUZ:
        case ELUL:
        case ADAR_2:
            return 29;

        case ADAR:
            return heb_is_leap_year(year) ? 30 : 29;
        case CHESHVAN:
            return hc_get_heb_year_type(year) == FULL_HEB_YEAR ? 30 : 29;
        case KISLEV:
            return hc_get_heb_year_type(year) == SHORT_HEB_YEAR ? 29 : 30;
        default:
            // invalid month here
            return 0;
    }
}



static int heb_check_date(const int year, const int month, const int day)
{
	if (year < 1)
		return 0;
	if (month > 13 || (month == 13 && !heb_is_leap_year(year)))
		return 0;
	return day > 0 && day <= heb_month_length(year, month);
}

/* internal add/mult exposed via hc_internal.h for tekufot.c */
void hc_add_heb_time(hc_abs_heb_time *target, const hc_abs_heb_time *to_add);
hc_abs_heb_time hc_mult_heb_time(long days, int hours, int parts, int regas, long times);
void hc_get_molad_tohu(hc_abs_heb_time *out);

static void add_parts(hc_abs_heb_time *target, const hc_abs_heb_time *to_add)
{
    target->abs_date += to_add->abs_date;
    target->hour     += to_add->hour;
    target->part     += to_add->part;
    target->rega     += to_add->rega;

    if (target->rega >= 76)  { target->part += target->rega / 76; target->rega %= 76; }
    else if (target->rega < 0) {
        int b = (-target->rega + 75) / 76; target->part -= b; target->rega += b * 76;
    }
    if (target->part >= 1080) { target->hour += target->part / 1080; target->part %= 1080; }
    else if (target->part < 0) {
        int b = (-target->part + 1079) / 1080; target->hour -= b; target->part += b * 1080;
    }
    while (target->hour >= 24) { target->hour -= 24; target->abs_date++; }
    while (target->hour <   0) { target->hour += 24; target->abs_date--; }
}

/* multiply a duration (days,hours,parts,regas) by times */
static hc_abs_heb_time mult_parts(const long days, const int hours, const int parts, const int regas, const long times)
{
    hc_abs_heb_time ret;
    long D = days  * times;
    long H = hours * times;
    long P = parts * times;
    long R = regas * times;

    P += R / 76; R = R % 76;
    if (R < 0) { long b = (-R + 75) / 76; P -= b; R += b * 76; }

    H += P / 1080; P = P % 1080;
    if (P < 0) { long b = (-P + 1079) / 1080; H -= b; P += b * 1080; }

    D += H / 24; H = H % 24;
    if (H < 0) { D--; H += 24; }

    ret.abs_date = D;
    ret.hour     = (int)H;
    ret.part     = (int)P;
    ret.rega     = (int)R;
    return ret;
}

static int compute_abs_molad_rosh_hashana(const int year, hc_abs_heb_time *abs_time)
{
	hc_abs_heb_time in;

	/* start with first molad */
	abs_time->abs_date = 2;
	abs_time->hour = 5;
	abs_time->part = 204;
	abs_time->rega = 0;
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
	pre_months = 235L * cycles;
	for (i = 0; i < (yr % 19); i++)
	{
		pre_months += 12 + heb_is_leap_year(i+1);
	}

	in = mult_parts(29, 12, 793, 0, pre_months);
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
	hc_day_of_week dw = (hc_day_of_week)((day-1) % 7);

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

heb_year_type hc_get_heb_year_type(const int year)
{
	const long r0 = rosh_hashana_abs_date(year);
	const long r1 = rosh_hashana_abs_date(year+1);
	long year_length = r1 -r0;
	heb_year_type t;
	if (year_length < 360)
		t = (heb_year_type)(year_length - 353);
	else
		t = (heb_year_type)(year_length - 383);
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
	premon = (long)(absday/29.7 - 1);
	yr = (int)((premon/235)*19);

    /** molad is faster to compute until we get to the last year */
    while (rosh_hashana_abs_date(yr+1) <= abs_date)
        yr++;

	/* Ok, seems we got the right year now */
	target->year = yr;
	dy = (int)(abs_date - rosh_hashana_abs_date(yr) + 1);

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

int hc_compute_molad(const int year, const int month, const hc_calendar_type cal_type,
		hc_date *date, heb_time *time)
{
	hc_abs_heb_time molad;
	compute_abs_molad_rosh_hashana(year, &molad);
	{
		/* months past Tishrei: Nisan..Elul account for leap year (7 or 6 months before Nisan) */
		int offset = month > 6 ? month - 7
		                       : month - 1 + (heb_is_leap_year(year) ? 7 : 6);
		hc_abs_heb_time to_add = mult_parts(29, 12, 793, 0, offset);
		add_parts(&molad, &to_add);
	}
	heb_compute_date(molad.abs_date, date);
	if (cal_type != HEBREW)
		hc_convert(date, cal_type);

	hc_set_hc_heb_time(time, molad.hour, molad.part, molad.rega);
	return 0;
}

int hc_compute_molad_rosh_hashana(const int year, const hc_calendar_type cal_type,
		hc_date *date, heb_time *time)
{
	return hc_compute_molad(year, 1, cal_type, date, time);
}

int hc_abs_day_to_gregorian(long abs_day, hc_date *out)
{
	heb_compute_date(abs_day, out);
	return hc_convert(out, GREGORIAN);
}

int hc_compute_keviut(const int year, int *rosh_hashana_dow, int *pesach_begin_dow, int *ck, int *leap)
{
	long rosh, pesach;
	if (year < 1)
		return -1;
	rosh = rosh_hashana_abs_date(year);
	if (rosh_hashana_dow != NULL)
		*rosh_hashana_dow = (int)(rosh % 7);
	pesach = heb_to_abs_date(year, 1, 15);
	if (pesach_begin_dow != NULL)
		*pesach_begin_dow = (int)(pesach % 7);
	if (ck != NULL)
		*ck = (int)hc_get_heb_year_type(year);
	if (leap != NULL)
		*leap = heb_is_leap_year(year);
	return 0;
}


/* ── package-internal helpers exposed via hc_internal.h ──────────────────── */

void hc_add_heb_time(hc_abs_heb_time *target, const hc_abs_heb_time *to_add)
{
    add_parts(target, to_add);
}

hc_abs_heb_time hc_mult_heb_time(long days, int hours, int parts, int regas, long times)
{
    return mult_parts(days, hours, parts, regas, times);
}

void hc_get_molad_tohu(hc_abs_heb_time *out)
{
    out->abs_date = 2;
    out->hour     = 5;
    out->part     = 204;
    out->rega     = 0;
}

/* next Hebrew month within target_year (used for fallback to Rosh Chodesh) */
static int heb_next_month(const int target_year, const int month)
{
    const int months_in_year = 12 + heb_is_leap_year(target_year);
    if (month == months_in_year) return 1;   /* last month (Adar/Adar II) → Nisan */
    if (month == 6)              return 7;   /* Elul → Tishrei (unreachable: Elul always 29 days) */
    return month + 1;
}

static int anniversary_impl(const int orig_year, const int orig_month, const int orig_day,
                             const int target_year, const int adar_policy_forward, hc_date *out)
{
    int month = orig_month;

    if (!heb_check_date(orig_year, orig_month, orig_day) || target_year < 1)
        return -1;

    /* Adar mapping */
    if (adar_policy_forward) {
        /* anniversary: non-leap Adar → Adar II in leap target year */
        if (month == ADAR && !heb_is_leap_year(orig_year) && heb_is_leap_year(target_year))
            month = ADAR_2;
        else if (month == ADAR_2 && !heb_is_leap_year(target_year))
            month = ADAR;
    } else {
        /* yahrzeit: non-leap Adar stays Adar I in a leap target year */
        if (month == ADAR_2 && !heb_is_leap_year(target_year))
            month = ADAR;
    }

    /* if the exact day doesn't exist in target year, fall to 1st of next month */
    if (orig_day > heb_month_length(target_year, month)) {
        out->calendar_type = HEBREW;
        out->year  = target_year;
        out->month = heb_next_month(target_year, month);
        out->day   = 1;
    } else {
        out->calendar_type = HEBREW;
        out->year  = target_year;
        out->month = month;
        out->day   = orig_day;
    }
    return 0;
}

int hc_anniversary_for(const int orig_year, const int orig_month, const int orig_day,
                        const int target_year, hc_date *out)
{
    return anniversary_impl(orig_year, orig_month, orig_day, target_year, 1, out);
}

int hc_yahrzeit_for(const int death_year, const int death_month, const int death_day,
                     const int target_year, hc_date *out)
{
    return anniversary_impl(death_year, death_month, death_day, target_year, 0, out);
}

/* set handles */
static hc_cal_impl hc_heb_impl_s =
{
    heb_to_abs_date,
    heb_compute_date,
	heb_check_date,
	heb_is_leap_year,
	heb_month_length
};

hc_cal_impl *heb_impl = &hc_heb_impl_s;
