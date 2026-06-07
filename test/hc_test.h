/*!
 * \file hc_test.h
 * \brief Minimal C test framework (no external dependencies).
 */
#ifndef HC_TEST_H_
#define HC_TEST_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int hc_tests_run;
extern int hc_tests_passed;
extern int hc_tests_failed;

#define HC_ASSERT(cond, msg) \
    do { \
        hc_tests_run++; \
        if (cond) { \
            hc_tests_passed++; \
        } else { \
            hc_tests_failed++; \
            fprintf(stderr, "FAIL [%s:%d] %s: %s\n", __FILE__, __LINE__, __func__, msg); \
        } \
    } while(0)

#define HC_ASSERT_EQ_INT(expected, actual) \
    do { \
        int _e = (expected), _a = (actual); \
        hc_tests_run++; \
        if (_e == _a) { hc_tests_passed++; } \
        else { hc_tests_failed++; \
               fprintf(stderr, "FAIL [%s:%d] %s: expected %d, got %d\n", \
                       __FILE__, __LINE__, __func__, _e, _a); } \
    } while(0)

#define HC_ASSERT_EQ_LONG(expected, actual) \
    do { \
        long _e = (expected), _a = (actual); \
        hc_tests_run++; \
        if (_e == _a) { hc_tests_passed++; } \
        else { hc_tests_failed++; \
               fprintf(stderr, "FAIL [%s:%d] %s: expected %ld, got %ld\n", \
                       __FILE__, __LINE__, __func__, _e, _a); } \
    } while(0)

#define HC_ASSERT_TRUE(cond)  HC_ASSERT((cond),  #cond " should be true")
#define HC_ASSERT_FALSE(cond) HC_ASSERT(!(cond), #cond " should be false")

#define HC_RUN_SUITE(name) \
    do { fprintf(stdout, "\n=== %s ===\n", #name); name(); } while(0)

static void hc_test_summary(void) __attribute__((unused));
static void hc_test_summary(void)
{
    fprintf(stdout, "\n── Test summary ──────────────────────────\n");
    fprintf(stdout, "  Run:    %d\n", hc_tests_run);
    fprintf(stdout, "  Passed: %d\n", hc_tests_passed);
    fprintf(stdout, "  Failed: %d\n", hc_tests_failed);
    fprintf(stdout, "──────────────────────────────────────────\n");
}

#endif /* HC_TEST_H_ */
