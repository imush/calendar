/* Haftarah resolution: precedence rules over the hebrewcalendar-data tables. */
#include "hc_test.h"
#include "../src/hc_haftarah.h"
#include "../src/hc_jewish_dates.h"
#include "../src/hconverter.h"

/* ── helpers ─────────────────────────────────────────────────────────────── */

static hc_date greg(int y, int m, int d)
{
    hc_date dt; dt.calendar_type = GREGORIAN;
    dt.year = y; dt.month = m; dt.day = d;
    return dt;
}

/* Assert that the single reading on `date` has the expected occasion and a
 * first reference matching book/chapter/verse. */
static void expect_day(int y, int m, int d, hc_custom custom, int in_israel,
                       hc_haftarah_occasion occ, hc_tanach_book book,
                       int from_ch, int from_v, int to_ch, int to_v)
{
    hc_date dt = greg(y, m, d);
    hc_haftarah_result rs[HC_MAX_HAFTARAH_RESULTS];
    int n = 0;
    HC_ASSERT_EQ_INT(0, hc_haftarah_for_day(&dt, custom, in_israel, rs, &n));
    if (n < 1) { HC_ASSERT(0, "expected at least one reading"); return; }
    HC_ASSERT_EQ_INT((int)occ,      (int)rs[0].occasion);
    HC_ASSERT_EQ_INT((int)book,     (int)rs[0].refs[0].book);
    HC_ASSERT_EQ_INT(from_ch,       rs[0].refs[0].from_ch);
    HC_ASSERT_EQ_INT(from_v,        rs[0].refs[0].from_v);
    HC_ASSERT_EQ_INT(to_ch,         rs[0].refs[0].to_ch);
    HC_ASSERT_EQ_INT(to_v,          rs[0].refs[0].to_v);
}

static int day_count(int y, int m, int d, hc_custom custom, int in_israel)
{
    hc_date dt = greg(y, m, d);
    hc_haftarah_result rs[HC_MAX_HAFTARAH_RESULTS];
    int n = 0;
    hc_haftarah_for_day(&dt, custom, in_israel, rs, &n);
    return n;
}

/* ── weekly parsha haftarot ──────────────────────────────────────────────── */

static void test_weekly(void)
{
    /* Bereishit 5786 (Diaspora): Isaiah 42:5-43:10 Ashkenaz,
     * 42:5-42:21 Sefard, 42:1-42:16 Teiman. */
    expect_day(2025, 10, 18, HC_CUSTOM_ASHKENAZ, 0,
               HC_HAFT_OCC_WEEKLY, HC_BOOK_ISAIAH, 42, 5, 43, 10);
    expect_day(2025, 10, 18, HC_CUSTOM_SEFARD, 0,
               HC_HAFT_OCC_WEEKLY, HC_BOOK_ISAIAH, 42, 5, 42, 21);
    expect_day(2025, 10, 18, HC_CUSTOM_TEIMAN, 0,
               HC_HAFT_OCC_WEEKLY, HC_BOOK_ISAIAH, 42, 1, 42, 16);

    /* Shmini 5786: II Samuel 6:1-7:17. */
    expect_day(2026, 4, 11, HC_CUSTOM_ASHKENAZ, 0,
               HC_HAFT_OCC_WEEKLY, HC_BOOK_II_SAMUEL, 6, 1, 7, 17);

    /* Haazinu 5786: II Samuel 22 (the Shirah). */
    expect_day(2025, 10, 4, HC_CUSTOM_ASHKENAZ, 0,
               HC_HAFT_OCC_WEEKLY, HC_BOOK_II_SAMUEL, 22, 1, 22, 51);
}

/* ── special Shabbatot ───────────────────────────────────────────────────── */

