#include "tekufot.h"
#include "hc_internal.h"

/*
 * Epoch: firstMoladNisan(year 1) = BaHaRaD + 0 months offset for Tishrei;
 * then we step back to Nisan by subtracting nisanOffset per opinion.
 *
 * firstMoladNisan = molad(year=1, month=Nisan=1).
 * Months from epoch (BaHaRaD = Tishrei year 1) to Nisan year 1:
 *   Tishrei(7) → Cheshvan(8) → ... → Elul(6) → Nisan(1): that is 6 months
 *   (in a non-leap year with 12 months: 7,8,9,10,11,12,1 → 6 steps).
 * firstMoladNisan = BaHaRaD + 6 * ONE_MONTH
 *
 * Then:
 *   Rav Ada:  firstTekufaNisan = firstMoladNisan − (0d 9h 642p 0r)
 *             seasonLength     = 91d 7h 519p 31r
 *   Shmuel:   firstTekufaNisan = firstMoladNisan − (7d 9h 642p 0r)
 *             seasonLength     = 91d 7h 540p 0r
 */

/* molad of Nisan year 1 = BaHaRaD + 6 months */
static hc_abs_heb_time first_molad_nisan(void)
{
    hc_abs_heb_time m;
    hc_get_molad_tohu(&m);
    hc_abs_heb_time six = hc_mult_heb_time(29, 12, 793, 0, 6);
    hc_add_heb_time(&m, &six);
    return m;
}

static void subtract_heb_time(hc_abs_heb_time *target, const hc_abs_heb_time *sub)
{
    hc_abs_heb_time neg;
    neg.abs_date = -sub->abs_date;
    neg.hour     = -sub->hour;
    neg.part     = -sub->part;
    neg.rega     = -sub->rega;
    hc_add_heb_time(target, &neg);
}

int hc_get_tekufa(hc_tekufa_opinion opinion, int hebrew_year, hc_season season,
                  long *abs_day_out, heb_time *time_out)
{
    if (hebrew_year < 1 || season < 0 || season > 3) return -1;

    hc_abs_heb_time first_nisan = first_molad_nisan();

    /* subtract nisanOffset to get firstTekufaNisan */
    hc_abs_heb_time offset;
    if (opinion == HC_RAV_ADA) {
        offset.abs_date = 0; offset.hour = 9; offset.part = 642; offset.rega = 0;
    } else { /* HC_SHMUEL */
        offset.abs_date = 7; offset.hour = 9; offset.part = 642; offset.rega = 0;
    }
    subtract_heb_time(&first_nisan, &offset);

    /* n = total seasons elapsed since firstTekufaNisan */
    long n = (long)(hebrew_year - 1) * 4 + (int)season;

    /* add n * seasonLength */
    hc_abs_heb_time delta;
    if (opinion == HC_RAV_ADA) {
        delta = hc_mult_heb_time(91, 7, 519, 31, n);
    } else {
        delta = hc_mult_heb_time(91, 7, 540, 0,  n);
    }
    hc_add_heb_time(&first_nisan, &delta);

    if (abs_day_out) *abs_day_out = first_nisan.abs_date;
    if (time_out) {
        time_out->hour = first_nisan.hour;
        time_out->part = first_nisan.part;
        time_out->rega = first_nisan.rega;
    }
    return 0;
}
