/* Smoke tests for hc_compute_zmanim.
   Exact times vary by algorithm precision; we test structure and ordering. */
#include "hc_test.h"
#include "../src/zmanim.h"
#include "../src/hconverter.h"
#include <math.h>

#define AVAIL(t) ((t) != HC_ZMAN_UNAVAILABLE)

void test_zmanim(void)
{
    /* New York: lat=40.67, lon=-73.94, UTC-5 (EST), not Jerusalem */
    hc_date ny_date;
    ny_date.calendar_type = GREGORIAN;
    ny_date.year = 2026; ny_date.month = 5; ny_date.day = 29; /* Friday May 29, 2026 */

    hc_zmanim z;
    int rc = hc_compute_zmanim(&ny_date, 40.67, -73.94, -5.0, 0, 0, &z);
    HC_ASSERT_EQ_INT(0, rc);

    /* All standard times should be available at this latitude in May */
    HC_ASSERT_TRUE(AVAIL(z.alot_hashachar));
    HC_ASSERT_TRUE(AVAIL(z.misheyakir));
    HC_ASSERT_TRUE(AVAIL(z.hanetz));
    HC_ASSERT_TRUE(AVAIL(z.hanetz_amiti));
    HC_ASSERT_TRUE(AVAIL(z.sof_shma));
    HC_ASSERT_TRUE(AVAIL(z.sof_tfila));
    HC_ASSERT_TRUE(AVAIL(z.chatzot));
    HC_ASSERT_TRUE(AVAIL(z.shkiah_amitis));
    HC_ASSERT_TRUE(AVAIL(z.shkiah));
    HC_ASSERT_TRUE(AVAIL(z.tzait_3_stars));
    HC_ASSERT_TRUE(AVAIL(z.tzait_alter_rebbe));
    HC_ASSERT_TRUE(AVAIL(z.tzait_rt));
    HC_ASSERT_TRUE(z.sha_ah_zmanit_sec > 0);

    /* Ordering invariants */
    HC_ASSERT_TRUE(z.alot_hashachar   < z.misheyakir);
    HC_ASSERT_TRUE(z.misheyakir       < z.hanetz);
    HC_ASSERT_TRUE(z.hanetz           < z.hanetz_amiti  || fabs(z.hanetz - z.hanetz_amiti) < 5);
    HC_ASSERT_TRUE(z.sof_shma         < z.sof_tfila);
    HC_ASSERT_TRUE(z.sof_tfila        < z.chatzot);
    HC_ASSERT_TRUE(z.chatzot          < z.mincha_gedola);
    HC_ASSERT_TRUE(z.mincha_gedola    < z.mincha_ketana);
    HC_ASSERT_TRUE(z.mincha_ketana    < z.plag_hamincha);
    HC_ASSERT_TRUE(z.plag_hamincha    < z.shkiah);
    HC_ASSERT_TRUE(z.shkiah           < z.tzait_3_stars);
    HC_ASSERT_TRUE(z.tzait_3_stars    < z.tzait_alter_rebbe);
    HC_ASSERT_TRUE(z.tzait_alter_rebbe< z.tzait_rt);

    /* May 29 is Friday → tomorrow (May 30) is Shabbat → candle lighting should be set */
    HC_ASSERT_TRUE(AVAIL(z.candle_lighting));
    HC_ASSERT_TRUE(z.candle_lighting  < z.shkiah);  /* 18 min before sunset */

    /* Jerusalem with longer candle lighting (40 min) */
    hc_zmanim zj;
    hc_date jer_date;
    jer_date.calendar_type = GREGORIAN;
    jer_date.year = 2026; jer_date.month = 5; jer_date.day = 29;
    hc_compute_zmanim(&jer_date, 31.78, 35.22, 3.0, 1, 1, &zj);
    /* Jerusalem candle lighting should be ~22 min earlier than New York's relative gap */
    HC_ASSERT_TRUE(AVAIL(zj.candle_lighting));
    /* 40-min gap: shkiah - candle_lighting ≈ 40 */
    double gap = zj.shkiah - zj.candle_lighting;
    HC_ASSERT_TRUE(gap > 35 && gap < 45);

    /* Non-Gregorian input should fail */
    hc_date heb_date;
    heb_date.calendar_type = HEBREW;
    heb_date.year = 5786; heb_date.month = 1; heb_date.day = 15;
    hc_zmanim z2;
    HC_ASSERT_EQ_INT(-1, hc_compute_zmanim(&heb_date, 40.0, -74.0, -5.0, 0, 0, &z2));

    /* sha'ah zmanit: roughly 3600 sec in a mid-latitude spring day */
    HC_ASSERT_TRUE(z.sha_ah_zmanit_sec > 3200 && z.sha_ah_zmanit_sec < 4500);

    /* ── Additional alot / misheyakir / tzet opinions available in NY ── */
    HC_ASSERT_TRUE(AVAIL(z.alot_rav_naeh));
    HC_ASSERT_TRUE(AVAIL(z.alot_sbh));
    HC_ASSERT_TRUE(AVAIL(z.alot_gra));
    HC_ASSERT_TRUE(AVAIL(z.misheyakir_sbh));
    HC_ASSERT_TRUE(AVAIL(z.misheyakir_nivreshet));
    HC_ASSERT_TRUE(AVAIL(z.tzait_melamed));

    /* Ordering — larger depression angle → earlier morning, later evening */
    HC_ASSERT_TRUE(z.alot_rav_naeh          < z.alot_sbh);
    HC_ASSERT_TRUE(z.alot_sbh               < z.alot_hashachar);
    HC_ASSERT_TRUE(z.alot_hashachar         < z.alot_gra);
    HC_ASSERT_TRUE(z.alot_gra               < z.misheyakir_nivreshet);
    HC_ASSERT_TRUE(z.misheyakir_nivreshet   < z.misheyakir_sbh);
    HC_ASSERT_TRUE(z.misheyakir_sbh         < z.misheyakir);
    HC_ASSERT_TRUE(z.tzait_3_stars          < z.tzait_melamed);
    HC_ASSERT_TRUE(z.tzait_melamed          < z.tzait_alter_rebbe);

    /* ── GR"A and MA portion-of-day zmanim ─────────────────────────────── */
    HC_ASSERT_TRUE(AVAIL(z.sof_shma_gra));
    HC_ASSERT_TRUE(AVAIL(z.sof_shma_ma));
    HC_ASSERT_EQ_INT(0, z.ma_polar_fallback);

    /* MA starts earliest (Alot anchor), Chabad and GRA close */
    HC_ASSERT_TRUE(z.sof_shma_ma            < z.sof_shma);
    HC_ASSERT_TRUE(z.sof_shma_ma            < z.sof_shma_gra);
    /* Each method: shema < shacharis < mincha gedola < ketana < plag */
    HC_ASSERT_TRUE(z.sof_shma_gra           < z.sof_tfila_gra);
    HC_ASSERT_TRUE(z.sof_tfila_gra          < z.mincha_gedola_gra);
    HC_ASSERT_TRUE(z.mincha_gedola_gra      < z.mincha_ketana_gra);
    HC_ASSERT_TRUE(z.mincha_ketana_gra      < z.plag_hamincha_gra);
    HC_ASSERT_TRUE(z.sof_shma_ma            < z.sof_tfila_ma);
    HC_ASSERT_TRUE(z.plag_hamincha_ma       > z.plag_hamincha_gra);

    /* sha'ah lengths differ predictably: MA (longer day) > Chabad > GRA (slightly) */
    HC_ASSERT_TRUE(z.sha_ah_zmanit_sec_ma   > z.sha_ah_zmanit_sec);
    HC_ASSERT_TRUE(z.sha_ah_zmanit_sec_gra  > 3200 && z.sha_ah_zmanit_sec_gra < 4500);

    /* ── Polar Magen Avraham fallback (Reykjavik summer solstice) ────── */
    hc_zmanim zp;
    hc_date rk_date;
    rk_date.calendar_type = GREGORIAN;
    rk_date.year = 2026; rk_date.month = 6; rk_date.day = 21;
    hc_compute_zmanim(&rk_date, 64.1466, -21.9426, 0.0, 0, 0, &zp);
    /* Alot GRA (−16.1°) unreachable near solstice at 64°N */
    HC_ASSERT_EQ_INT(1, zp.ma_polar_fallback);
    HC_ASSERT_TRUE(zp.alot_gra == HC_ZMAN_UNAVAILABLE);
    /* sha'ah = 2h exact = 7200s */
    HC_ASSERT_TRUE(fabs(zp.sha_ah_zmanit_sec_ma - 7200.0) < 1.0);
    /* All MA portion-of-day zmanim available via the fallback */
    HC_ASSERT_TRUE(AVAIL(zp.sof_shma_ma));
    HC_ASSERT_TRUE(AVAIL(zp.plag_hamincha_ma));
    HC_ASSERT_TRUE(zp.sof_shma_ma           < zp.sof_tfila_ma);
    HC_ASSERT_TRUE(zp.mincha_gedola_ma      < zp.mincha_ketana_ma);
}
