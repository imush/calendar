#include "hc_torah_reading.h"
#include "hc_jewish_dates.h"
#include "parshiot.h"

#include <stddef.h>
#include <string.h>

/* ── spans ───────────────────────────────────────────────────────────── */

static hc_torah_span span_of(hc_special_torah which, int i)
{
    return HC_SPECIAL_TORAH[which][i];
}

/*! Consecutive fragments read as one span. */
static hc_torah_span join(hc_torah_span a, hc_torah_span b)
{
    hc_torah_span out = a;
    out.to_ch = b.to_ch;
    out.to_v  = b.to_v;
    return out;
}

/*! A whole recorded reading joined end to end -- for a maftir, which is one
 *  span however many fragments it was written in. */
static hc_torah_span joined(hc_special_torah which)
{
    int n = HC_SPECIAL_TORAH_LEN[which];
    return join(span_of(which, 0), span_of(which, n - 1));
}

/*! Copy a recorded reading's fragments in as the aliyot. */
static void take(hc_torah_reading *r, hc_special_torah which)
{
    int n = HC_SPECIAL_TORAH_LEN[which];
    for (int i = 0; i < n; i++) r->aliyot[i] = span_of(which, i);
    r->aliyot_count = n;
}

/*!
 * Fold the aliyot numbered in `into` (1-based) into the ones before them.
 * Fewer are read on a weekday than on a Shabbat, and this is how one recorded
 * division serves both.
 */
static void merge(hc_torah_reading *r, const int *into, int n_into)
{
    hc_torah_span out[HC_MAX_ALIYOT];
    int n = 0;
    for (int i = 0; i < r->aliyot_count; i++) {
        int drop = 0;
        for (int k = 0; k < n_into; k++) if (into[k] == i + 1) drop = 1;
        if (drop && n > 0) out[n - 1] = join(out[n - 1], r->aliyot[i]);
        else               out[n++]   = r->aliyot[i];
    }
    memcpy(r->aliyot, out, (size_t)n * sizeof(out[0]));
    r->aliyot_count = n;
}

static void reset(hc_torah_reading *r, hc_reading_slot slot)
{
    memset(r, 0, sizeof(*r));
    r->slot         = slot;
    r->maftir.book  = HC_BOOK_NONE;
    r->parshiyot[0] = HC_PARSHA_NONE;
    r->parshiyot[1] = HC_PARSHA_NONE;
    r->division     = HC_CUSTOM_COUNT;
    for (int i = 0; i < HC_MAX_ALIYOT; i++) r->aliyot_sefer[i] = 1;
}

/*! Whether this custom reads with Sefard where Sefard and Ashkenaz divide a
 *  reading differently. Chabad hangs off Sefard in the tree but goes with
 *  Ashkenaz in these places, so it is asked about first. */
static int reads_sefard(hc_custom c)
{
    return !hc_custom_is_under(c, HC_CUSTOM_CHABAD)
        &&  hc_custom_is_under(c, HC_CUSTOM_SEFARD);
}

int hc_torah_reading_sefarim(const hc_torah_reading *r)
{
    int most = r->maftir_sefer;
    for (int i = 0; i < r->aliyot_count; i++)
        if (r->aliyot_sefer[i] > most) most = r->aliyot_sefer[i];
    return most ? most : 1;
}

/* ── what the day is, as far as the Torah reading is concerned ───────── */

typedef struct {
    hc_special_day festival;   /* the day whose reading replaces the parsha */
    int rosh_chodesh, yom_kippur, fast, shushan_purim;
    int sukkot_intermediate;   /* opentorah's intermediate day number, or 0 */
    int pesach_day;            /* the day of Pesach on chol hamoed, or 0 */
    int chanukah_day;          /* 1..8, or 0 */
    hc_special_torah parsha_maftir;   /* one of the four, or HC_ST_COUNT */
} occasion;