static void test_special_shabbatot(void)
{
    /* Arba Parshiyot, 5786. */
    expect_day(2026, 2, 14, HC_CUSTOM_ASHKENAZ, 0,
               HC_HAFT_OCC_PARSHAT_SHEKALIM,  HC_BOOK_II_KINGS,  12,  1, 12, 17);
    expect_day(2026, 2, 28, HC_CUSTOM_ASHKENAZ, 0,
               HC_HAFT_OCC_PARSHAT_ZACHOR,    HC_BOOK_I_SAMUEL,  15,  2, 15, 34);
    expect_day(2026, 3,  7, HC_CUSTOM_ASHKENAZ, 0,
               HC_HAFT_OCC_PARSHAT_PARAH,     HC_BOOK_EZEKIEL,   36, 16, 36, 38);
    expect_day(2026, 3, 14, HC_CUSTOM_ASHKENAZ, 0,
               HC_HAFT_OCC_PARSHAT_HACHODESH, HC_BOOK_EZEKIEL,   45, 16, 46, 18);

    /* Shabbat Hagadol 5786: Malachi 3:4-24. */
    expect_day(2026, 3, 28, HC_CUSTOM_ASHKENAZ, 0,
               HC_HAFT_OCC_SHABBAT_HAGADOL, HC_BOOK_MALACHI, 3, 4, 3, 24);

    /* Chabad keeps the weekly haftarah on Shabbat Hagadol unless the day is
     * also Erev Pesach. 2026-03-28 is 10 Nisan, so Chabad reads Tzav. */
    expect_day(2026, 3, 28, HC_CUSTOM_CHABAD, 0,
               HC_HAFT_OCC_WEEKLY, HC_BOOK_JEREMIAH, 7, 21, 7, 28);
}

/* ── Rosh Chodesh / Machar Chodesh ───────────────────────────────────────── */

static void test_rosh_chodesh(void)
{
    /* 1 Iyar 5786 falls on Shabbat: Isaiah 66 (+ the repeated last verse). */
    expect_day(2026, 4, 18, HC_CUSTOM_ASHKENAZ, 0,
               HC_HAFT_OCC_ROSH_CHODESH, HC_BOOK_ISAIAH, 66, 1, 66, 24);

    /* 29 Iyar 5786 is Shabbat, 1 Sivan is Sunday → Machar Chodesh. */
    expect_day(2026, 5, 16, HC_CUSTOM_ASHKENAZ, 0,
               HC_HAFT_OCC_MACHAR_CHODESH, HC_BOOK_I_SAMUEL, 20, 18, 20, 42);

    /* 1 Nisan is always Parshat Hachodesh when it falls on Shabbat, and a
     * special Shabbat blocks the Rosh Chodesh replace. 5782: 2022-04-02. */
    expect_day(2022, 4, 2, HC_CUSTOM_ASHKENAZ, 0,
               HC_HAFT_OCC_PARSHAT_HACHODESH, HC_BOOK_EZEKIEL, 45, 16, 46, 18);
}

/* ── Rosh Chodesh / Machar Chodesh replace-vs-add ────────────────────────── */

/* Compare a whole reading against an expected "Book c:v-c:v;..." rendering. */
static void expect_refs(int y, int m, int d, hc_custom custom, int in_israel,
                        const char *expected)
{
    hc_date dt = greg(y, m, d);
    hc_haftarah_result r;
    char buf[512];
    int n = 0;
    HC_ASSERT_EQ_INT(0, hc_haftarah_for_date(&dt, custom, in_israel, &r));
    buf[0] = '\0';
    for (int i = 0; i < r.refs_count; i++)
        n += snprintf(buf + n, sizeof(buf) - (size_t)n, "%s %d:%d-%d:%d;",
                      hc_tanach_book_name(r.refs[i].book),
                      r.refs[i].from_ch, r.refs[i].from_v,
                      r.refs[i].to_ch,   r.refs[i].to_v);
    hc_tests_run++;
    if (strcmp(buf, expected) == 0) {
        hc_tests_passed++;
    } else {
        hc_tests_failed++;
        fprintf(stderr, "FAIL [%s:%d] %04d-%02d-%02d custom=%d\n  expected %s\n  got      %s\n",
                __FILE__, __LINE__, y, m, d, (int)custom, expected, buf);
    }
}

