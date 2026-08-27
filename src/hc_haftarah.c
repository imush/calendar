#include "hc_haftarah.h"
#include "hc_jewish_dates.h"
#include "parshiot.h"

#include <stddef.h>
#include <string.h>

/* ── ABI introspection (see header) ──────────────────────────────────────── */

int hc_haftarah_sizeof_ref(void)          { return (int)sizeof(hc_haftarah_ref); }
int hc_haftarah_sizeof_result(void)       { return (int)sizeof(hc_haftarah_result); }
int hc_haftarah_offsetof_refs(void)       { return (int)offsetof(hc_haftarah_result, refs); }
int hc_haftarah_offsetof_refs_count(void) { return (int)offsetof(hc_haftarah_result, refs_count); }

/* ── occasion names ──────────────────────────────────────────────────────── */

static const char *const OCCASION_NAMES[HC_HAFT_OCC_COUNT] = {
    [HC_HAFT_OCC_WEEKLY]               = "Weekly",
    [HC_HAFT_OCC_PARSHAT_SHEKALIM]     = "Parshat Shekalim",
    [HC_HAFT_OCC_PARSHAT_ZACHOR]       = "Parshat Zachor",
    [HC_HAFT_OCC_PARSHAT_PARAH]        = "Parshat Parah",
    [HC_HAFT_OCC_PARSHAT_HACHODESH]    = "Parshat Hachodesh",
    [HC_HAFT_OCC_SHABBAT_HAGADOL]      = "Shabbat Hagadol",
    [HC_HAFT_OCC_CHANUKAH_SHABBAT_1]   = "Chanukah",
    [HC_HAFT_OCC_CHANUKAH_SHABBAT_2]   = "Chanukah",
    [HC_HAFT_OCC_ROSH_CHODESH]         = "Rosh Chodesh",
    [HC_HAFT_OCC_MACHAR_CHODESH]       = "Machar Chodesh",
    [HC_HAFT_OCC_ROSH_HASHANA]         = "Rosh Hashana",
    [HC_HAFT_OCC_YOM_KIPPUR]           = "Yom Kippur",
    [HC_HAFT_OCC_YOM_KIPPUR_AFTERNOON] = "Yom Kippur (afternoon)",
    [HC_HAFT_OCC_SUKKOT]               = "Sukkot",
    [HC_HAFT_OCC_SHMINI_ATZERET]       = "Shemini Atzeret",
    [HC_HAFT_OCC_SIMCHAT_TORAH]        = "Simchat Torah",
    [HC_HAFT_OCC_PESACH]               = "Pesach",
    [HC_HAFT_OCC_SHAVUOT]              = "Shavuot",
    [HC_HAFT_OCC_CHOL_HAMOED_PESACH]   = "Chol HaMoed Pesach",
    [HC_HAFT_OCC_CHOL_HAMOED_SUKKOT]   = "Chol HaMoed Sukkot",
    [HC_HAFT_OCC_TISHA_BAV]            = "Tisha B'Av",
    [HC_HAFT_OCC_TISHA_BAV_AFTERNOON]  = "Tisha B'Av (afternoon)",
    [HC_HAFT_OCC_FAST_AFTERNOON]       = "Fast day (afternoon)",
};

const char *hc_haftarah_occasion_name(hc_haftarah_occasion occ)
{
    if (occ < 0 || occ >= HC_HAFT_OCC_COUNT) return NULL;
    return OCCASION_NAMES[occ];
}

/* ── data lookup helpers ─────────────────────────────────────────────────── */

/* Copy a span list into a result. Returns 1 if anything was written. A
 * zero-length span means "this custom has no reading here" (opentorah
 * genuinely leaves some fast-day haftarot undefined for Sefard/Teiman). */
static int fill(const hc_haftarah_spans *s, hc_haftarah_occasion occ,
                hc_haftarah_result *out)
{
    if (!s || !s->refs || s->refs_count <= 0) return 0;
    int n = s->refs_count;
    if (n > HC_MAX_HAFTARAH_REFS) n = HC_MAX_HAFTARAH_REFS;
    out->occasion   = occ;
    out->refs_count = n;
    memcpy(out->refs, s->refs, (size_t)n * sizeof(hc_haftarah_ref));
    return 1;
}