static void scan(hc_date *date, int in_israel, occasion *o)
{
    hc_special_day days[HC_MAX_SPECIAL_DAYS];
    int n = 0;
    memset(o, 0, sizeof(*o));
    o->festival      = HC_SD_NONE;
    o->parsha_maftir = HC_ST_COUNT;
    if (hc_get_special_days(date, in_israel, days, &n) != 0) return;

    for (int i = 0; i < n; i++) {
        hc_special_day d = days[i];
        if (!hc_sd_applies(d, in_israel)) continue;
        switch (d) {
        case HC_SD_ROSH_CHODESH:     o->rosh_chodesh = 1; break;
        case HC_SD_SHUSHAN_PURIM:    o->shushan_purim = 1; o->festival = d; break;
        case HC_SD_SHABBAT_SHEKALIM:  o->parsha_maftir = HC_ST_PARSHAS_SHEKALIM_MAFTIR;  break;
        case HC_SD_SHABBAT_ZACHOR:    o->parsha_maftir = HC_ST_PARSHAS_ZACHOR_MAFTIR;    break;
        case HC_SD_SHABBAT_PARA:      o->parsha_maftir = HC_ST_PARSHAS_PARAH_MAFTIR;     break;
        case HC_SD_SHABBAT_HACHODESH: o->parsha_maftir = HC_ST_PARSHAS_HACHODESH_MAFTIR; break;

        case HC_SD_YOM_KIPPUR:       o->yom_kippur = 1; o->festival = d; break;
        case HC_SD_TZOM_GEDALIA: case HC_SD_TENTH_TEVET: case HC_SD_TAANIT_ESTHER:
        case HC_SD_FAST_17_TAMUZ: case HC_SD_FAST_9_AV:
            o->fast = 1; o->festival = d; break;

        /* Chol HaMoed is numbered from the first intermediate day, which is
         * the second of the festival in Israel and the third outside it. */
        case HC_SD_CHOL_HAMOED_SUKKOT_1_I: case HC_SD_CHOL_HAMOED_SUKKOT_1_C: o->sukkot_intermediate = 1; break;
        case HC_SD_CHOL_HAMOED_SUKKOT_2_I: case HC_SD_CHOL_HAMOED_SUKKOT_2_C: o->sukkot_intermediate = 2; break;
        case HC_SD_CHOL_HAMOED_SUKKOT_3_I: case HC_SD_CHOL_HAMOED_SUKKOT_3_C: o->sukkot_intermediate = 3; break;
        case HC_SD_CHOL_HAMOED_SUKKOT_4_I: case HC_SD_CHOL_HAMOED_SUKKOT_4_C: o->sukkot_intermediate = 4; break;
        case HC_SD_CHOL_HAMOED_SUKKOT_5_I: o->sukkot_intermediate = 5; break;
        case HC_SD_HOSHANA_RABBA:          o->sukkot_intermediate = in_israel ? 6 : 5; break;

        case HC_SD_CHOL_HAMOED_PESACH_1_I: o->pesach_day = 2; break;
        case HC_SD_CHOL_HAMOED_PESACH_2_I: o->pesach_day = 3; break;
        case HC_SD_CHOL_HAMOED_PESACH_3_I: o->pesach_day = 4; break;
        case HC_SD_CHOL_HAMOED_PESACH_4_I: o->pesach_day = 5; break;
        case HC_SD_CHOL_HAMOED_PESACH_5_I: o->pesach_day = 6; break;
        case HC_SD_CHOL_HAMOED_PESACH_1_C: o->pesach_day = 3; break;
        case HC_SD_CHOL_HAMOED_PESACH_2_C: o->pesach_day = 4; break;
        case HC_SD_CHOL_HAMOED_PESACH_3_C: o->pesach_day = 5; break;
        case HC_SD_CHOL_HAMOED_PESACH_4_C: o->pesach_day = 6; break;

        case HC_SD_ROSH_HASHANA_1: case HC_SD_ROSH_HASHANA_2:
        case HC_SD_SUKKOT_1: case HC_SD_SUKKOT_2_C:
        case HC_SD_SHMINI_ATZERET_C: case HC_SD_SIMCHAT_TORAH_C: case HC_SD_SIMCHAT_TORAH_I:
        case HC_SD_PESACH_1: case HC_SD_PESACH_2_C:
        case HC_SD_PESACH_7: case HC_SD_PESACH_LAST_C:
        case HC_SD_SHAVUOT: case HC_SD_SHAVUOT_2_C: case HC_SD_PURIM:
            o->festival = d; break;
        default: {
            int night = hc_sd_chanukah_night(d);
            if (night > 0) o->chanukah_day = night;
            break;
        }
        }
    }
    if (o->sukkot_intermediate || o->pesach_day) o->festival = HC_SD_NONE;
}