static void test_rosh_chodesh_corrections(void)
{
    /*
     * Shabbat Rosh Chodesh Elul 5775 (2015-08-15, parshat Re'eh). The
     * Shiva d'Nechemta hold their ground for everyone except Chabad,
     * who read the Rosh Chodesh haftarah. Both then pick up the Machar
     * Chodesh addition, because 1 Elul falls the next day.
     */
    expect_refs(2015, 8, 15, HC_CUSTOM_ASHKENAZ, 0, "Isaiah 54:11-55:5;");
    expect_refs(2015, 8, 15, HC_CUSTOM_CHABAD,   0,
                "Isaiah 66:1-66:24;Isaiah 66:23-66:23;"
                "I Samuel 20:18-20:18;I Samuel 20:42-20:42;");
    /* Fes never replaces for Machar Chodesh — it only ever appends. */
    expect_refs(2015, 8, 15, HC_CUSTOM_FES, 0,
                "Isaiah 54:11-55:5;I Samuel 20:18-20:18;I Samuel 20:42-20:42;");

    /*
     * A special Shabbat blocks the replace, so Chabad appends the Rosh
     * Chodesh verses to the special-parsha haftarah instead. 2015-03-21
     * is Parshat Hachodesh falling on 1 Nisan.
     */
    expect_refs(2015, 3, 21, HC_CUSTOM_ASHKENAZ, 0, "Ezekiel 45:16-46:18;");
    expect_refs(2015, 3, 21, HC_CUSTOM_CHABAD,   0,
                "Ezekiel 45:18-46:15;Isaiah 66:1-66:1;"
                "Isaiah 66:23-66:24;Isaiah 66:23-66:23;");

    /*
     * Worst case for HC_MAX_HAFTARAH_REFS: 30 Kislev 5776 (2015-12-12) is
     * Shabbat Chanukah, Rosh Chodesh Teves and Erev Rosh Chodesh at once.
     * Chanukah owns the reading; Teves blocks the Rosh Chodesh replace and
     * being Rosh Chodesh blocks the Machar Chodesh one, so Chabad appends
     * both additions — 6 refs.
     */
    expect_refs(2015, 12, 12, HC_CUSTOM_CHABAD, 0,
                "Zechariah 2:14-4:7;"
                "Isaiah 66:1-66:1;Isaiah 66:23-66:24;Isaiah 66:23-66:23;"
                "I Samuel 20:18-20:18;I Samuel 20:42-20:42;");
    expect_refs(2015, 12, 12, HC_CUSTOM_ASHKENAZ, 0, "Zechariah 2:14-4:7;");

    /* Machar Chodesh Elul is likewise suppressed (2021-08-07 = 29 Av). */
    expect_refs(2021, 8, 7, HC_CUSTOM_ASHKENAZ, 0, "Isaiah 54:11-55:5;");
    expect_refs(2021, 8, 7, HC_CUSTOM_CHABAD,   0,
                "Isaiah 54:11-55:5;I Samuel 20:18-20:18;I Samuel 20:42-20:42;");

    /* Rosh Chodesh Tishrei is Rosh Hashana — never mentioned, no addition. */
    hc_date rh = greg(2026, 9, 12);            /* 1 Tishrei 5787, a Shabbat */
    hc_haftarah_result r;
    HC_ASSERT_EQ_INT(0, hc_haftarah_for_date(&rh, HC_CUSTOM_CHABAD, 0, &r));
    HC_ASSERT_EQ_INT((int)HC_HAFT_OCC_ROSH_HASHANA, (int)r.occasion);
    HC_ASSERT_EQ_INT(1, r.refs_count);
}

/* ── Chanukah ────────────────────────────────────────────────────────────── */

static void test_chanukah(void)
{
    /* 5786: a single Chanukah Shabbat (30 Kislev) → Zechariah. */
    expect_day(2025, 12, 20, HC_CUSTOM_ASHKENAZ, 0,
               HC_HAFT_OCC_CHANUKAH_SHABBAT_1, HC_BOOK_ZECHARIAH, 2, 14, 4, 7);

    /*
     * opentorah splits on the Chanukah day number, not the parsha:
     * `if dayNumber < 8 then shabbos1Haftarah else shabbos2Haftarah`.
     * Two Chanukah Shabbatot occur exactly when 25 Kislev is itself a
     * Shabbat; the eighth day is then also a Shabbat and reads I Kings 7.
     * Scan a century for such years and check both Shabbatot.
     */
    int two_shabbat_years = 0;
    for (int hy = 5750; hy < 5850; hy++) {
        hc_date k25; k25.calendar_type = HEBREW;
        k25.year = hy; k25.month = 9; k25.day = 25;
        if (hc_get_day_of_week(&k25) != 0) continue;   /* not a Saturday */
        two_shabbat_years++;

        hc_date k32 = k25;
        hc_date_add_days(&k32, 7);                     /* eighth day */

        hc_haftarah_result r1, r8;
        HC_ASSERT_EQ_INT(0, hc_haftarah_for_date(&k25, HC_CUSTOM_ASHKENAZ, 0, &r1));
        HC_ASSERT_EQ_INT(0, hc_haftarah_for_date(&k32, HC_CUSTOM_ASHKENAZ, 0, &r8));
        HC_ASSERT_EQ_INT((int)HC_HAFT_OCC_CHANUKAH_SHABBAT_1, (int)r1.occasion);
        HC_ASSERT_EQ_INT((int)HC_BOOK_ZECHARIAH, (int)r1.refs[0].book);
        HC_ASSERT_EQ_INT((int)HC_HAFT_OCC_CHANUKAH_SHABBAT_2, (int)r8.occasion);
        HC_ASSERT_EQ_INT((int)HC_BOOK_I_KINGS,   (int)r8.refs[0].book);
        HC_ASSERT_EQ_INT(7,  r8.refs[0].from_ch);
        HC_ASSERT_EQ_INT(40, r8.refs[0].from_v);
    }
    HC_ASSERT_TRUE(two_shabbat_years > 0);
}

