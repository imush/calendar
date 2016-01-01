/**
 Some definitions common to Julian and Gregorian implementations
 */

/** Days since creation to start of Gregorian and Julian calendars */
const long COMMON_BEGINNING = 1373434;

/** Standard month lengths for Gregorian and Julian calendars */
const int COMMON_MONTH_LENGTH[12] =
	{ 31, -1, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