/* Look up an "Occasion_VARIANT" entry from the special-haftarot table. */
static int special(const char *key, hc_custom custom,
                   hc_haftarah_occasion occ, hc_haftarah_result *out)
{
    const hc_special_haftarah *e = hc_special_haftarah_lookup(key);
    if (!e) return 0;
    return fill(&e->customs[custom], occ, out);
}

/* Weekly parsha haftarah. */
static int weekly(hc_parsha p, hc_custom custom, hc_haftarah_result *out)
{
    if (p <= HC_PARSHA_NONE || p >= HC_PARSHA_COUNT) return 0;
    return fill(&HC_HAFTAROT_WEEKLY[p][custom], HC_HAFT_OCC_WEEKLY, out);
}

/*
 * Append an "addition" entry onto an already-resolved reading, leaving
 * the occasion alone — the extra verses don't change what the reading is.
 * Most customs have no addition defined, in which case this is a no-op.
 */
static int append(const char *key, hc_custom custom, hc_haftarah_result *out)
{
    const hc_special_haftarah *e = hc_special_haftarah_lookup(key);
    if (!e) return 0;
    const hc_haftarah_spans *s = &e->customs[custom];
    if (!s->refs || s->refs_count <= 0) return 0;

    int room = HC_MAX_HAFTARAH_REFS - out->refs_count;
    int n = s->refs_count < room ? s->refs_count : room;
    if (n <= 0) return 0;
    memcpy(out->refs + out->refs_count, s->refs, (size_t)n * sizeof(hc_haftarah_ref));
    out->refs_count += n;
    return 1;
}

/* ── date helpers ────────────────────────────────────────────────────────── */

/* hc_get_day_of_week: 0 = Saturday, 1 = Sunday, ... 6 = Friday. */
static int is_shabbat(hc_date *d) { return hc_get_day_of_week(d) == 0; }

/*
 * Month whose Rosh Chodesh falls on this Hebrew date, or -1.
 * Day 30 is the first day of a two-day Rosh Chodesh, belonging to the
 * *next* month. month+1 is always in range: the only 30-day months are
 * followed by a real month (Elul and both Adars are 29 days).
 */
static int rosh_chodesh_month(const hc_date *heb)
{
    if (heb->day == 1)  return heb->month;
    if (heb->day == 30) return heb->month + 1;
    return -1;
}

/* Hebrew month numbers used by the Rosh Chodesh rules. */
#define HEB_AV       5
#define HEB_ELUL     6
#define HEB_TISHREI  7
#define HEB_TEVES   10

static int is_chol_hamoed_pesach(hc_special_day d)
{
    return d >= HC_SD_CHOL_HAMOED_PESACH_1_I && d <= HC_SD_CHOL_HAMOED_PESACH_4_C;
}

static int is_chol_hamoed_sukkot(hc_special_day d)
{
    return d >= HC_SD_CHOL_HAMOED_SUKKOT_1_I && d <= HC_SD_CHOL_HAMOED_SUKKOT_4_C;
}

/*
 * Yom Tov days that own a haftarah of their own, mapped to their
 * SpecialHaftarot key. Returns NULL for anything else. Simchat Torah is
 * handled separately: its haftarah is Vezot HaBracha's, not a special
 * entry.
 */
static const char *festival_key(hc_special_day d, hc_haftarah_occasion *occ)
{
    switch (d) {
        case HC_SD_ROSH_HASHANA_1:   *occ = HC_HAFT_OCC_ROSH_HASHANA;   return "RoshHashanah1_MAIN";
        case HC_SD_ROSH_HASHANA_2:   *occ = HC_HAFT_OCC_ROSH_HASHANA;   return "RoshHashanah2_MAIN";
        case HC_SD_YOM_KIPPUR:       *occ = HC_HAFT_OCC_YOM_KIPPUR;     return "YomKippur_MAIN";
        case HC_SD_SUKKOT_1:         *occ = HC_HAFT_OCC_SUKKOT;         return "Succos1_MAIN";
        case HC_SD_SUKKOT_2_C:       *occ = HC_HAFT_OCC_SUKKOT;         return "Succos2_MAIN";
        case HC_SD_SHMINI_ATZERET_C: *occ = HC_HAFT_OCC_SHMINI_ATZERET; return "SheminiAtzeres_MAIN";
        case HC_SD_PESACH_1:         *occ = HC_HAFT_OCC_PESACH;         return "Pesach1_MAIN";
        case HC_SD_PESACH_2_C:       *occ = HC_HAFT_OCC_PESACH;         return "Pesach2_MAIN";
        case HC_SD_PESACH_7:         *occ = HC_HAFT_OCC_PESACH;         return "Pesach7_MAIN";
        case HC_SD_PESACH_LAST_C:    *occ = HC_HAFT_OCC_PESACH;         return "Pesach8_MAIN";
        case HC_SD_SHAVUOT:          *occ = HC_HAFT_OCC_SHAVUOT;        return "Shavuos1_MAIN";
        case HC_SD_SHAVUOT_2_C:      *occ = HC_HAFT_OCC_SHAVUOT;        return "Shavuos2_MAIN";
        default: return NULL;
    }
}

