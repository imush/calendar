/*!
 * \file test_torah_reading.c
 * \brief The Torah readings, against opentorah's own answers.
 *
 * The oracle is the same file the Java library checks itself against, shared
 * through the data submodule: every reading opentorah produces, for all
 * customs, both lands, Jewish years 5780-5860. A line is
 *
 *   date <TAB> location|dayOfWeek|parsha|specialDays|slot <TAB> customs <TAB> kind <TAB> value
 *
 * and this walks them, asks the library the same question, and compares.
 */
#include "hc_test.h"
#include "hc_torah_reading.h"
#include "hconverter.h"

#include <stdio.h>
#include <string.h>

static const char *BOOK_NAMES[] = {
    "", "Genesis", "Exodus", "Leviticus", "Numbers", "Deuteronomy",
    "Joshua", "Judges", "SamuelI", "SamuelII", "KingsI", "KingsII",
    "Isaiah", "Jeremiah", "Ezekiel", "Hosea", "Joel", "Amos", "Obadiah",
    "Jonah", "Micah", "Nahum", "Habakkuk", "Zephaniah", "Haggai",
    "Zechariah", "Malachi",
};

/*! The fixture names a custom as opentorah spells it, "ChayeyOdom"; the data
 *  names it as its key, "CHAYEY_ODOM". Convert and look up. */
static hc_custom custom_by_name(const char *n)
{
    char key[64];
    size_t at = 0;
    for (size_t i = 0; n[i] && at + 2 < sizeof(key); i++) {
        if (i > 0 && n[i] >= 'A' && n[i] <= 'Z') key[at++] = '_';
        char ch = n[i];
        if (ch >= 'a' && ch <= 'z') ch = (char)(ch - 'a' + 'A');
        key[at++] = ch;
    }
    key[at] = '\0';
    for (int i = 0; i < HC_CUSTOM_COUNT; i++) {
        const char *c = hc_custom_name((hc_custom)i);
        if (c && strcmp(c, key) == 0) return (hc_custom)i;
    }
    return (hc_custom)-1;
}

/*! Render aliyot the way the fixture writes them: "1:Book:c:v:c:v ...". */
static void render_torah(const hc_torah_reading *r, char *out, size_t cap)
{
    out[0] = '\0';
    if (!r || r->aliyot_count == 0) { snprintf(out, cap, "-"); return; }
    size_t at = 0;
    for (int i = 0; i < r->aliyot_count; i++) {
        const hc_torah_span *s = &r->aliyot[i];
        at += (size_t)snprintf(out + at, cap - at, "%s%d:%s:%d:%d:%d:%d",
                               i ? " " : "", i + 1, BOOK_NAMES[s->book],
                               s->from_ch, s->from_v, s->to_ch, s->to_v);
        if (at >= cap) return;
    }
}

static void render_maftir(const hc_torah_reading *r, char *out, size_t cap)
{
    if (!r || r->maftir.book == HC_BOOK_NONE) { snprintf(out, cap, "-"); return; }
    const hc_torah_span *s = &r->maftir;
    snprintf(out, cap, "%s:%d:%d:%d:%d", BOOK_NAMES[s->book],
             s->from_ch, s->from_v, s->to_ch, s->to_v);
}

void test_torah_reading(void)
{
    FILE *f = fopen(HC_READINGS_ORACLE, "r");
    if (!f) {
        HC_ASSERT(0, "readings oracle not found at " HC_READINGS_ORACLE);
        return;
    }

    char line[4096];
    int checked = 0, wrong = 0;
    char first_bad[512] = "";

    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        char *date = strtok(line, "\t");
        char *situation = strtok(NULL, "\t");
        char *customs = strtok(NULL, "\t");
        char *kind = strtok(NULL, "\t");
        char *value = strtok(NULL, "\t\n\r");
        if (!date || !situation || !customs || !kind || !value) continue;
        int want_torah = strcmp(kind, "torah") == 0;
        if (!want_torah && strcmp(kind, "maftir") != 0) continue;

        /* location|dayOfWeek|parsha|specialDays|slot */
        char sit[256];
        snprintf(sit, sizeof(sit), "%s", situation);
        char *loc = strtok(sit, "|");
        strtok(NULL, "|");                    /* day of week */
        strtok(NULL, "|");                    /* parsha */
        strtok(NULL, "|");                    /* special days */
        char *slot_s = strtok(NULL, "|");
        if (!loc || !slot_s) continue;
        int in_israel = strcmp(loc, "EY") == 0;
        hc_reading_slot slot = strcmp(slot_s, "afternoon") == 0 ? HC_SLOT_AFTERNOON
                             : strcmp(slot_s, "evening")   == 0 ? HC_SLOT_EVENING
                             :                                    HC_SLOT_MORNING;

        int y, m, d;
        if (sscanf(date, "%d-%d-%d", &y, &m, &d) != 3) continue;

        /* customs: "ALL" or "A+B+C", in opentorah's spelling */
        char cbuf[1024];
        snprintf(cbuf, sizeof(cbuf), "%s", customs);
        hc_custom list[HC_CUSTOM_COUNT];
        int n_customs = 0;
        if (strcmp(cbuf, "ALL") == 0) {
            for (int i = 0; i < HC_CUSTOM_COUNT; i++) list[n_customs++] = (hc_custom)i;
        } else {
            for (char *t = strtok(cbuf, "+"); t; t = strtok(NULL, "+")) {
                hc_custom c = custom_by_name(t);
                if ((int)c >= 0) list[n_customs++] = c;
            }
        }

        for (int i = 0; i < n_customs; i++) {
            hc_date dt; dt.calendar_type = GREGORIAN;
            dt.year = y; dt.month = m; dt.day = d;
            hc_torah_reading rs[HC_MAX_TORAH_READINGS];
            int n = 0;
            char got[2048];
            const hc_torah_reading *found = NULL;
            if (hc_torah_reading_for_day(&dt, list[i], in_israel, rs, &n) == 0)
                for (int k = 0; k < n; k++)
                    if (rs[k].slot == slot) found = &rs[k];
            if (want_torah) render_torah(found, got, sizeof(got));
            else            render_maftir(found, got, sizeof(got));

            checked++;
            if (strcmp(got, value) != 0) {
                if (wrong == 0)
                    snprintf(first_bad, sizeof(first_bad),
                             "%s %s %s: expected %.90s, got %.90s",
                             date, situation, hc_custom_name(list[i]), value, got);
                wrong++;
            }
        }
    }
    fclose(f);

    HC_ASSERT(checked > 0, "readings oracle had rows to check");
    if (wrong != 0) {
        fprintf(stderr, "    %d of %d disagree; first: %s\n", wrong, checked, first_bad);
    }
    HC_ASSERT(wrong == 0, "every Torah reading agrees with opentorah");
}