/* ── the readings ────────────────────────────────────────────────────── */

/*! Numbers 28:9-15 -- the Shabbat of Rosh Chodesh, the last two fragments. */
static hc_torah_span rosh_chodesh_maftir(void)
{
    int n = HC_SPECIAL_TORAH_LEN[HC_ST_ROSH_CHODESH_TORAH];
    return join(span_of(HC_ST_ROSH_CHODESH_TORAH, n - 2),
                span_of(HC_ST_ROSH_CHODESH_TORAH, n - 1));
}

static hc_torah_span korbanot(int i)      { return span_of(HC_ST_SUCCOS_KORBANOT, i); }
static hc_torah_span chan_korban(int i)   { return span_of(HC_ST_CHANUKAH_KORBANOT, i); }

/*! The two korbanot fragments of a day's nasi, read as one. */
static hc_torah_span chanukah_full(int n)
{
    return join(chan_korban(2 * (n - 1)), chan_korban(2 * (n - 1) + 1));
}

/*! Outside Israel each day of Chol HaMoed reads the korbanot of both days it
 *  could be, because which one it is depends on the doubted day. */
static hc_torah_span korbanot_today(int n, int in_israel)
{
    return in_israel ? korbanot(n) : join(korbanot(n), korbanot(n + 1));
}

/*! Succot days 1 and 2, and Pesach day 2, share one division. */
static void succos1_torah(hc_torah_reading *r, int shabbat)
{
    take(r, HC_ST_SUCCOS1_SHABBOS_TORAH);
    if (!shabbat) { const int into[] = {2, 4}; merge(r, into, 2); }
}

/*! The last day of a festival. On a weekday the first two aliyot are not read
 *  at all, rather than merged. */
static void festival_end_torah(hc_torah_reading *r, int shabbat)
{
    take(r, HC_ST_FESTIVAL_END_SHABBOS_TORAH);
    if (!shabbat) {
        memmove(r->aliyot, r->aliyot + 2, (size_t)(r->aliyot_count - 2) * sizeof(r->aliyot[0]));
        r->aliyot_count -= 2;
    }
}

/*! Vezot Haberachah read to its end, with Bereishit begun after it from a
 *  second scroll, and the maftir from a third. */
static void simchat_torah_torah(hc_torah_reading *r)
{
    const hc_chumash_reading *vz = hc_chumash_lookup(HC_VEZOT_HABRACHA, HC_PARSHA_NONE);
    for (int i = 0; i < 7; i++) r->aliyot[i] = vz->aliyot[i];
    r->aliyot_count = 7;
    const int into[] = {7};
    merge(r, into, 1);                                   /* seven into six */
    r->aliyot[r->aliyot_count] = span_of(HC_ST_SIMCHAS_TORAH_CHASSAN_BEREISHIS, 0);
    r->aliyot_sefer[r->aliyot_count] = 2;
    r->aliyot_count++;
}

/*! The morning reading of a day that reads something other than the parsha,
 *  or 0 if this day reads the parsha after all. */