/* Simchat Torah reads Vezot HaBracha, hence its haftarah too. In Israel
 * that is 22 Tishrei (Shmini Atzeret); in the Diaspora, 23 Tishrei. */
static int is_simchat_torah(hc_special_day d)
{
    return d == HC_SD_SIMCHAT_TORAH_I || d == HC_SD_SIMCHAT_TORAH_C;
}

/*
 * Fast-day afternoon haftarah. Tzom Gedalia overrides the default for a
 * couple of customs (opentorah: FastOfGedalia.afternoonHaftarahExceptions),
 * so try the exception table first.
 */
static int fast_afternoon(hc_custom custom, int is_gedalia,
                          hc_haftarah_occasion occ, hc_haftarah_result *out)
{
    if (is_gedalia &&
        special("FastOfGedalia_AFTERNOON_EXCEPTIONS", custom, occ, out))
        return 1;
    return special("Fast_AFTERNOON_DEFAULT", custom, occ, out);
}

/* ── hc_haftarah_for_date ────────────────────────────────────────────────── */

int hc_haftarah_for_date(hc_date *date, hc_custom custom, int in_israel,
                         hc_haftarah_result *out)
{
    if (!date || !out) return -1;
    if (custom < 0 || custom >= HC_CUSTOM_COUNT) return -1;

    /* Roll forward to the current or upcoming Shabbat. */
    hc_date shabbat = *date;
    if (shabbat.calendar_type != HEBREW && hc_convert(&shabbat, HEBREW) != 0)
        return -1;
    int dow = (int)hc_get_day_of_week(&shabbat); /* 0 = Sat */
    if (dow != 0 && hc_date_add_days(&shabbat, 7 - dow) != 0) return -1;

    hc_date next = shabbat;
    if (hc_date_add_days(&next, 1) != 0) return -1;

    hc_special_day days[HC_MAX_SPECIAL_DAYS];
    int n = 0;
    if (hc_get_special_days(&shabbat, in_israel, days, &n) != 0) return -1;

    int chanukah_night = 0;
    int shekalim = 0, zachor = 0, parah = 0, hachodesh = 0;
    int hagadol = 0, erev_pesach = 0, chm_pesach = 0, chm_sukkot = 0;
    int simchat_torah = 0;
    hc_special_day festival = HC_SD_NONE;

    for (int i = 0; i < n; i++) {
        hc_special_day d = days[i];
        switch (d) {
            case HC_SD_SHABBAT_SHEKALIM:  shekalim    = 1; break;
            case HC_SD_SHABBAT_ZACHOR:    zachor      = 1; break;
            case HC_SD_SHABBAT_PARA:      parah       = 1; break;
            case HC_SD_SHABBAT_HACHODESH: hachodesh   = 1; break;
            case HC_SD_SHABBAT_HAGADOL:   hagadol     = 1; break;
            case HC_SD_EREV_PESACH:       erev_pesach = 1; break;
            default:
                if (is_simchat_torah(d))         simchat_torah = 1;
                else if (hc_sd_is_chanukah(d))   chanukah_night = hc_sd_chanukah_night(d);
                else if (is_chol_hamoed_pesach(d)) chm_pesach  = 1;
                else if (is_chol_hamoed_sukkot(d)) chm_sukkot  = 1;
                else {
                    hc_haftarah_occasion ignored;
                    if (festival == HC_SD_NONE && festival_key(d, &ignored))
                        festival = d;
                }
                break;
        }
    }

    /*
     * opentorah's SpecialShabbos: the four parshiyot plus Shabbos
     * Hagadol. Note this is a property of the *day*, not of which branch
     * below fires — Chabad keeps the weekly haftarah on Shabbos Hagadol
     * but the day is still a special Shabbat for the Rosh Chodesh rules.
     */
    int is_special_shabbos = shekalim || zachor || parah || hachodesh || hagadol;

    /* ── Base reading ────────────────────────────────────────────────── */
    int have = 0;

    if (simchat_torah && weekly(HC_VEZOT_HABRACHA, custom, out)) {
        out->occasion = HC_HAFT_OCC_SIMCHAT_TORAH;
        have = 1;
    }
    if (!have && festival != HC_SD_NONE) {
        hc_haftarah_occasion occ;
        const char *key = festival_key(festival, &occ);
        if (key) have = special(key, custom, occ, out);
    }
    if (!have && chm_pesach)
        have = special("PesachIntermediate_SHABBAT", custom,
                       HC_HAFT_OCC_CHOL_HAMOED_PESACH, out);
    if (!have && chm_sukkot)
        have = special("SuccosIntermediate_SHABBAT", custom,
                       HC_HAFT_OCC_CHOL_HAMOED_SUKKOT, out);

    /* Arba Parshiyot */
    if (!have && shekalim)  have = special("ParshasShekalim_MAIN",  custom, HC_HAFT_OCC_PARSHAT_SHEKALIM,  out);
    if (!have && zachor)    have = special("ParshasZachor_MAIN",    custom, HC_HAFT_OCC_PARSHAT_ZACHOR,    out);
    if (!have && parah)     have = special("ParshasParah_MAIN",     custom, HC_HAFT_OCC_PARSHAT_PARAH,     out);
    if (!have && hachodesh) have = special("ParshasHachodesh_MAIN", custom, HC_HAFT_OCC_PARSHAT_HACHODESH, out);

    /* Shabbat Hagadol — Chabad keeps the weekly haftarah unless the day
     * is also Erev Pesach. */
    if (!have && hagadol && (custom != HC_CUSTOM_CHABAD || erev_pesach))
        have = special("ShabbosHagodol_MAIN", custom, HC_HAFT_OCC_SHABBAT_HAGADOL, out);

    /* Chanukah. opentorah splits on the day number, not the parsha:
     * `if dayNumber < 8 then shabbos1Haftarah else shabbos2Haftarah`.
     * Night 8 is only ever a Shabbat when 25 Kislev was itself a
     * Shabbat — i.e. exactly the years with two Chanukah Shabbatot. */
    if (!have && chanukah_night > 0) {
        int second = (chanukah_night == 8);
        have = special(second ? "Chanukah_SHABBAT_2" : "Chanukah_SHABBAT_1", custom,
                       second ? HC_HAFT_OCC_CHANUKAH_SHABBAT_2
                              : HC_HAFT_OCC_CHANUKAH_SHABBAT_1, out);
    }

    /* Weekly parsha; a combined week follows the second parsha. */
    if (!have) {
        hc_reading reading = { HC_PARSHA_NONE, HC_PARSHA_NONE };
        hc_get_parsha(&shabbat, in_israel, &reading);
        hc_parsha target = (reading.p2 != HC_PARSHA_NONE) ? reading.p2 : reading.p1;
        have = weekly(target, custom, out);
    }
    if (!have) return -1;

    /*
     * ── Rosh Chodesh / Machar Chodesh corrections ────────────────────
     *
     * These are post-corrections applied to whatever was resolved above,
     * mirroring opentorah's RoshChodesh.correct / ErevRoshChodesh.correct.
     * Each is a replace-or-add decision: when the Rosh Chodesh haftarah
     * is allowed to displace the base reading it replaces it outright;
     * when it isn't, Chabad (and Fes, for Machar Chodesh) still append a
     * few verses of it to whatever is being read instead.
     */
    int rc = rosh_chodesh_month(&shabbat);
    int mc = rosh_chodesh_month(&next);

    /* Rosh Chodesh Tishrei is Rosh Hashana — never mentioned as Rosh Chodesh. */
    if (rc > 0 && rc != HEB_TISHREI) {
        /* Teves is always Chanukah and Av is always the Three Weeks, so in
         * both the day's own haftarah outranks Rosh Chodesh. */
        int allow_replace = !is_special_shabbos && rc != HEB_TEVES && rc != HEB_AV;
        /* In Elul the Shiva d'Nechemta hold their ground — except for
         * Chabad, who read the Rosh Chodesh haftarah. */
        if (allow_replace && (rc != HEB_ELUL || custom == HC_CUSTOM_CHABAD))
            special("RoshChodesh_SHABBAT", custom, HC_HAFT_OCC_ROSH_CHODESH, out);
        else
            append("RoshChodesh_SHABBAT_ADDITION", custom, out);
    }

    if (mc > 0 && mc != HEB_TISHREI) {
        /* A Shabbat that is itself Rosh Chodesh reads the Rosh Chodesh
         * haftarah, not Machar Chodesh. */
        int allow_replace = !is_special_shabbos && rc <= 0 &&
                            mc != HEB_TEVES && mc != HEB_AV && mc != HEB_ELUL;
        /* Fes never replaces — it always takes the addition instead. */
        if (allow_replace && custom != HC_CUSTOM_FES)
            special("ErevRoshChodesh_SHABBAT", custom, HC_HAFT_OCC_MACHAR_CHODESH, out);
        else
            append("ErevRoshChodesh_SHABBAT_ADDITION", custom, out);
    }

    return 0;
}