/* ── Yom Tov, fasts, and days with no haftarah ───────────────────────────── */

static void test_festivals_and_fasts(void)
{
    /* Yom Kippur 5787: morning Isaiah 57:14, afternoon Jonah + Micah. */
    HC_ASSERT_EQ_INT(2, day_count(2026, 9, 21, HC_CUSTOM_ASHKENAZ, 0));
    expect_day(2026, 9, 21, HC_CUSTOM_ASHKENAZ, 0,
               HC_HAFT_OCC_YOM_KIPPUR, HC_BOOK_ISAIAH, 57, 14, 58, 14);

    /* Tisha B'Av 5786: morning Jeremiah 8:13, afternoon the fast default. */
    HC_ASSERT_EQ_INT(2, day_count(2026, 7, 23, HC_CUSTOM_ASHKENAZ, 0));
    expect_day(2026, 7, 23, HC_CUSTOM_ASHKENAZ, 0,
               HC_HAFT_OCC_TISHA_BAV, HC_BOOK_JEREMIAH, 8, 13, 9, 23);

    /* Simchat Torah reads Vezot HaBracha's haftarah — Joshua 1. In the
     * Diaspora that is 23 Tishrei; in Israel, 22 Tishrei. */
    expect_day(2026, 10, 4, HC_CUSTOM_ASHKENAZ, 0,
               HC_HAFT_OCC_SIMCHAT_TORAH, HC_BOOK_JOSHUA, 1, 1, 1, 18);
    expect_day(2026, 10, 3, HC_CUSTOM_ASHKENAZ, 1,
               HC_HAFT_OCC_SIMCHAT_TORAH, HC_BOOK_JOSHUA, 1, 1, 1, 18);
    /* Diaspora keeps Shemini Atzeret separate on 22 Tishrei. */
    expect_day(2026, 10, 3, HC_CUSTOM_ASHKENAZ, 0,
               HC_HAFT_OCC_SHMINI_ATZERET, HC_BOOK_I_KINGS, 8, 54, 8, 66);

    /* Tzom Gedalia mincha. opentorah hangs its exception on Morocco, so Fes
     * inherits it; hebrewcalendar-data narrows it to Marrakesh, which is
     * the custom that actually keeps Hosea + Joel. Morocco and Fes read the
     * same default as Ashkenaz. */
    expect_day(2026, 9, 14, HC_CUSTOM_ASHKENAZ, 0,
               HC_HAFT_OCC_FAST_AFTERNOON, HC_BOOK_ISAIAH, 55, 6, 56, 8);
    expect_day(2026, 9, 14, HC_CUSTOM_MOROCCO, 0,
               HC_HAFT_OCC_FAST_AFTERNOON, HC_BOOK_ISAIAH, 55, 6, 56, 8);
    expect_day(2026, 9, 14, HC_CUSTOM_FES, 0,
               HC_HAFT_OCC_FAST_AFTERNOON, HC_BOOK_ISAIAH, 55, 6, 56, 8);
    expect_day(2026, 9, 14, HC_CUSTOM_MARRAKESH, 0,
               HC_HAFT_OCC_FAST_AFTERNOON, HC_BOOK_HOSEA, 14, 2, 14, 10);
    /* ...and on any other fast Marrakesh is back on the default. */
    expect_day(2026, 7, 2, HC_CUSTOM_MARRAKESH, 0,
               HC_HAFT_OCC_FAST_AFTERNOON, HC_BOOK_ISAIAH, 55, 6, 56, 8);
    expect_day(2026, 7, 2, HC_CUSTOM_MOROCCO, 0,
               HC_HAFT_OCC_FAST_AFTERNOON, HC_BOOK_ISAIAH, 55, 6, 56, 8);

    /* opentorah leaves the fast-day haftarah undefined for Sefard and
     * Teiman — those customs read no haftarah at mincha. */
    HC_ASSERT_EQ_INT(0, day_count(2026, 7, 2, HC_CUSTOM_SEFARD, 0));
    HC_ASSERT_EQ_INT(0, day_count(2026, 7, 2, HC_CUSTOM_TEIMAN, 0));

    /* Ordinary weekday and weekday Chol HaMoed: no haftarah at all. */
    HC_ASSERT_EQ_INT(0, day_count(2026, 4, 15, HC_CUSTOM_ASHKENAZ, 0));
    HC_ASSERT_EQ_INT(0, day_count(2026, 6, 10, HC_CUSTOM_ASHKENAZ, 0));
}