static int festival_morning(const occasion *o, int shabbat, int in_israel,
                            hc_custom custom, int pesach_on_thursday,
                            hc_torah_reading *r)
{
    reset(r, HC_SLOT_MORNING);

    if (o->sukkot_intermediate) {
        hc_torah_span today = korbanot_today(o->sukkot_intermediate, in_israel);
        if (shabbat) {
            take(r, HC_ST_INTERMEDIATE_SHABBOS_TORAH);
            r->maftir = today; r->maftir_sefer = 2;
            return 1;
        }
        /* The korbanot run out before the days do, so from the fourth
         * intermediate day on the same three are read. */
        int n = o->sukkot_intermediate < 4 ? o->sukkot_intermediate : 4;
        if (reads_sefard(custom)) {
            for (int i = 0; i < 4; i++) r->aliyot[i] = today;
        } else {
            r->aliyot[0] = korbanot(n);
            r->aliyot[1] = korbanot(n + 1);
            r->aliyot[2] = korbanot(n + 2);
            r->aliyot[3] = today;
        }
        r->aliyot_count = 4;
        return 1;
    }

    if (o->pesach_day) {
        hc_torah_span maftir_end = joined(HC_ST_PESACH_INTERMEDIATE_MAFTIR_END);
        if (shabbat) {
            take(r, HC_ST_INTERMEDIATE_SHABBOS_TORAH);
            r->maftir = maftir_end; r->maftir_sefer = 2;
            return 1;
        }
        /* When Pesach begins on a Thursday the fourth and fifth days fall on
         * Shabbat and the day after, and the readings shift back by one. */
        int day = o->pesach_day;
        if (pesach_on_thursday && (day == 4 || day == 5)) day--;
        switch (day) {
        case 2: succos1_torah(r, 0); { const int into[] = {4, 5}; merge(r, into, 2); } break;
        case 3: take(r, HC_ST_PESACH_INTERMEDIATE_TORAH3); break;
        case 4: take(r, HC_ST_PESACH_INTERMEDIATE_TORAH4); break;
        case 5: {
            /* the fourth, fifth and sixth of the Shabbat aliyot, the middle
             * two read as one */
            take(r, HC_ST_INTERMEDIATE_SHABBOS_TORAH);
            memmove(r->aliyot, r->aliyot + 3, 4 * sizeof(r->aliyot[0]));
            r->aliyot_count = 4;
            const int into[] = {3};
            merge(r, into, 1);
            r->aliyot_count = 3;
            break;
        }
        case 6: take(r, HC_ST_PESACH_INTERMEDIATE_TORAH6); break;
        default: return 0;
        }
        r->aliyot[r->aliyot_count++] = maftir_end;   /* read as a fourth aliyah */
        return 1;
    }

    if (o->chanukah_day && !shabbat) {
        int n = o->chanukah_day, sefard = reads_sefard(custom);
        if (o->rosh_chodesh) {
            /* Rosh Chodesh Tevet: its own reading in three from one scroll,
             * and Chanukah after it from a second. */
            r->aliyot[0] = join(span_of(HC_ST_ROSH_CHODESH_TORAH, 0),
                                span_of(HC_ST_ROSH_CHODESH_TORAH, 2));
            r->aliyot[1] = join(span_of(HC_ST_ROSH_CHODESH_TORAH, 3),
                                span_of(HC_ST_ROSH_CHODESH_TORAH, 4));
            r->aliyot[2] = span_of(HC_ST_ROSH_CHODESH_TORAH, 5);
            r->aliyot[3] = chanukah_full(n);
            r->aliyot_sefer[3] = 2;
            r->aliyot_count = 4;
        } else if (n == 1) {
            r->aliyot[0] = sefard
                ? join(span_of(HC_ST_CHANUKAH_DAY1_COHEN, 0), span_of(HC_ST_CHANUKAH_DAY1_COHEN, 1))
                : span_of(HC_ST_CHANUKAH_DAY1_COHEN, 1);
            r->aliyot[1] = chan_korban(0);
            r->aliyot[2] = chan_korban(1);
            r->aliyot_count = 3;
        } else {
            int last = HC_SPECIAL_TORAH_LEN[HC_ST_CHANUKAH_KORBANOT] - 1;
            hc_torah_span zos = chan_korban(last);
            r->aliyot[0] = chan_korban(2 * (n - 1));
            r->aliyot[1] = chan_korban(2 * (n - 1) + 1);
            if (n != 8) r->aliyot[2] = sefard ? chanukah_full(n) : chanukah_full(n + 1);
            else        r->aliyot[2] = sefard ? join(chanukah_full(n), zos) : zos;
            r->aliyot_count = 3;
        }
        return 1;
    }

    if (o->festival == HC_SD_NONE) return 0;

    switch (o->festival) {
    case HC_SD_ROSH_HASHANA_1:
        take(r, HC_ST_ROSH_HASHANAH1_SHABBOS_TORAH);
        if (!shabbat) { const int into[] = {3, 5}; merge(r, into, 2); }
        r->maftir = joined(HC_ST_ROSH_HASHANAH1_MAFTIR); r->maftir_sefer = 1; return 1;
    case HC_SD_ROSH_HASHANA_2:
        take(r, HC_ST_ROSH_HASHANAH2_TORAH);
        r->maftir = joined(HC_ST_ROSH_HASHANAH1_MAFTIR); r->maftir_sefer = 1; return 1;
    case HC_SD_YOM_KIPPUR:
        take(r, HC_ST_YOM_KIPPUR_SHABBOS_TORAH);
        if (!shabbat) { const int into[] = {2}; merge(r, into, 1); }
        r->maftir = joined(HC_ST_YOM_KIPPUR_MAFTIR); r->maftir_sefer = 1; return 1;
    case HC_SD_SUKKOT_1: case HC_SD_SUKKOT_2_C:
        succos1_torah(r, shabbat);
        r->maftir = korbanot(0); r->maftir_sefer = 1; return 1;
    case HC_SD_SHMINI_ATZERET_C:
        take(r, HC_ST_FESTIVAL_END_SHABBOS_TORAH);
        if (!shabbat) { const int into[] = {2, 3}; merge(r, into, 2); }
        r->maftir = korbanot(HC_SPECIAL_TORAH_LEN[HC_ST_SUCCOS_KORBANOT] - 1);
        r->maftir_sefer = 1; return 1;
    case HC_SD_SIMCHAT_TORAH_C: case HC_SD_SIMCHAT_TORAH_I:
        simchat_torah_torah(r);
        r->maftir = korbanot(HC_SPECIAL_TORAH_LEN[HC_ST_SUCCOS_KORBANOT] - 1);
        r->maftir_sefer = 3; return 1;
    case HC_SD_PESACH_1:
        take(r, HC_ST_PESACH1_SHABBOS_TORAH);
        if (!shabbat) { const int into[] = {4, 7}; merge(r, into, 2); }
        r->maftir = joined(HC_ST_PESACH1_MAFTIR); r->maftir_sefer = 1; return 1;
    case HC_SD_PESACH_2_C:
        succos1_torah(r, 0);
        r->maftir = joined(HC_ST_PESACH1_MAFTIR); r->maftir_sefer = 1; return 1;
    case HC_SD_PESACH_7:
        take(r, HC_ST_PESACH7_SHABBOS_TORAH);
        if (!shabbat) { const int into[] = {2, 4}; merge(r, into, 2); }
        r->maftir = joined(HC_ST_PESACH_INTERMEDIATE_MAFTIR_END); r->maftir_sefer = 1; return 1;
    case HC_SD_PESACH_LAST_C:
        festival_end_torah(r, shabbat);
        r->maftir = joined(HC_ST_PESACH_INTERMEDIATE_MAFTIR_END); r->maftir_sefer = 1; return 1;
    case HC_SD_SHAVUOT:
        take(r, HC_ST_SHAVUOS1_TORAH);
        r->maftir = joined(HC_ST_SHAVUOS1_MAFTIR); r->maftir_sefer = 1; return 1;
    case HC_SD_SHAVUOT_2_C:
        festival_end_torah(r, shabbat);
        r->maftir = joined(HC_ST_SHAVUOS1_MAFTIR); r->maftir_sefer = 1; return 1;
    case HC_SD_PURIM:
        take(r, HC_ST_PURIM_TORAH); return 1;
    case HC_SD_SHUSHAN_PURIM:
        /* Purim Meshulash: on Shabbat the parsha is still read, and Purim's
         * reading becomes its maftir. */
        if (shabbat) return 0;
        take(r, HC_ST_PURIM_TORAH); return 1;
    case HC_SD_FAST_9_AV:
        take(r, HC_ST_TISHA_BE_AV_TORAH); return 1;
    case HC_SD_TZOM_GEDALIA: case HC_SD_TENTH_TEVET:
    case HC_SD_TAANIT_ESTHER: case HC_SD_FAST_17_TAMUZ:
        take(r, HC_ST_FAST_AFTERNOON_TORAH_PART1);
        r->aliyot[1] = span_of(HC_ST_INTERMEDIATE_SHABBOS_TORAH, 3);
        r->aliyot[2] = span_of(HC_ST_INTERMEDIATE_SHABBOS_TORAH, 4);
        r->aliyot_count = 3; return 1;
    default:
        return 0;
    }
}

