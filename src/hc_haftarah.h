/*!
 * \file hc_haftarah.h
 * \brief Which haftarah is read on a given day (or the upcoming Shabbat).
 *
 * The reference data itself lives in the hebrewcalendar-data submodule
 * (haftarot_data.h — weekly parsha × custom, plus the special-occasion
 * table). This header owns the *precedence* rules that pick between
 * them, mirroring opentorah's SpecialReadings.scala:
 *
 *     Yom Tov > Chol HaMoed Shabbat > Special Parsha > Shabbat Hagadol
 *             > Chanukah > Rosh Chodesh > Machar Chodesh > weekly parsha
 *
 * This is the single source of truth: the Java and Dart wrappers call
 * straight through to it rather than re-implementing the precedence.
 */

#ifndef SRC_HC_HAFTARAH_H_
#define SRC_HC_HAFTARAH_H_

#include "hconverter.h"
#include "haftarot_data.h"

/*! Longest multi-part haftarah in the data set (Rosh Chodesh Chabad = 3). */
#define HC_MAX_HAFTARAH_REFS    6

/*! Most readings a single day can carry (Yom Kippur / Tisha B'Av = 2). */
#define HC_MAX_HAFTARAH_RESULTS 3

/*!
 * What drove the choice of haftarah. Consumers can use this to label
 * the reading ("Rosh Chodesh", "Parshat Zachor") without repeating the
 * classification work.
 */
typedef enum hc_haftarah_occasion {
    HC_HAFT_OCC_WEEKLY = 0,          /*!< ordinary weekly parsha haftarah */
    HC_HAFT_OCC_PARSHAT_SHEKALIM,
    HC_HAFT_OCC_PARSHAT_ZACHOR,
    HC_HAFT_OCC_PARSHAT_PARAH,
    HC_HAFT_OCC_PARSHAT_HACHODESH,
    HC_HAFT_OCC_SHABBAT_HAGADOL,
    HC_HAFT_OCC_CHANUKAH_SHABBAT_1,
    HC_HAFT_OCC_CHANUKAH_SHABBAT_2,
    HC_HAFT_OCC_ROSH_CHODESH,
    HC_HAFT_OCC_MACHAR_CHODESH,
    HC_HAFT_OCC_ROSH_HASHANA,
    HC_HAFT_OCC_YOM_KIPPUR,
    HC_HAFT_OCC_YOM_KIPPUR_AFTERNOON,
    HC_HAFT_OCC_SUKKOT,
    HC_HAFT_OCC_SHMINI_ATZERET,
    HC_HAFT_OCC_SIMCHAT_TORAH,
    HC_HAFT_OCC_PESACH,
    HC_HAFT_OCC_SHAVUOT,
    HC_HAFT_OCC_CHOL_HAMOED_PESACH,
    HC_HAFT_OCC_CHOL_HAMOED_SUKKOT,
    HC_HAFT_OCC_TISHA_BAV,
    HC_HAFT_OCC_TISHA_BAV_AFTERNOON,
    HC_HAFT_OCC_FAST_AFTERNOON,
    HC_HAFT_OCC_COUNT
} hc_haftarah_occasion;

/*! One resolved reading: the occasion plus its (possibly multi-part) refs. */
typedef struct {
    hc_haftarah_occasion occasion;
    hc_haftarah_ref      refs[HC_MAX_HAFTARAH_REFS];
    int                  refs_count;
} hc_haftarah_result;

/*!
 * \brief Haftarah for the current or upcoming Shabbat.
 *
 * If \p date is itself a Saturday that Shabbat is used; otherwise the
 * next one. Always yields a reading (every Shabbat of the year has one).
 *
 * \param[in]  date       any calendar type
 * \param[in]  custom     which minhag's haftarah to resolve
 * \param[in]  in_israel  1 for Eretz Israel schedule, 0 for Diaspora
 * \param[out] out        populated on success
 * \return 0 on success, -1 on invalid input or if no reading resolves
 */
int hc_haftarah_for_date(hc_date *date, hc_custom custom, int in_israel,
                         hc_haftarah_result *out);

/*!
 * \brief Haftarah reading(s) that fall ON \p date itself.
 *
 *   - Shabbat            → 1 result (same as hc_haftarah_for_date)
 *   - Yom Tov weekday    → 1 result
 *   - Yom Kippur         → 2 results (morning + afternoon)
 *   - Tisha B'Av         → 2 results (morning + afternoon)
 *   - other public fasts → 1 result (afternoon only; some customs have
 *                          no fast-day haftarah at all, giving 0)
 *   - Chol HaMoed weekday / ordinary weekday → 0 results
 *
 * \param[out] results  array of at least HC_MAX_HAFTARAH_RESULTS entries
 * \param[out] count    number of results written (may be 0)
 * \return 0 on success, -1 on invalid input
 */
int hc_haftarah_for_day(hc_date *date, hc_custom custom, int in_israel,
                        hc_haftarah_result results[HC_MAX_HAFTARAH_RESULTS],
                        int *count);

/*! English label for an occasion (NULL if out of range). */
const char *hc_haftarah_occasion_name(hc_haftarah_occasion occ);

/*!
 * \name ABI introspection
 * FFI wrappers (Dart, JNA, …) redeclare hc_haftarah_result in their own
 * type system and have to get the padding right. These let a wrapper
 * assert its layout against the compiler's on the actual target ABI
 * instead of hardcoding offsets that hold only on x86-64.
 */
/**@{*/
int hc_haftarah_sizeof_ref(void);
int hc_haftarah_sizeof_result(void);
int hc_haftarah_offsetof_refs(void);
int hc_haftarah_offsetof_refs_count(void);
/**@}*/

#endif /* SRC_HC_HAFTARAH_H_ */