/* ── exhaustive scan ─────────────────────────────────────────────────────── */

/*
 * Every Shabbat of every year, for every custom and both locations, must
 * resolve to a non-empty reading whose refs stay inside the fixed-size
 * result buffer. This is the guard against a data table growing a longer
 * multi-part haftarah than HC_MAX_HAFTARAH_REFS, or a precedence branch
 * falling through to nothing.
 */
static void test_every_shabbat_resolves(void)
{
    int failures = 0, checked = 0, max_refs = 0, max_special = 0;

    for (int hy = 5780; hy < 5820; hy++) {
        hc_date d; d.calendar_type = HEBREW;
        d.year = hy; d.month = 7; d.day = 1;
        /* Roll to the first Shabbat of the year. */
        int dow = (int)hc_get_day_of_week(&d);
        if (dow != 0) hc_date_add_days(&d, 7 - dow);

        for (int week = 0; week < 55; week++, hc_date_add_days(&d, 7)) {
            if (d.calendar_type == HEBREW && d.year > hy) break;

            hc_special_day sds[HC_MAX_SPECIAL_DAYS];
            int sn = 0;
            for (int israel = 0; israel <= 1; israel++) {
                hc_get_special_days(&d, israel, sds, &sn);
                if (sn > max_special) max_special = sn;

                for (int c = 0; c < HC_CUSTOM_COUNT; c++) {
                    hc_haftarah_result r;
                    checked++;
                    if (hc_haftarah_for_date(&d, (hc_custom)c, israel, &r) != 0 ||
                        r.refs_count < 1) {
                        if (failures < 10)
                            fprintf(stderr, "  no haftarah: heb %d-%d-%d custom=%d israel=%d\n",
                                    d.year, d.month, d.day, c, israel);
                        failures++;
                        continue;
                    }
                    if (r.refs_count > max_refs) max_refs = r.refs_count;
                    for (int i = 0; i < r.refs_count; i++) {
                        if (r.refs[i].book <= HC_BOOK_NONE ||
                            r.refs[i].book >= HC_BOOK_COUNT) failures++;
                    }
                }
            }
        }
    }

    HC_ASSERT_EQ_INT(0, failures);
    /* 40 years × ~51 Shabbatot × 2 locations × 18 customs ≈ 73k. */
    HC_ASSERT_TRUE(checked > 70000);
    /* Headroom checks: if either of these ever hits its cap, the
     * corresponding #define in hc_haftarah.h / hc_jewish_dates.h is
     * silently truncating data. */
    HC_ASSERT_TRUE(max_refs     <  HC_MAX_HAFTARAH_REFS);
    HC_ASSERT_TRUE(max_special  <  HC_MAX_SPECIAL_DAYS);
}

/* ── suite entry point ───────────────────────────────────────────────────── */

/* ── Shabbat Shuvah and the Shabbat before Rosh Hashanah ─────────────────── */

/*
 * upstream keeps the Shabbat Shuvah haftarah in Vayeilech's weekly slot. That
 * lands correctly only in the years when Vayeilech falls on Shabbat Shuvah; in
 * the others Vayeilech is folded into Nitzavim-Vayeilech and read a week
 * BEFORE Rosh Hashanah. Both weeks are resolved explicitly, so pin down both
 * shapes of year.
 */