/*! The division of the parsha this custom reads: the nearest ancestor that has
 *  one of its own, else the common division. */
static const hc_torah_span *aliyot_for(const hc_chumash_reading *w, hc_custom c,
                                       uint8_t *division)
{
    *division = HC_CUSTOM_COUNT;
    if (w->aliyot_chabad && hc_custom_is_under(c, HC_CUSTOM_CHABAD)) {
        *division = HC_CUSTOM_CHABAD; return w->aliyot_chabad;
    }
    if (w->aliyot_ashkenaz && hc_custom_is_under(c, HC_CUSTOM_ASHKENAZ)) {
        *division = HC_CUSTOM_ASHKENAZ; return w->aliyot_ashkenaz;
    }
    return w->aliyot;
}

/*! The week read on this Shabbat, or NULL if a festival has taken it. */
static const hc_chumash_reading *reading_on(hc_date *shabbat, int in_israel)
{
    hc_reading rd;
    if (hc_get_parsha(shabbat, in_israel, &rd) != 0) return NULL;
    if (rd.p1 == HC_PARSHA_NONE) return NULL;
    const hc_chumash_reading *w = hc_chumash_lookup(rd.p1, rd.p2);
    if (!w && rd.p2 != HC_PARSHA_NONE) w = hc_chumash_lookup(rd.p2, HC_PARSHA_NONE);
    return w;
}

