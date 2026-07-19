/* Port of MonthDaySpecialDayTest and related holiday tests */
#include "hc_test.h"
#include "../src/hc_jewish_dates.h"
#include "../src/hconverter.h"

static int has_day(hc_special_day days[HC_MAX_SPECIAL_DAYS], int count, hc_special_day want)
{
    for (int i = 0; i < count; i++)
        if (days[i] == want) return 1;
    return 0;
}

static void get_days(int hy, int hm, int hd, int in_israel,
                     hc_special_day days[HC_MAX_SPECIAL_DAYS], int *count)
{
    hc_date dt; dt.calendar_type = HEBREW;
    dt.year = hy; dt.month = hm; dt.day = hd;
    *count = 0;
    hc_get_special_days(&dt, in_israel, days, count);
}

void test_jewish_dates(void)
{
    hc_special_day days[HC_MAX_SPECIAL_DAYS];
    int count;

    /* Rosh Hashana */
    get_days(5777, 7, 1, 0, days, &count);
    HC_ASSERT_TRUE(has_day(days, count, HC_SD_ROSH_HASHANA_1));
    HC_ASSERT_TRUE(hc_sd_is_yom_tov(HC_SD_ROSH_HASHANA_1));

    get_days(5777, 7, 2, 0, days, &count);
    HC_ASSERT_TRUE(has_day(days, count, HC_SD_ROSH_HASHANA_2));

    /* Yom Kippur */
    get_days(5777, 7, 10, 0, days, &count);
    HC_ASSERT_TRUE(has_day(days, count, HC_SD_YOM_KIPPUR));
    HC_ASSERT_TRUE(hc_sd_is_yom_tov(HC_SD_YOM_KIPPUR));

    /* Erev Yom Kippur */
    get_days(5777, 7, 9, 0, days, &count);
    HC_ASSERT_TRUE(has_day(days, count, HC_SD_EREV_YOM_KIPPUR));

    /* Sukkot */
    get_days(5777, 7, 15, 0, days, &count);
    HC_ASSERT_TRUE(has_day(days, count, HC_SD_SUKKOT_1));

    /* Chol Hamoed Sukkot (diaspora: day 17=CHM1) */
    get_days(5777, 7, 17, 0, days, &count);
    HC_ASSERT_TRUE(has_day(days, count, HC_SD_CHOL_HAMOED_SUKKOT_1_C));
    HC_ASSERT_TRUE(hc_sd_is_chol_hamoed(HC_SD_CHOL_HAMOED_SUKKOT_1_C));

    /* Hoshana Rabba */
    get_days(5777, 7, 21, 0, days, &count);
    HC_ASSERT_TRUE(has_day(days, count, HC_SD_HOSHANA_RABBA));

    /* Shmini Atzeret (diaspora) */
    get_days(5777, 7, 22, 0, days, &count);
    HC_ASSERT_TRUE(has_day(days, count, HC_SD_SHMINI_ATZERET_C));
    HC_ASSERT_FALSE(has_day(days, count, HC_SD_SIMCHAT_TORAH_I));

    /* Simchat Torah Israel */
    get_days(5777, 7, 22, 1, days, &count);
    HC_ASSERT_TRUE(has_day(days, count, HC_SD_SIMCHAT_TORAH_I));
    HC_ASSERT_FALSE(has_day(days, count, HC_SD_SHMINI_ATZERET_C));

    /* Chanukah: Kislev 25 = day 1 */
    get_days(5777, 9, 25, 0, days, &count);
    HC_ASSERT_TRUE(has_day(days, count, HC_SD_CHANUKAH_1));
    HC_ASSERT_TRUE(hc_sd_is_chanukah(HC_SD_CHANUKAH_1));

    get_days(5777, 9, 32, 0, days, &count); /* Kislev 32 = invalid, skip */
    /* Use Tevet 1 for Chanukah day that spills */
    /* Tevet 2 = Chanukah day 7 (Kislev is short in 5777); also Tal Umatar active */
    get_days(5777, 10, 2, 0, days, &count);
    HC_ASSERT_TRUE(has_day(days, count, HC_SD_CHANUKAH_7));

    /* 10 Tevet */
    get_days(5777, 10, 10, 0, days, &count);
    HC_ASSERT_TRUE(has_day(days, count, HC_SD_TENTH_TEVET));
    HC_ASSERT_TRUE(hc_sd_is_fast(HC_SD_TENTH_TEVET));

    /* Purim (non-leap: Adar=12, month 12) */
    HC_ASSERT_FALSE(hc_is_leap_year(5777, HEBREW));
    get_days(5777, 12, 14, 0, days, &count);
    HC_ASSERT_TRUE(has_day(days, count, HC_SD_PURIM));

    /* Purim in leap year: Adar II = month 13 */
    HC_ASSERT_TRUE(hc_is_leap_year(5779, HEBREW));
    get_days(5779, 13, 14, 0, days, &count);
    HC_ASSERT_TRUE(has_day(days, count, HC_SD_PURIM));
    /* Purim Katan = 14 Adar I in leap */
    get_days(5779, 12, 14, 0, days, &count);
    HC_ASSERT_TRUE(has_day(days, count, HC_SD_PURIM_KATAN));

    /* Pesach */
    get_days(5777, 1, 15, 0, days, &count);
    HC_ASSERT_TRUE(has_day(days, count, HC_SD_PESACH_1));
    HC_ASSERT_TRUE(hc_sd_is_yom_tov(HC_SD_PESACH_1));

    /* Pesach 2 (diaspora only) */
    get_days(5777, 1, 16, 0, days, &count);
    HC_ASSERT_TRUE(has_day(days, count, HC_SD_PESACH_2_C));
    HC_ASSERT_TRUE(hc_sd_applies(HC_SD_PESACH_2_C, 0));
    HC_ASSERT_FALSE(hc_sd_applies(HC_SD_PESACH_2_C, 1));

    /* Shavuot */
    get_days(5777, 3, 6, 0, days, &count);
    HC_ASSERT_TRUE(has_day(days, count, HC_SD_SHAVUOT));
    HC_ASSERT_TRUE(hc_sd_is_yom_tov(HC_SD_SHAVUOT));

    /* Fast of 9 Av */
    get_days(5777, 5, 9, 0, days, &count);
    HC_ASSERT_TRUE(has_day(days, count, HC_SD_FAST_9_AV));
    HC_ASSERT_TRUE(hc_sd_is_fast(HC_SD_FAST_9_AV));

    /* Rosh Chodesh (Iyar 1 = day 1 of Iyar) */
    get_days(5777, 2, 1, 0, days, &count);
    HC_ASSERT_TRUE(has_day(days, count, HC_SD_ROSH_CHODESH));
    /* Rosh Chodesh Iyar 30 (30th of Nisan = 1st of Iyar) */
    get_days(5777, 1, 30, 0, days, &count);
    HC_ASSERT_TRUE(has_day(days, count, HC_SD_ROSH_CHODESH));

    /* Chabad days */
    get_days(5777, 9, 19, 0, days, &count);
    HC_ASSERT_TRUE(has_day(days, count, HC_SD_NINETEENTH_KISLEV));
    HC_ASSERT_TRUE(hc_sd_is_chabad(HC_SD_NINETEENTH_KISLEV));

    get_days(5777, 4, 3, 0, days, &count);
    HC_ASSERT_TRUE(has_day(days, count, HC_SD_TAMUZ_3));
    HC_ASSERT_TRUE(hc_sd_is_chabad(HC_SD_TAMUZ_3));

    /* hc_sd_applies: Israel-only does not apply in diaspora */
    HC_ASSERT_FALSE(hc_sd_applies(HC_SD_SIMCHAT_TORAH_I, 0));
    HC_ASSERT_TRUE (hc_sd_applies(HC_SD_SIMCHAT_TORAH_I, 1));

    /* hc_sd_name */
    HC_ASSERT_TRUE(hc_sd_name(HC_SD_PESACH_1) != 0);
    HC_ASSERT_TRUE(hc_sd_name(HC_SD_NONE) == 0);

    /* 19 Kislev name */
    HC_ASSERT_TRUE(strcmp(hc_sd_name(HC_SD_NINETEENTH_KISLEV), "19 Kislev") == 0);

    /* ── Named Shabbatot ─────────────────────────────────────────────────── */

    /* Year 5777 (Sep 2016 – Sep 2017) — non-leap; Rosh Hashana on Monday,
       9 Av on Tuesday, Beshalach falls on 15 Shvat. */

    /* Shabbat Shuvah: 6 Tishrei 5777 (Sat Oct 8, 2016) */
    get_days(5777, 7, 6, 0, days, &count);
    HC_ASSERT_TRUE(has_day(days, count, HC_SD_SHABBAT_SHUVAH));
    HC_ASSERT_TRUE(hc_sd_is_named_shabbat(HC_SD_SHABBAT_SHUVAH));
    /* Not Shuvah the Shabbat before Rosh Hashana */
    get_days(5776, 6, 24, 0, days, &count);
    HC_ASSERT_FALSE(has_day(days, count, HC_SD_SHABBAT_SHUVAH));

    /* Shabbat Chazon: 6 Av 5777 (Sat Jul 29, 2017) */
    get_days(5777, 5, 6, 0, days, &count);
    HC_ASSERT_TRUE(has_day(days, count, HC_SD_SHABBAT_CHAZON));
    HC_ASSERT_FALSE(has_day(days, count, HC_SD_SHABBAT_NACHAMU));
    /* Shabbat Nachamu: 13 Av 5777 (Sat Aug 5, 2017) */
    get_days(5777, 5, 13, 0, days, &count);
    HC_ASSERT_TRUE(has_day(days, count, HC_SD_SHABBAT_NACHAMU));
    HC_ASSERT_FALSE(has_day(days, count, HC_SD_SHABBAT_CHAZON));
    HC_ASSERT_TRUE(hc_sd_is_named_shabbat(HC_SD_SHABBAT_NACHAMU));
    /* 7 days past 9 Av is the boundary; 20 Av is not Nachamu */
    get_days(5777, 5, 20, 0, days, &count);
    HC_ASSERT_FALSE(has_day(days, count, HC_SD_SHABBAT_NACHAMU));

    /* Shabbat Shirah: 15 Shvat 5777 (Sat Feb 11, 2017) — Beshalach */
    get_days(5777, 11, 15, 0, days, &count);
    HC_ASSERT_TRUE(has_day(days, count, HC_SD_SHABBAT_SHIRAH));
    HC_ASSERT_TRUE(hc_sd_is_named_shabbat(HC_SD_SHABBAT_SHIRAH));
    /* The previous Shabbat (8 Shvat = Bo) is not Shirah */
    get_days(5777, 11, 8, 0, days, &count);
    HC_ASSERT_FALSE(has_day(days, count, HC_SD_SHABBAT_SHIRAH));

    /* Shabbat Hagadol: 12 Nisan 5777 (Sat Apr 8, 2017) */
    get_days(5777, 1, 12, 0, days, &count);
    HC_ASSERT_TRUE(has_day(days, count, HC_SD_SHABBAT_HAGADOL));
    HC_ASSERT_TRUE(hc_sd_is_named_shabbat(HC_SD_SHABBAT_HAGADOL));

    /* Arba Parshiyot classified as named Shabbatot */
    HC_ASSERT_TRUE(hc_sd_is_named_shabbat(HC_SD_SHABBAT_SHEKALIM));
    HC_ASSERT_TRUE(hc_sd_is_named_shabbat(HC_SD_SHABBAT_ZACHOR));
    HC_ASSERT_TRUE(hc_sd_is_named_shabbat(HC_SD_SHABBAT_PARA));
    HC_ASSERT_TRUE(hc_sd_is_named_shabbat(HC_SD_SHABBAT_HACHODESH));
    /* Shabbat Mevarchim is NOT a named Shabbat (rendered separately) */
    HC_ASSERT_FALSE(hc_sd_is_named_shabbat(HC_SD_SHABBAT_MEVARCHIM));

    /* Names */
    HC_ASSERT_TRUE(strcmp(hc_sd_name(HC_SD_SHABBAT_NACHAMU), "Shabbat Nachamu") == 0);
    HC_ASSERT_TRUE(strcmp(hc_sd_name(HC_SD_SHABBAT_SHUVAH),  "Shabbat Shuvah")  == 0);
    HC_ASSERT_TRUE(strcmp(hc_sd_name(HC_SD_SHABBAT_SHIRAH),  "Shabbat Shirah")  == 0);
}
