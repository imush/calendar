/* Port of JewishTimeTest.java — tests abs_heb_time arithmetic via hc_get_tekufa
   and direct inspection of hc_mult_heb_time / hc_add_heb_time. */
#include "hc_test.h"
#include "../src/hc_internal.h"

void test_jewish_time(void)
{
    /* add1: (0d 17h 19p) + (0d 22h 1076p) = (1d 16h 15p) */
    {
        hc_abs_heb_time t0 = {0, 17,  19, 0};
        hc_abs_heb_time t1 = {0, 22,1076, 0};
        hc_add_heb_time(&t0, &t1);
        HC_ASSERT_EQ_LONG(1,  t0.abs_date);
        HC_ASSERT_EQ_INT (16, t0.hour);
        HC_ASSERT_EQ_INT (15, t0.part);
        HC_ASSERT_EQ_INT (0,  t0.rega);
    }

    /* add2: (10000d 17h 19p) + (10000d 22h 1076p) = (20001d 16h 15p) */
    {
        hc_abs_heb_time t0 = {10000, 17,  19, 0};
        hc_abs_heb_time t1 = {10000, 22,1076, 0};
        hc_add_heb_time(&t0, &t1);
        HC_ASSERT_EQ_LONG(20001, t0.abs_date);
        HC_ASSERT_EQ_INT (16,    t0.hour);
        HC_ASSERT_EQ_INT (15,    t0.part);
    }

    /* subtract1: (10000d 22h 1076p) - (10000d 17h 19p) = (0d 5h 1057p) */
    {
        hc_abs_heb_time t0 = {10000, 22,1076, 0};
        hc_abs_heb_time sub= {10000, 17,  19, 0};
        hc_abs_heb_time neg = {-sub.abs_date, -sub.hour, -sub.part, 0};
        hc_add_heb_time(&t0, &neg);
        HC_ASSERT_EQ_LONG(0,    t0.abs_date);
        HC_ASSERT_EQ_INT (5,    t0.hour);
        HC_ASSERT_EQ_INT (1057, t0.part);
    }

    /* subtract2: (10001d 17h 19p) - (10000d 22h 1076p) = (0d 18h 23p) */
    {
        hc_abs_heb_time t0  = {10001, 17,  19, 0};
        hc_abs_heb_time sub = {10000, 22,1076, 0};
        hc_abs_heb_time neg = {-sub.abs_date, -sub.hour, -sub.part, 0};
        hc_add_heb_time(&t0, &neg);
        HC_ASSERT_EQ_LONG(0,  t0.abs_date);
        HC_ASSERT_EQ_INT (18, t0.hour);
        HC_ASSERT_EQ_INT (23, t0.part);
    }

    /* subtract3: x - x = 0 */
    {
        hc_abs_heb_time t0 = {10000, 22,1076, 0};
        hc_abs_heb_time neg= {-10000, -22,-1076, 0};
        hc_add_heb_time(&t0, &neg);
        HC_ASSERT_EQ_LONG(0, t0.abs_date);
        HC_ASSERT_EQ_INT (0, t0.hour);
        HC_ASSERT_EQ_INT (0, t0.part);
    }

    /* times1: (10000d 22h 1076p) * 2 = (20001d 21h 1072p) */
    {
        hc_abs_heb_time r = hc_mult_heb_time(10000, 22, 1076, 0, 2);
        HC_ASSERT_EQ_LONG(20001, r.abs_date);
        HC_ASSERT_EQ_INT (21,    r.hour);
        HC_ASSERT_EQ_INT (1072,  r.part);
    }

    /* times1b: * 22 = (220021d 1h 992p) */
    {
        hc_abs_heb_time r = hc_mult_heb_time(10000, 22, 1076, 0, 22);
        HC_ASSERT_EQ_LONG(220021, r.abs_date);
        HC_ASSERT_EQ_INT (1,      r.hour);
        HC_ASSERT_EQ_INT (992,    r.part);
    }

    /* times2: (0d 0h 1076p) * 2 = (0d 1h 1072p) */
    {
        hc_abs_heb_time r = hc_mult_heb_time(0, 0, 1076, 0, 2);
        HC_ASSERT_EQ_LONG(0,    r.abs_date);
        HC_ASSERT_EQ_INT (1,    r.hour);
        HC_ASSERT_EQ_INT (1072, r.part);
    }

    /* rega arithmetic: 76 rega = 1 chelek */
    {
        hc_abs_heb_time r = hc_mult_heb_time(0, 0, 0, 31, 4);
        /* 4*31=124 rega = 1 chelek + 48 rega */
        HC_ASSERT_EQ_INT(1,  r.part);
        HC_ASSERT_EQ_INT(48, r.rega);
    }
}