/*! The next weekly reading after this day: the coming Shabbat's, or the one
 *  after when this day is itself Shabbat. A Shabbat taken by a festival has
 *  none, so the search steps over it. */
static const hc_chumash_reading *next_weekly(hc_date *date, int in_israel)
{
    hc_date d = *date;
    int to_shabbat = (SATURDAY + 7 - (int)hc_get_day_of_week(&d)) % 7;
    if (hc_date_add_days(&d, to_shabbat == 0 ? 7 : to_shabbat) != 0) return NULL;
    for (int week = 0; week < 54; week++) {
        const hc_chumash_reading *w = reading_on(&d, in_israel);
        if (w) return w;
        if (hc_date_add_days(&d, 7) != 0) return NULL;
    }
    return NULL;
}

/*! Whether Pesach of this year began on a Thursday, which shifts Chol HaMoed. */
static int pesach_on_thursday(hc_date *date, int in_israel)
{
    for (int back = 0; back < 30; back++) {
        hc_date d = *date;
        if (hc_date_add_days(&d, -back) != 0) break;
        hc_special_day days[HC_MAX_SPECIAL_DAYS];
        int n = 0;
        if (hc_get_special_days(&d, in_israel, days, &n) != 0) continue;
        for (int i = 0; i < n; i++)
            if (days[i] == HC_SD_PESACH_1)
                return (int)hc_get_day_of_week(&d) == THURSDAY;
    }
    return 0;
}