static void test_shabbat_shuvah(void)
{
    /* ── A "Vayeilech year": Vayeilech itself falls on Shabbat Shuvah ── */

    /* 27 Elul 5785 — Nitzavim alone, the Shabbat before Rosh Hashanah.
       Isaiah 61:10-63:9 is the seventh haftarah of consolation. */
    expect_day(2025, 9, 20, HC_CUSTOM_ASHKENAZ, 0,
               HC_HAFT_OCC_SHABBAT_BEFORE_ROSH_HASHANA,
               HC_BOOK_ISAIAH, 61, 10, 63, 9);

    /* 5 Tishrei 5786 — Shabbat Shuvah, reading Vayeilech. */
    expect_day(2025, 9, 27, HC_CUSTOM_ASHKENAZ, 0,
               HC_HAFT_OCC_SHABBAT_SHUVAH, HC_BOOK_HOSEA, 14, 2, 14, 10);

    /* 12 Tishrei 5786 — Haazinu, now AFTER Yom Kippur, keeps its own
       haftarah (the song of David). This is the only week it belongs to. */
    expect_day(2025, 10, 4, HC_CUSTOM_ASHKENAZ, 0,
               HC_HAFT_OCC_WEEKLY, HC_BOOK_II_SAMUEL, 22, 1, 22, 51);

    /* ── A "Haazinu year": the pair is combined and Haazinu falls on
          Shabbat Shuvah. This is the case upstream gets wrong. ── */

    /* 23 Elul 5786 — Nitzavim-Vayeilech combined, before Rosh Hashanah.
       The combined-week rule would take Vayeilech's (= Shabbat Shuvah's)
       haftarah; it must take Nitzavim's instead. */
    expect_day(2026, 9, 5, HC_CUSTOM_ASHKENAZ, 0,
               HC_HAFT_OCC_SHABBAT_BEFORE_ROSH_HASHANA,
               HC_BOOK_ISAIAH, 61, 10, 63, 9);

    /* 8 Tishrei 5787 — Shabbat Shuvah, reading Haazinu. Without the
       occasion this fell through to II Samuel 22. */
    expect_day(2026, 9, 19, HC_CUSTOM_ASHKENAZ, 0,
               HC_HAFT_OCC_SHABBAT_SHUVAH, HC_BOOK_HOSEA, 14, 2, 14, 10);

    /* Both weeks hold across the custom tree, not just Ashkenaz. */
    const hc_custom customs[] = { HC_CUSTOM_CHABAD, HC_CUSTOM_SEFARD,
                                  HC_CUSTOM_TEIMAN, HC_CUSTOM_ITALKI };
    for (unsigned i = 0; i < sizeof customs / sizeof customs[0]; i++) {
        hc_date before = greg(2026, 9, 5), shuvah = greg(2026, 9, 19);
        hc_haftarah_result r;
        HC_ASSERT_EQ_INT(0, hc_haftarah_for_date(&before, customs[i], 0, &r));
        HC_ASSERT_EQ_INT((int)HC_HAFT_OCC_SHABBAT_BEFORE_ROSH_HASHANA, (int)r.occasion);
        HC_ASSERT_EQ_INT(0, hc_haftarah_for_date(&shuvah, customs[i], 0, &r));
        HC_ASSERT_EQ_INT((int)HC_HAFT_OCC_SHABBAT_SHUVAH, (int)r.occasion);
    }

    /*
     * Over 40 years: every year has exactly one Shabbat Shuvah and exactly
     * one Shabbat before Rosh Hashanah, and Nitzavim's haftarah is never
     * skipped — the failure that motivated this.
     */
    for (int y = 5786; y < 5826; y++) {
        int shuvah = 0, before = 0;
        hc_date d; set_hc_date(&d, y - 1, 6, 1, HEBREW);   /* 1 Elul */
        for (int w = 0; w < 12; w++) {
            hc_date c = d;
            if (hc_get_day_of_week(&c) != 0) { hc_date_add_days(&d, 1); w--; continue; }
            hc_date h = c; hc_convert(&h, HEBREW);
            if (h.year > y || (h.year == y && h.month == 7 && h.day > 12)) break;
            hc_haftarah_result r;
            if (hc_haftarah_for_date(&c, HC_CUSTOM_ASHKENAZ, 0, &r) == 0) {
                if (r.occasion == HC_HAFT_OCC_SHABBAT_SHUVAH) shuvah++;
                if (r.occasion == HC_HAFT_OCC_SHABBAT_BEFORE_ROSH_HASHANA) before++;
            }
            hc_date_add_days(&d, 7);
        }
        HC_ASSERT_EQ_INT(1, shuvah);
        HC_ASSERT_EQ_INT(1, before);
    }
}

void test_haftarah(void)
{
    test_weekly();
    test_special_shabbatot();
    test_rosh_chodesh();
    test_rosh_chodesh_corrections();
    test_chanukah();
    test_festivals_and_fasts();
    test_every_shabbat_resolves();
    test_shabbat_shuvah();
}
