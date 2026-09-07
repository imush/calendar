/*!
 * \file hc_torah_reading.h
 * \brief What is read from the Torah on a given day.
 *
 * Distinct from the parsha of the week, which names it: this is what is read
 * from the scroll, aliyah by aliyah, with the maftir where there is one.
 *
 * The data is generated (torah_data.h -- the weeks and the readings of the
 * special days); the divisions built from it are here. Verified against
 * opentorah's own answers -- see test/cross_validate_readings.c.
 */
#ifndef SRC_HC_TORAH_READING_H_
#define SRC_HC_TORAH_READING_H_

#include "hconverter.h"
#include "parshiot.h"
#include "torah_data.h"

/*! Seven aliyot on a Shabbat, and one more where Rosh Chodesh takes a place. */
#define HC_MAX_ALIYOT 8
/*! Night, morning and Mincha: no day reads at more than three times. */
#define HC_MAX_TORAH_READINGS 3

/*!
 * When in the day a reading is read. The Jewish day begins at night, so
 * EVENING comes first: one reading falls there, on Simchat Torah.
 */
typedef enum hc_reading_slot {
    HC_SLOT_EVENING,
    HC_SLOT_MORNING,
    HC_SLOT_AFTERNOON
} hc_reading_slot;

/*!
 * One reading: its aliyot in order, the maftir if there is one, and which
 * scroll each is read from.
 */
typedef struct {
    hc_reading_slot slot;

    hc_torah_span aliyot[HC_MAX_ALIYOT];
    int           aliyot_count;

    /*! Which scroll each aliyah is read from, 1-based, parallel to aliyot;
     *  maftir_sefer says the same for the maftir, or 0 where there is none.
     *  A scroll is taken out for each passage read from a different place, so
     *  this follows from where each passage came from, not from how far apart
     *  the verses are: an ordinary maftir repeats the tail of the parsha, and
     *  Rosh Chodesh's own aliyot overlap one another. */
    uint8_t aliyot_sefer[HC_MAX_ALIYOT];

    /*! book is HC_BOOK_NONE when the reading has no maftir. */
    hc_torah_span maftir;
    uint8_t       maftir_sefer;

    /*! The parshiyot this reading is of -- two for a joined week -- and
     *  HC_PARSHA_NONE where the reading is not the weekly parsha at all,
     *  which is every festival and fast. On a Monday, a Thursday or at
     *  Mincha it is the parsha of the Shabbat ahead. */
    uint8_t parshiyot[2];

    /*! The custom whose division of the parsha this follows, where that is not
     *  the common one -- Chabad on Vayigash, Naso and Devarim, Ashkenaz on
     *  Masei -- and HC_CUSTOM_COUNT otherwise. */
    uint8_t division;

    /*! What is worth saying beyond the verses, or NULL. Only the night of
     *  Simchat Torah has one. English, as recorded upstream. */
    const char *note;
} hc_torah_reading;

/*!
 * \brief The Shabbat morning reading for \p date.
 *
 * \return 0 on success, -1 if \p date is not a Shabbat with a weekly parsha.
 */
int hc_torah_reading_for_date(hc_date *date, hc_custom custom, int in_israel,
                              hc_torah_reading *out);

/*!
 * \brief Every Torah reading that falls ON \p date, in the order they are read.
 *
 *   - Shabbat morning     → the week's parsha, or a festival that has taken it
 *   - festival or fast    → its own reading
 *   - Rosh Chodesh, Chanukah on a weekday → theirs
 *   - Monday, Thursday, Shabbat Mincha    → the opening three of the parsha
 *                                           of the Shabbat ahead
 *   - Yom Kippur and the fasts            → an afternoon reading as well
 *   - Simchat Torah                       → a reading at night, for some
 *   - an ordinary weekday                 → nothing
 *
 * \param[out] results  array of at least HC_MAX_TORAH_READINGS entries
 * \param[out] count    number written (may be 0)
 * \return 0 on success, -1 on invalid input
 */
int hc_torah_reading_for_day(hc_date *date, hc_custom custom, int in_israel,
                             hc_torah_reading results[HC_MAX_TORAH_READINGS],
                             int *count);

/*! How many scrolls are taken out for this reading (1, 2 or 3). */
int hc_torah_reading_sefarim(const hc_torah_reading *r);

/*! \name ABI introspection — see hc_haftarah.h */
/**@{*/
int hc_torah_sizeof_span(void);
int hc_torah_sizeof_reading(void);
int hc_torah_offsetof_aliyot(void);
int hc_torah_offsetof_aliyot_count(void);
/**@}*/

#endif /* SRC_HC_TORAH_READING_H_ */