/*! The parsha's own reading, with the special days that displace part of it. */
static void with_special_days(const occasion *o, hc_torah_reading *r)
{
    int rosh_chodesh = o->rosh_chodesh;
    int maftir_replaced = 1;

    if (o->shushan_purim) {
        /* Purim Meshulash: Purim's reading is read as the maftir; Zachor was
         * read the week before. */
        r->maftir = join(span_of(HC_ST_PURIM_TORAH, 0),
                         span_of(HC_ST_PURIM_TORAH,
                                 HC_SPECIAL_TORAH_LEN[HC_ST_PURIM_TORAH] - 1));
    } else if (o->parsha_maftir != HC_ST_COUNT) {
        r->maftir = joined(o->parsha_maftir);
    } else if (o->chanukah_day) {
        r->maftir = chanukah_full(o->chanukah_day);
    } else if (rosh_chodesh) {
        r->maftir = rosh_chodesh_maftir();
        rosh_chodesh = 0;
    } else {
        maftir_replaced = 0;
    }

    r->maftir_sefer = r->maftir.book == HC_BOOK_NONE ? 0 : (maftir_replaced ? 2 : 1);

    if (rosh_chodesh) {
        /* Something else has the maftir, so Rosh Chodesh is read as the
         * seventh aliyah and the parsha's seventh joins the sixth. Three
         * scrolls: the parsha, Rosh Chodesh, and whatever took the maftir. */
        int n = r->aliyot_count;
        r->aliyot[n - 2] = join(r->aliyot[n - 2], r->aliyot[n - 1]);
        r->aliyot[n - 1] = rosh_chodesh_maftir();
        r->aliyot_sefer[n - 1] = 2;
        r->maftir_sefer = 3;
    }
}

/*! The three aliyot of a Monday, a Thursday or Shabbat Mincha. */
static int weekday_parsha(hc_date *date, int in_israel, hc_reading_slot slot,
                          hc_torah_reading *r)
{
    const hc_chumash_reading *w = next_weekly(date, in_israel);
    if (!w) return 0;
    reset(r, slot);
    for (int i = 0; i < 3; i++) r->aliyot[i] = w->weekday[i];
    r->aliyot_count = 3;
    r->parshiyot[0] = w->parsha1;
    r->parshiyot[1] = w->parsha2;
    return 1;
}

/*! The reading of the night of Simchat Torah, or 0. Ashkenaz only, and not all
 *  of them: the first five aliyot of Vezot Haberachah, though some read only
 *  the first three. That split does not follow rite lines, so it cannot be a
 *  custom of its own and is said in the note instead. Outside Israel only. */
static int simchat_torah_evening(const occasion *o, hc_custom custom,
                                 hc_torah_reading *r)
{
    if (o->festival != HC_SD_SIMCHAT_TORAH_C) return 0;
    if (!hc_custom_is_under(custom, HC_CUSTOM_ASHKENAZ)) return 0;
    const hc_chumash_reading *vz = hc_chumash_lookup(HC_VEZOT_HABRACHA, HC_PARSHA_NONE);
    if (!vz) return 0;
    reset(r, HC_SLOT_EVENING);
    uint8_t unused;
    const hc_torah_span *a = aliyot_for(vz, custom, &unused);
    for (int i = 0; i < 5; i++) r->aliyot[i] = a[i];
    r->aliyot_count = 5;
    r->note = "Read by Ashkenaz, and not by all of them. Some read only the "
              "first three of these aliyot. The split does not follow rite "
              "lines. Nitei Gavriel, Hilchos Sukkos.";
    return 1;
}

