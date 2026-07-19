#include "hc_test.h"

/* shared counters — defined once here, extern in hc_test.h */
int hc_tests_run    = 0;
int hc_tests_passed = 0;
int hc_tests_failed = 0;

/* forward declarations of test suites */
void test_jewish_time(void);
void test_hconverter(void);
void test_parshiot(void);
void test_tekufot(void);
void test_jewish_dates(void);
void test_year_scan(void);
void test_zmanim(void);

int main(void)
{
    HC_RUN_SUITE(test_jewish_time);
    HC_RUN_SUITE(test_hconverter);
    HC_RUN_SUITE(test_tekufot);
    HC_RUN_SUITE(test_parshiot);
    HC_RUN_SUITE(test_jewish_dates);
    HC_RUN_SUITE(test_year_scan);
    HC_RUN_SUITE(test_zmanim);

    hc_test_summary();
    return hc_tests_failed > 0 ? 1 : 0;
}
