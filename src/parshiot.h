/*!
 * \file parshiot.h
 * \brief Weekly Torah reading (parsha) lookup for any Shabbat.
 */

#ifndef SRC_PARSHIOT_H_
#define SRC_PARSHIOT_H_

#include "hconverter.h"

/*!
 * The 54 weekly Torah portions (0 = none / Yom Tov placeholder).
 */
typedef enum hc_parsha {
    HC_PARSHA_NONE = 0,
    HC_BEREISHIT, HC_NOACH, HC_LECH_LECHA, HC_VAYERA, HC_CHAYEI_SARAH,
    HC_TOLDOT, HC_VAYETZE, HC_VAYISHLACH, HC_VAYESHEV, HC_MIKETZ,
    HC_VAYIGASH, HC_VAYECHI, HC_SHEMOT, HC_VAERA, HC_BO, HC_BESHALACH,
    HC_YITRO, HC_MISHPATIM, HC_TERUMAH, HC_TETZAVEH, HC_KI_TISA,
    HC_VAYAKHEL, HC_PEKUDEI, HC_VAYIKRA, HC_TZAV, HC_SHEMINI,
    HC_TAZRIA, HC_METZORA, HC_ACHAREI_MOT, HC_KEDOSHIM, HC_EMOR,
    HC_BEHAR, HC_BECHUKOTAI, HC_BAMIDBAR, HC_NASO, HC_BEHAALOTECHA,
    HC_SHELACH, HC_KORACH, HC_CHUKAT, HC_BALAK, HC_PINCHAS,
    HC_MATOT, HC_MASEI, HC_DEVARIM, HC_VAETCHANAN, HC_EIKEV,
    HC_REEH, HC_SHOFTIM, HC_KI_TEITZEI, HC_KI_TAVO, HC_NITZAVIM,
    HC_VAYEILECH, HC_HAAZINU,
    HC_PARSHA_COUNT
} hc_parsha;

/*!
 * One weekly reading slot: up to two parshiyot (double parsha).
 * p2 == HC_PARSHA_NONE means single parsha.
 * p1 == HC_PARSHA_NONE means Yom Tov / Chol Hamoed (no regular reading).
 */
typedef struct {
    hc_parsha p1;
    hc_parsha p2;
} hc_reading;

/*!
 * \brief Get the Torah reading for a given Shabbat date.
 *
 * \param[in]  date       must be a Saturday (getDayOfWeek == 7 in Hebrew convention,
 *                        or abs_day % 7 == 0); pass as any calendar type.
 * \param[in]  in_israel  1 for Eretz Israel schedule, 0 for Diaspora.
 * \param[out] reading    populated with the reading (p1==HC_PARSHA_NONE → Yom Tov).
 * \return 0 on success, -1 if date is not a Saturday or is out of range.
 */
int hc_get_parsha(hc_date *date, int in_israel, hc_reading *reading);

/*!
 * English name of a parsha (NULL for HC_PARSHA_NONE).
 */
const char *hc_parsha_name(hc_parsha p);

#endif /* SRC_PARSHIOT_H_ */