int hc_torah_reading_for_day(hc_date *date, hc_custom custom, int in_israel,
                             hc_torah_reading results[HC_MAX_TORAH_READINGS],
                             int *count)
{
    if (!date || !results || !count) return -1;
    if (custom < 0 || custom >= HC_CUSTOM_COUNT) return -1;
    *count = 0;

    int dow = (int)hc_get_day_of_week(date);
    int shabbat = (dow == SATURDAY);

    occasion o;
    scan(date, in_israel, &o);

    hc_torah_reading r;
    if (simchat_torah_evening(&o, custom, &r)) results[(*count)++] = r;

    /* A day that reads something of its own reads that instead of the parsha;
     * only where it reads nothing of its own does the parsha, or Rosh Chodesh,
     * get the morning. */
    int have_morning = festival_morning(&o, shabbat, in_israel, custom,
                                        pesach_on_thursday(date, in_israel), &r);
    if (!have_morning && shabbat) {
        const hc_chumash_reading *w = reading_on(date, in_israel);
        if (w) {
            reset(&r, HC_SLOT_MORNING);
            uint8_t division;
            const hc_torah_span *a = aliyot_for(w, custom, &division);
            for (int i = 0; i < 7; i++) r.aliyot[i] = a[i];
            r.aliyot_count = 7;
            r.maftir = w->maftir;
            r.parshiyot[0] = w->parsha1;
            r.parshiyot[1] = w->parsha2;
            r.division = division;
            with_special_days(&o, &r);
            have_morning = 1;
        }
    } else if (!have_morning && o.rosh_chodesh) {
        /* Rosh Chodesh on a weekday: four aliyot, and the Gra divides the
         * middle of them differently. */
        reset(&r, HC_SLOT_MORNING);
        r.aliyot[0] = join(span_of(HC_ST_ROSH_CHODESH_TORAH, 0), span_of(HC_ST_ROSH_CHODESH_TORAH, 1));
        r.aliyot[1] = hc_custom_is_under(custom, HC_CUSTOM_HAGRA)
            ? join(span_of(HC_ST_ROSH_CHODESH_TORAH, 2), span_of(HC_ST_ROSH_CHODESH_TORAH, 3))
            : join(span_of(HC_ST_ROSH_CHODESH_TORAH, 1), span_of(HC_ST_ROSH_CHODESH_TORAH, 2));
        r.aliyot[2] = join(span_of(HC_ST_ROSH_CHODESH_TORAH, 3), span_of(HC_ST_ROSH_CHODESH_TORAH, 4));
        r.aliyot[3] = span_of(HC_ST_ROSH_CHODESH_TORAH, 5);
        r.aliyot_count = 4;
        have_morning = 1;
    } else if (!have_morning && (dow == MONDAY || dow == THURSDAY)) {
        have_morning = weekday_parsha(date, in_israel, HC_SLOT_MORNING, &r);
    }
    if (have_morning) results[(*count)++] = r;

    if (o.yom_kippur) {
        reset(&r, HC_SLOT_AFTERNOON);
        take(&r, HC_ST_YOM_KIPPUR_AFTERNOON_TORAH);
        results[(*count)++] = r;
    } else if (o.fast) {
        reset(&r, HC_SLOT_AFTERNOON);
        take(&r, HC_ST_FAST_AFTERNOON_TORAH_PART1);
        r.aliyot[1] = span_of(HC_ST_INTERMEDIATE_SHABBOS_TORAH, 3);
        r.aliyot[2] = span_of(HC_ST_INTERMEDIATE_SHABBOS_TORAH, 4);
        r.aliyot_count = 3;
        results[(*count)++] = r;
    } else if (shabbat) {
        if (weekday_parsha(date, in_israel, HC_SLOT_AFTERNOON, &r))
            results[(*count)++] = r;
    }
    return 0;
}

int hc_torah_reading_for_date(hc_date *date, hc_custom custom, int in_israel,
                              hc_torah_reading *out)
{
    hc_torah_reading rs[HC_MAX_TORAH_READINGS];
    int n = 0;
    if (!out) return -1;
    if (hc_torah_reading_for_day(date, custom, in_israel, rs, &n) != 0) return -1;
    for (int i = 0; i < n; i++)
        if (rs[i].slot == HC_SLOT_MORNING) { *out = rs[i]; return 0; }
    return -1;
}

int hc_torah_sizeof_span(void)          { return (int)sizeof(hc_torah_span); }
int hc_torah_sizeof_reading(void)       { return (int)sizeof(hc_torah_reading); }
int hc_torah_offsetof_aliyot(void)      { return (int)offsetof(hc_torah_reading, aliyot); }
int hc_torah_offsetof_aliyot_count(void){ return (int)offsetof(hc_torah_reading, aliyot_count); }