/* ── hc_haftarah_for_day ─────────────────────────────────────────────────── */

int hc_haftarah_for_day(hc_date *date, hc_custom custom, int in_israel,
                        hc_haftarah_result results[HC_MAX_HAFTARAH_RESULTS],
                        int *count)
{
    if (!date || !results || !count) return -1;
    if (custom < 0 || custom >= HC_CUSTOM_COUNT) return -1;
    *count = 0;

    hc_date d = *date;
    if (d.calendar_type != HEBREW && hc_convert(&d, HEBREW) != 0) return -1;

    /* On Shabbat the day's reading is exactly the upcoming-Shabbat one. */
    if (is_shabbat(&d)) {
        if (hc_haftarah_for_date(&d, custom, in_israel, &results[0]) == 0)
            *count = 1;
        return 0;
    }

    hc_special_day days[HC_MAX_SPECIAL_DAYS];
    int n = 0;
    if (hc_get_special_days(&d, in_israel, days, &n) != 0) return -1;

    int yom_kippur = 0, tisha_bav = 0, gedalia = 0, other_fast = 0;
    int simchat_torah = 0;
    hc_special_day festival = HC_SD_NONE;

    for (int i = 0; i < n; i++) {
        hc_special_day sd = days[i];
        if (sd == HC_SD_YOM_KIPPUR)          { yom_kippur = 1; continue; }
        if (sd == HC_SD_FAST_9_AV)           { tisha_bav  = 1; continue; }
        if (sd == HC_SD_TZOM_GEDALIA)        { gedalia    = 1; continue; }
        if (is_simchat_torah(sd))            { simchat_torah = 1; continue; }
        hc_haftarah_occasion ignored;
        if (festival == HC_SD_NONE && festival_key(sd, &ignored)) { festival = sd; continue; }
        if (hc_sd_is_fast(sd))                 other_fast = 1;
    }

    if (yom_kippur) {
        if (special("YomKippur_MAIN", custom, HC_HAFT_OCC_YOM_KIPPUR, &results[*count]))
            (*count)++;
        if (special("YomKippur_AFTERNOON", custom, HC_HAFT_OCC_YOM_KIPPUR_AFTERNOON, &results[*count]))
            (*count)++;
        return 0;
    }
    if (tisha_bav) {
        if (special("TishaBeAv_MAIN", custom, HC_HAFT_OCC_TISHA_BAV, &results[*count]))
            (*count)++;
        if (fast_afternoon(custom, 0, HC_HAFT_OCC_TISHA_BAV_AFTERNOON, &results[*count]))
            (*count)++;
        return 0;
    }
    if (simchat_torah) {
        if (weekly(HC_VEZOT_HABRACHA, custom, &results[0])) {
            results[0].occasion = HC_HAFT_OCC_SIMCHAT_TORAH;
            *count = 1;
        }
        return 0;
    }
    if (festival != HC_SD_NONE) {
        hc_haftarah_occasion occ;
        const char *key = festival_key(festival, &occ);
        if (key && special(key, custom, occ, &results[0])) *count = 1;
        return 0;
    }
    if (gedalia || other_fast) {
        if (fast_afternoon(custom, gedalia, HC_HAFT_OCC_FAST_AFTERNOON, &results[0]))
            *count = 1;
        return 0;
    }

    return 0;
}
