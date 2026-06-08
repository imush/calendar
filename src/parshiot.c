#include "parshiot.h"
#include "hc_internal.h"
#include <stdlib.h>
#include <string.h>

/* from hconverter.c */
hc_cal_impl *get_calendar(hc_calendar_type t);
/* from hebrew.c */
heb_year_type hc_get_heb_year_type(int year);

/* ── parsha name table ───────────────────────────────────────────────────── */

static const char *PARSHA_NAMES[HC_PARSHA_COUNT] = {
    [HC_PARSHA_NONE]    = "",
    [HC_BEREISHIT]      = "Bereishit",
    [HC_NOACH]          = "Noach",
    [HC_LECH_LECHA]     = "Lech Lecha",
    [HC_VAYERA]         = "Vayera",
    [HC_CHAYEI_SARAH]   = "Chayei Sarah",
    [HC_TOLDOT]         = "Toldot",
    [HC_VAYETZE]        = "Vayetze",
    [HC_VAYISHLACH]     = "Vayishlach",
    [HC_VAYESHEV]       = "Vayeshev",
    [HC_MIKETZ]         = "Miketz",
    [HC_VAYIGASH]       = "Vayigash",
    [HC_VAYECHI]        = "Vayechi",
    [HC_SHEMOT]         = "Shemot",
    [HC_VAERA]          = "Vaera",
    [HC_BO]             = "Bo",
    [HC_BESHALACH]      = "Beshalach",
    [HC_YITRO]          = "Yitro",
    [HC_MISHPATIM]      = "Mishpatim",
    [HC_TERUMAH]        = "Terumah",
    [HC_TETZAVEH]       = "Tetzaveh",
    [HC_KI_TISA]        = "Ki Tisa",
    [HC_VAYAKHEL]       = "Vayakhel",
    [HC_PEKUDEI]        = "Pekudei",
    [HC_VAYIKRA]        = "Vayikra",
    [HC_TZAV]           = "Tzav",
    [HC_SHEMINI]        = "Shemini",
    [HC_TAZRIA]         = "Tazria",
    [HC_METZORA]        = "Metzora",
    [HC_ACHAREI_MOT]    = "Acharei Mot",
    [HC_KEDOSHIM]       = "Kedoshim",
    [HC_EMOR]           = "Emor",
    [HC_BEHAR]          = "Behar",
    [HC_BECHUKOTAI]     = "Bechukotai",
    [HC_BAMIDBAR]       = "Bamidbar",
    [HC_NASO]           = "Naso",
    [HC_BEHAALOTECHA]   = "Behaalotecha",
    [HC_SHELACH]        = "Shelach",
    [HC_KORACH]         = "Korach",
    [HC_CHUKAT]         = "Chukat",
    [HC_BALAK]          = "Balak",
    [HC_PINCHAS]        = "Pinchas",
    [HC_MATOT]          = "Matot",
    [HC_MASEI]          = "Masei",
    [HC_DEVARIM]        = "Devarim",
    [HC_VAETCHANAN]     = "Vaetchanan",
    [HC_EIKEV]          = "Eikev",
    [HC_REEH]           = "Reeh",
    [HC_SHOFTIM]        = "Shoftim",
    [HC_KI_TEITZEI]     = "Ki Teitzei",
    [HC_KI_TAVO]        = "Ki Tavo",
    [HC_NITZAVIM]       = "Nitzavim",
    [HC_VAYEILECH]      = "Vayeilech",
    [HC_HAAZINU]        = "Haazinu",
};

const char *hc_parsha_name(hc_parsha p)
{
    if (p <= HC_PARSHA_NONE || p >= HC_PARSHA_COUNT) return NULL;
    return PARSHA_NAMES[p];
}

/* ── schedule table types ────────────────────────────────────────────────── */

/* sentinel marking end of a schedule array */
#define SCH_END  {(hc_parsha)0xFF, (hc_parsha)0xFF}
/* Yom Tov / Chol Hamoed slot (no parsha) */
#define YT       {HC_PARSHA_NONE, HC_PARSHA_NONE}
/* single parsha */
#define S(p)     {(p), HC_PARSHA_NONE}
/* double parsha */
#define D(a,b)   {(a), (b)}

/* ── 14 year-type schedules (Israel / Diaspora) ─────────────────────────── */
/* Encoding matches ParshiotYearType.java exactly.
   Each array is terminated by SCH_END (p1==0xFF).                          */

/* A : Mon/Full/Thu — Israel (50) */
static const hc_reading SCH_F25_I[] = {
    S(HC_VAYEILECH), S(HC_HAAZINU), YT,
    S(HC_BEREISHIT), S(HC_NOACH), S(HC_LECH_LECHA), S(HC_VAYERA), S(HC_CHAYEI_SARAH),
    S(HC_TOLDOT), S(HC_VAYETZE), S(HC_VAYISHLACH), S(HC_VAYESHEV), S(HC_MIKETZ),
    S(HC_VAYIGASH), S(HC_VAYECHI), S(HC_SHEMOT), S(HC_VAERA), S(HC_BO), S(HC_BESHALACH),
    S(HC_YITRO), S(HC_MISHPATIM), S(HC_TERUMAH), S(HC_TETZAVEH), S(HC_KI_TISA),
    D(HC_VAYAKHEL,HC_PEKUDEI), S(HC_VAYIKRA), S(HC_TZAV), YT,
    S(HC_SHEMINI), D(HC_TAZRIA,HC_METZORA), D(HC_ACHAREI_MOT,HC_KEDOSHIM), S(HC_EMOR),
    D(HC_BEHAR,HC_BECHUKOTAI), S(HC_BAMIDBAR), S(HC_NASO), S(HC_BEHAALOTECHA),
    S(HC_SHELACH), S(HC_KORACH), S(HC_CHUKAT), S(HC_BALAK), S(HC_PINCHAS),
    D(HC_MATOT,HC_MASEI), S(HC_DEVARIM), S(HC_VAETCHANAN), S(HC_EIKEV), S(HC_REEH),
    S(HC_SHOFTIM), S(HC_KI_TEITZEI), S(HC_KI_TAVO), D(HC_NITZAVIM,HC_VAYEILECH),
    SCH_END
};
/* A/B/C Diaspora (50) */
static const hc_reading SCH_F25_D[] = {
    S(HC_VAYEILECH), S(HC_HAAZINU), YT,
    S(HC_BEREISHIT), S(HC_NOACH), S(HC_LECH_LECHA), S(HC_VAYERA), S(HC_CHAYEI_SARAH),
    S(HC_TOLDOT), S(HC_VAYETZE), S(HC_VAYISHLACH), S(HC_VAYESHEV), S(HC_MIKETZ),
    S(HC_VAYIGASH), S(HC_VAYECHI), S(HC_SHEMOT), S(HC_VAERA), S(HC_BO), S(HC_BESHALACH),
    S(HC_YITRO), S(HC_MISHPATIM), S(HC_TERUMAH), S(HC_TETZAVEH), S(HC_KI_TISA),
    D(HC_VAYAKHEL,HC_PEKUDEI), S(HC_VAYIKRA), S(HC_TZAV), YT,
    S(HC_SHEMINI), D(HC_TAZRIA,HC_METZORA), D(HC_ACHAREI_MOT,HC_KEDOSHIM), S(HC_EMOR),
    D(HC_BEHAR,HC_BECHUKOTAI), S(HC_BAMIDBAR), YT,
    S(HC_NASO), S(HC_BEHAALOTECHA), S(HC_SHELACH), S(HC_KORACH), D(HC_CHUKAT,HC_BALAK),
    S(HC_PINCHAS), D(HC_MATOT,HC_MASEI), S(HC_DEVARIM), S(HC_VAETCHANAN), S(HC_EIKEV),
    S(HC_REEH), S(HC_SHOFTIM), S(HC_KI_TEITZEI), S(HC_KI_TAVO), D(HC_NITZAVIM,HC_VAYEILECH),
    SCH_END
};
/* B : Mon/Short/Tue — same as A Israel for both */
/* (SCH_F25_I used for both Israel and Diaspora) */

/* C : Tue/Normal/Thu — same as A (Israel=I, Diaspora=D) */

/* D : Thu/Full/Sun (51) */
static const hc_reading SCH_F51[] = {
    S(HC_HAAZINU), YT, YT,
    S(HC_BEREISHIT), S(HC_NOACH), S(HC_LECH_LECHA), S(HC_VAYERA), S(HC_CHAYEI_SARAH),
    S(HC_TOLDOT), S(HC_VAYETZE), S(HC_VAYISHLACH), S(HC_VAYESHEV), S(HC_MIKETZ),
    S(HC_VAYIGASH), S(HC_VAYECHI), S(HC_SHEMOT), S(HC_VAERA), S(HC_BO), S(HC_BESHALACH),
    S(HC_YITRO), S(HC_MISHPATIM), S(HC_TERUMAH), S(HC_TETZAVEH), S(HC_KI_TISA),
    S(HC_VAYAKHEL), S(HC_PEKUDEI), S(HC_VAYIKRA), S(HC_TZAV), YT,
    S(HC_SHEMINI), D(HC_TAZRIA,HC_METZORA), D(HC_ACHAREI_MOT,HC_KEDOSHIM), S(HC_EMOR),
    D(HC_BEHAR,HC_BECHUKOTAI), S(HC_BAMIDBAR), S(HC_NASO), S(HC_BEHAALOTECHA),
    S(HC_SHELACH), S(HC_KORACH), S(HC_CHUKAT), S(HC_BALAK), S(HC_PINCHAS),
    D(HC_MATOT,HC_MASEI), S(HC_DEVARIM), S(HC_VAETCHANAN), S(HC_EIKEV), S(HC_REEH),
    S(HC_SHOFTIM), S(HC_KI_TEITZEI), S(HC_KI_TAVO), S(HC_NITZAVIM),
    SCH_END
};
/* E : Thu/Normal/Sat — Israel (51) */
static const hc_reading SCH_N57_I[] = {
    S(HC_HAAZINU), YT, YT,
    S(HC_BEREISHIT), S(HC_NOACH), S(HC_LECH_LECHA), S(HC_VAYERA), S(HC_CHAYEI_SARAH),
    S(HC_TOLDOT), S(HC_VAYETZE), S(HC_VAYISHLACH), S(HC_VAYESHEV), S(HC_MIKETZ),
    S(HC_VAYIGASH), S(HC_VAYECHI), S(HC_SHEMOT), S(HC_VAERA), S(HC_BO), S(HC_BESHALACH),
    S(HC_YITRO), S(HC_MISHPATIM), S(HC_TERUMAH), S(HC_TETZAVEH), S(HC_KI_TISA),
    D(HC_VAYAKHEL,HC_PEKUDEI), S(HC_VAYIKRA), S(HC_TZAV), YT,
    S(HC_SHEMINI), D(HC_TAZRIA,HC_METZORA), D(HC_ACHAREI_MOT,HC_KEDOSHIM), S(HC_EMOR),
    S(HC_BEHAR), S(HC_BECHUKOTAI), S(HC_BAMIDBAR), S(HC_NASO), S(HC_BEHAALOTECHA),
    S(HC_SHELACH), S(HC_KORACH), S(HC_CHUKAT), S(HC_BALAK), S(HC_PINCHAS),
    D(HC_MATOT,HC_MASEI), S(HC_DEVARIM), S(HC_VAETCHANAN), S(HC_EIKEV), S(HC_REEH),
    S(HC_SHOFTIM), S(HC_KI_TEITZEI), S(HC_KI_TAVO), S(HC_NITZAVIM),
    SCH_END
};
/* E : Thu/Normal/Sat — Diaspora (52) */
static const hc_reading SCH_N57_D[] = {
    S(HC_HAAZINU), YT, YT,
    S(HC_BEREISHIT), S(HC_NOACH), S(HC_LECH_LECHA), S(HC_VAYERA), S(HC_CHAYEI_SARAH),
    S(HC_TOLDOT), S(HC_VAYETZE), S(HC_VAYISHLACH), S(HC_VAYESHEV), S(HC_MIKETZ),
    S(HC_VAYIGASH), S(HC_VAYECHI), S(HC_SHEMOT), S(HC_VAERA), S(HC_BO), S(HC_BESHALACH),
    S(HC_YITRO), S(HC_MISHPATIM), S(HC_TERUMAH), S(HC_TETZAVEH), S(HC_KI_TISA),
    D(HC_VAYAKHEL,HC_PEKUDEI), S(HC_VAYIKRA), S(HC_TZAV), YT, YT,
    S(HC_SHEMINI), D(HC_TAZRIA,HC_METZORA), D(HC_ACHAREI_MOT,HC_KEDOSHIM), S(HC_EMOR),
    D(HC_BEHAR,HC_BECHUKOTAI), S(HC_BAMIDBAR),
    S(HC_NASO), S(HC_BEHAALOTECHA), S(HC_SHELACH), S(HC_KORACH), S(HC_CHUKAT), S(HC_BALAK),
    S(HC_PINCHAS), D(HC_MATOT,HC_MASEI), S(HC_DEVARIM), S(HC_VAETCHANAN), S(HC_EIKEV),
    S(HC_REEH), S(HC_SHOFTIM), S(HC_KI_TEITZEI), S(HC_KI_TAVO), S(HC_NITZAVIM),
    SCH_END
};
/* F : Sat/Full/Tue (51) */
static const hc_reading SCH_F73[] = {
    YT, S(HC_HAAZINU), YT, YT,
    S(HC_BEREISHIT), S(HC_NOACH), S(HC_LECH_LECHA), S(HC_VAYERA), S(HC_CHAYEI_SARAH),
    S(HC_TOLDOT), S(HC_VAYETZE), S(HC_VAYISHLACH), S(HC_VAYESHEV), S(HC_MIKETZ),
    S(HC_VAYIGASH), S(HC_VAYECHI), S(HC_SHEMOT), S(HC_VAERA), S(HC_BO), S(HC_BESHALACH),
    S(HC_YITRO), S(HC_MISHPATIM), S(HC_TERUMAH), S(HC_TETZAVEH), S(HC_KI_TISA),
    D(HC_VAYAKHEL,HC_PEKUDEI), S(HC_VAYIKRA), S(HC_TZAV), YT,
    S(HC_SHEMINI), D(HC_TAZRIA,HC_METZORA), D(HC_ACHAREI_MOT,HC_KEDOSHIM), S(HC_EMOR),
    D(HC_BEHAR,HC_BECHUKOTAI), S(HC_BAMIDBAR), S(HC_NASO), S(HC_BEHAALOTECHA),
    S(HC_SHELACH), S(HC_KORACH), S(HC_CHUKAT), S(HC_BALAK), S(HC_PINCHAS),
    D(HC_MATOT,HC_MASEI), S(HC_DEVARIM), S(HC_VAETCHANAN), S(HC_EIKEV), S(HC_REEH),
    S(HC_SHOFTIM), S(HC_KI_TEITZEI), S(HC_KI_TAVO), D(HC_NITZAVIM,HC_VAYEILECH),
    SCH_END
};
/* G : Sat/Short/Sun (51) */
static const hc_reading SCH_S71[] = {
    YT, S(HC_HAAZINU), YT, YT,
    S(HC_BEREISHIT), S(HC_NOACH), S(HC_LECH_LECHA), S(HC_VAYERA), S(HC_CHAYEI_SARAH),
    S(HC_TOLDOT), S(HC_VAYETZE), S(HC_VAYISHLACH), S(HC_VAYESHEV), S(HC_MIKETZ),
    S(HC_VAYIGASH), S(HC_VAYECHI), S(HC_SHEMOT), S(HC_VAERA), S(HC_BO), S(HC_BESHALACH),
    S(HC_YITRO), S(HC_MISHPATIM), S(HC_TERUMAH), S(HC_TETZAVEH), S(HC_KI_TISA),
    D(HC_VAYAKHEL,HC_PEKUDEI), S(HC_VAYIKRA), S(HC_TZAV), YT,
    S(HC_SHEMINI), D(HC_TAZRIA,HC_METZORA), D(HC_ACHAREI_MOT,HC_KEDOSHIM), S(HC_EMOR),
    D(HC_BEHAR,HC_BECHUKOTAI), S(HC_BAMIDBAR), S(HC_NASO), S(HC_BEHAALOTECHA),
    S(HC_SHELACH), S(HC_KORACH), S(HC_CHUKAT), S(HC_BALAK), S(HC_PINCHAS),
    D(HC_MATOT,HC_MASEI), S(HC_DEVARIM), S(HC_VAETCHANAN), S(HC_EIKEV), S(HC_REEH),
    S(HC_SHOFTIM), S(HC_KI_TEITZEI), S(HC_KI_TAVO), S(HC_NITZAVIM),
    SCH_END
};
/* H : Mon/Full/Sat — Israel (54) */
static const hc_reading SCH_F27_I[] = {
    S(HC_VAYEILECH), S(HC_HAAZINU), YT,
    S(HC_BEREISHIT), S(HC_NOACH), S(HC_LECH_LECHA), S(HC_VAYERA), S(HC_CHAYEI_SARAH),
    S(HC_TOLDOT), S(HC_VAYETZE), S(HC_VAYISHLACH), S(HC_VAYESHEV), S(HC_MIKETZ),
    S(HC_VAYIGASH), S(HC_VAYECHI), S(HC_SHEMOT), S(HC_VAERA), S(HC_BO), S(HC_BESHALACH),
    S(HC_YITRO), S(HC_MISHPATIM), S(HC_TERUMAH), S(HC_TETZAVEH), S(HC_KI_TISA),
    S(HC_VAYAKHEL), S(HC_PEKUDEI), S(HC_VAYIKRA), S(HC_TZAV),
    S(HC_SHEMINI), S(HC_TAZRIA), S(HC_METZORA), YT,
    S(HC_ACHAREI_MOT), S(HC_KEDOSHIM), S(HC_EMOR), S(HC_BEHAR), S(HC_BECHUKOTAI),
    S(HC_BAMIDBAR), S(HC_NASO), S(HC_BEHAALOTECHA), S(HC_SHELACH), S(HC_KORACH),
    S(HC_CHUKAT), S(HC_BALAK), S(HC_PINCHAS), S(HC_MATOT), S(HC_MASEI),
    S(HC_DEVARIM), S(HC_VAETCHANAN), S(HC_EIKEV), S(HC_REEH),
    S(HC_SHOFTIM), S(HC_KI_TEITZEI), S(HC_KI_TAVO), S(HC_NITZAVIM),
    SCH_END
};
/* H : Mon/Full/Sat — Diaspora (55) */
static const hc_reading SCH_F27_D[] = {
    S(HC_VAYEILECH), S(HC_HAAZINU), YT,
    S(HC_BEREISHIT), S(HC_NOACH), S(HC_LECH_LECHA), S(HC_VAYERA), S(HC_CHAYEI_SARAH),
    S(HC_TOLDOT), S(HC_VAYETZE), S(HC_VAYISHLACH), S(HC_VAYESHEV), S(HC_MIKETZ),
    S(HC_VAYIGASH), S(HC_VAYECHI), S(HC_SHEMOT), S(HC_VAERA), S(HC_BO), S(HC_BESHALACH),
    S(HC_YITRO), S(HC_MISHPATIM), S(HC_TERUMAH), S(HC_TETZAVEH), S(HC_KI_TISA),
    S(HC_VAYAKHEL), S(HC_PEKUDEI), S(HC_VAYIKRA), S(HC_TZAV),
    S(HC_SHEMINI), S(HC_TAZRIA), S(HC_METZORA), YT, YT,
    S(HC_ACHAREI_MOT), S(HC_KEDOSHIM), S(HC_EMOR), S(HC_BEHAR), S(HC_BECHUKOTAI),
    S(HC_BAMIDBAR), S(HC_NASO), S(HC_BEHAALOTECHA), S(HC_SHELACH), S(HC_KORACH),
    S(HC_CHUKAT), S(HC_BALAK), S(HC_PINCHAS), D(HC_MATOT,HC_MASEI),
    S(HC_DEVARIM), S(HC_VAETCHANAN), S(HC_EIKEV), S(HC_REEH),
    S(HC_SHOFTIM), S(HC_KI_TEITZEI), S(HC_KI_TAVO), S(HC_NITZAVIM),
    SCH_END
};
/* I : Mon/Short/Thu — Israel (54) */
static const hc_reading SCH_S25_I[] = {
    S(HC_VAYEILECH), S(HC_HAAZINU), YT,
    S(HC_BEREISHIT), S(HC_NOACH), S(HC_LECH_LECHA), S(HC_VAYERA), S(HC_CHAYEI_SARAH),
    S(HC_TOLDOT), S(HC_VAYETZE), S(HC_VAYISHLACH), S(HC_VAYESHEV), S(HC_MIKETZ),
    S(HC_VAYIGASH), S(HC_VAYECHI), S(HC_SHEMOT), S(HC_VAERA), S(HC_BO), S(HC_BESHALACH),
    S(HC_YITRO), S(HC_MISHPATIM), S(HC_TERUMAH), S(HC_TETZAVEH), S(HC_KI_TISA),
    S(HC_VAYAKHEL), S(HC_PEKUDEI), S(HC_VAYIKRA), S(HC_TZAV),
    S(HC_SHEMINI), S(HC_TAZRIA), S(HC_METZORA), YT,
    S(HC_ACHAREI_MOT), S(HC_KEDOSHIM), S(HC_EMOR), S(HC_BEHAR), S(HC_BECHUKOTAI),
    S(HC_BAMIDBAR), S(HC_NASO), S(HC_BEHAALOTECHA), S(HC_SHELACH), S(HC_KORACH),
    S(HC_CHUKAT), S(HC_BALAK), S(HC_PINCHAS), D(HC_MATOT,HC_MASEI),
    S(HC_DEVARIM), S(HC_VAETCHANAN), S(HC_EIKEV), S(HC_REEH),
    S(HC_SHOFTIM), S(HC_KI_TEITZEI), S(HC_KI_TAVO), D(HC_NITZAVIM,HC_VAYEILECH),
    SCH_END
};
/* I : Mon/Short/Thu — Diaspora (55) */
static const hc_reading SCH_S25_D[] = {
    S(HC_VAYEILECH), S(HC_HAAZINU), YT,
    S(HC_BEREISHIT), S(HC_NOACH), S(HC_LECH_LECHA), S(HC_VAYERA), S(HC_CHAYEI_SARAH),
    S(HC_TOLDOT), S(HC_VAYETZE), S(HC_VAYISHLACH), S(HC_VAYESHEV), S(HC_MIKETZ),
    S(HC_VAYIGASH), S(HC_VAYECHI), S(HC_SHEMOT), S(HC_VAERA), S(HC_BO), S(HC_BESHALACH),
    S(HC_YITRO), S(HC_MISHPATIM), S(HC_TERUMAH), S(HC_TETZAVEH), S(HC_KI_TISA),
    S(HC_VAYAKHEL), S(HC_PEKUDEI), S(HC_VAYIKRA), S(HC_TZAV),
    S(HC_SHEMINI), S(HC_TAZRIA), S(HC_METZORA), YT,
    S(HC_ACHAREI_MOT), S(HC_KEDOSHIM), S(HC_EMOR), S(HC_BEHAR), S(HC_BECHUKOTAI),
    S(HC_BAMIDBAR), YT,
    S(HC_NASO), S(HC_BEHAALOTECHA), S(HC_SHELACH), S(HC_KORACH), D(HC_CHUKAT,HC_BALAK),
    S(HC_PINCHAS), D(HC_MATOT,HC_MASEI), S(HC_DEVARIM), S(HC_VAETCHANAN), S(HC_EIKEV),
    S(HC_REEH), S(HC_SHOFTIM), S(HC_KI_TEITZEI), S(HC_KI_TAVO), D(HC_NITZAVIM,HC_VAYEILECH),
    SCH_END
};
/* J : Tue/Normal/Sat — Israel (54) */
static const hc_reading SCH_N37_I[] = {
    S(HC_VAYEILECH), S(HC_HAAZINU), YT,
    S(HC_BEREISHIT), S(HC_NOACH), S(HC_LECH_LECHA), S(HC_VAYERA), S(HC_CHAYEI_SARAH),
    S(HC_TOLDOT), S(HC_VAYETZE), S(HC_VAYISHLACH), S(HC_VAYESHEV), S(HC_MIKETZ),
    S(HC_VAYIGASH), S(HC_VAYECHI), S(HC_SHEMOT), S(HC_VAERA), S(HC_BO), S(HC_BESHALACH),
    S(HC_YITRO), S(HC_MISHPATIM), S(HC_TERUMAH), S(HC_TETZAVEH), S(HC_KI_TISA),
    S(HC_VAYAKHEL), S(HC_PEKUDEI), S(HC_VAYIKRA), S(HC_TZAV),
    S(HC_SHEMINI), S(HC_TAZRIA), S(HC_METZORA), YT,
    S(HC_ACHAREI_MOT), S(HC_KEDOSHIM), S(HC_EMOR), S(HC_BEHAR), S(HC_BECHUKOTAI),
    S(HC_BAMIDBAR), S(HC_NASO), S(HC_BEHAALOTECHA), S(HC_SHELACH), S(HC_KORACH),
    S(HC_CHUKAT), S(HC_BALAK), S(HC_PINCHAS), S(HC_MATOT), S(HC_MASEI),
    S(HC_DEVARIM), S(HC_VAETCHANAN), S(HC_EIKEV), S(HC_REEH),
    S(HC_SHOFTIM), S(HC_KI_TEITZEI), S(HC_KI_TAVO), S(HC_NITZAVIM),
    SCH_END
};
/* J : Tue/Normal/Sat — Diaspora (55) */
static const hc_reading SCH_N37_D[] = {
    S(HC_VAYEILECH), S(HC_HAAZINU), YT,
    S(HC_BEREISHIT), S(HC_NOACH), S(HC_LECH_LECHA), S(HC_VAYERA), S(HC_CHAYEI_SARAH),
    S(HC_TOLDOT), S(HC_VAYETZE), S(HC_VAYISHLACH), S(HC_VAYESHEV), S(HC_MIKETZ),
    S(HC_VAYIGASH), S(HC_VAYECHI), S(HC_SHEMOT), S(HC_VAERA), S(HC_BO), S(HC_BESHALACH),
    S(HC_YITRO), S(HC_MISHPATIM), S(HC_TERUMAH), S(HC_TETZAVEH), S(HC_KI_TISA),
    S(HC_VAYAKHEL), S(HC_PEKUDEI), S(HC_VAYIKRA), S(HC_TZAV),
    S(HC_SHEMINI), S(HC_TAZRIA), S(HC_METZORA), YT, YT,
    S(HC_ACHAREI_MOT), S(HC_KEDOSHIM), S(HC_EMOR), S(HC_BEHAR), S(HC_BECHUKOTAI),
    S(HC_BAMIDBAR), S(HC_NASO), S(HC_BEHAALOTECHA), S(HC_SHELACH), S(HC_KORACH),
    S(HC_CHUKAT), S(HC_BALAK), S(HC_PINCHAS), D(HC_MATOT,HC_MASEI),
    S(HC_DEVARIM), S(HC_VAETCHANAN), S(HC_EIKEV), S(HC_REEH),
    S(HC_SHOFTIM), S(HC_KI_TEITZEI), S(HC_KI_TAVO), S(HC_NITZAVIM),
    SCH_END
};
/* K : Thu/Full/Tue — both (55) */
static const hc_reading SCH_F53[] = {
    S(HC_HAAZINU), YT, YT,
    S(HC_BEREISHIT), S(HC_NOACH), S(HC_LECH_LECHA), S(HC_VAYERA), S(HC_CHAYEI_SARAH),
    S(HC_TOLDOT), S(HC_VAYETZE), S(HC_VAYISHLACH), S(HC_VAYESHEV), S(HC_MIKETZ),
    S(HC_VAYIGASH), S(HC_VAYECHI), S(HC_SHEMOT), S(HC_VAERA), S(HC_BO), S(HC_BESHALACH),
    S(HC_YITRO), S(HC_MISHPATIM), S(HC_TERUMAH), S(HC_TETZAVEH), S(HC_KI_TISA),
    S(HC_VAYAKHEL), S(HC_PEKUDEI), S(HC_VAYIKRA), S(HC_TZAV),
    S(HC_SHEMINI), S(HC_TAZRIA), S(HC_METZORA), S(HC_ACHAREI_MOT), YT,
    S(HC_KEDOSHIM), S(HC_EMOR), S(HC_BEHAR), S(HC_BECHUKOTAI),
    S(HC_BAMIDBAR), S(HC_NASO), S(HC_BEHAALOTECHA), S(HC_SHELACH), S(HC_KORACH),
    S(HC_CHUKAT), S(HC_BALAK), S(HC_PINCHAS), S(HC_MATOT), S(HC_MASEI),
    S(HC_DEVARIM), S(HC_VAETCHANAN), S(HC_EIKEV), S(HC_REEH),
    S(HC_SHOFTIM), S(HC_KI_TEITZEI), S(HC_KI_TAVO), D(HC_NITZAVIM,HC_VAYEILECH),
    SCH_END
};
/* L : Thu/Short/Sun — both (55) */
static const hc_reading SCH_S51[] = {
    S(HC_HAAZINU), YT, YT,
    S(HC_BEREISHIT), S(HC_NOACH), S(HC_LECH_LECHA), S(HC_VAYERA), S(HC_CHAYEI_SARAH),
    S(HC_TOLDOT), S(HC_VAYETZE), S(HC_VAYISHLACH), S(HC_VAYESHEV), S(HC_MIKETZ),
    S(HC_VAYIGASH), S(HC_VAYECHI), S(HC_SHEMOT), S(HC_VAERA), S(HC_BO), S(HC_BESHALACH),
    S(HC_YITRO), S(HC_MISHPATIM), S(HC_TERUMAH), S(HC_TETZAVEH), S(HC_KI_TISA),
    S(HC_VAYAKHEL), S(HC_PEKUDEI), S(HC_VAYIKRA), S(HC_TZAV),
    S(HC_SHEMINI), S(HC_TAZRIA), S(HC_METZORA), S(HC_ACHAREI_MOT), YT,
    S(HC_KEDOSHIM), S(HC_EMOR), S(HC_BEHAR), S(HC_BECHUKOTAI),
    S(HC_BAMIDBAR), S(HC_NASO), S(HC_BEHAALOTECHA), S(HC_SHELACH), S(HC_KORACH),
    S(HC_CHUKAT), S(HC_BALAK), S(HC_PINCHAS), S(HC_MATOT), S(HC_MASEI),
    S(HC_DEVARIM), S(HC_VAETCHANAN), S(HC_EIKEV), S(HC_REEH),
    S(HC_SHOFTIM), S(HC_KI_TEITZEI), S(HC_KI_TAVO), S(HC_NITZAVIM),
    SCH_END
};
/* M : Sat/Full/Thu — Israel (55) */
static const hc_reading SCH_F75_I[] = {
    YT, S(HC_HAAZINU), YT, YT,
    S(HC_BEREISHIT), S(HC_NOACH), S(HC_LECH_LECHA), S(HC_VAYERA), S(HC_CHAYEI_SARAH),
    S(HC_TOLDOT), S(HC_VAYETZE), S(HC_VAYISHLACH), S(HC_VAYESHEV), S(HC_MIKETZ),
    S(HC_VAYIGASH), S(HC_VAYECHI), S(HC_SHEMOT), S(HC_VAERA), S(HC_BO), S(HC_BESHALACH),
    S(HC_YITRO), S(HC_MISHPATIM), S(HC_TERUMAH), S(HC_TETZAVEH), S(HC_KI_TISA),
    S(HC_VAYAKHEL), S(HC_PEKUDEI), S(HC_VAYIKRA), S(HC_TZAV),
    S(HC_SHEMINI), S(HC_TAZRIA), S(HC_METZORA), YT,
    S(HC_ACHAREI_MOT), S(HC_KEDOSHIM), S(HC_EMOR), S(HC_BEHAR), S(HC_BECHUKOTAI),
    S(HC_BAMIDBAR), S(HC_NASO), S(HC_BEHAALOTECHA), S(HC_SHELACH), S(HC_KORACH),
    S(HC_CHUKAT), S(HC_BALAK), S(HC_PINCHAS), D(HC_MATOT,HC_MASEI),
    S(HC_DEVARIM), S(HC_VAETCHANAN), S(HC_EIKEV), S(HC_REEH),
    S(HC_SHOFTIM), S(HC_KI_TEITZEI), S(HC_KI_TAVO), D(HC_NITZAVIM,HC_VAYEILECH),
    SCH_END
};
/* M : Sat/Full/Thu — Diaspora (56) */
static const hc_reading SCH_F75_D[] = {
    YT, S(HC_HAAZINU), YT, YT,
    S(HC_BEREISHIT), S(HC_NOACH), S(HC_LECH_LECHA), S(HC_VAYERA), S(HC_CHAYEI_SARAH),
    S(HC_TOLDOT), S(HC_VAYETZE), S(HC_VAYISHLACH), S(HC_VAYESHEV), S(HC_MIKETZ),
    S(HC_VAYIGASH), S(HC_VAYECHI), S(HC_SHEMOT), S(HC_VAERA), S(HC_BO), S(HC_BESHALACH),
    S(HC_YITRO), S(HC_MISHPATIM), S(HC_TERUMAH), S(HC_TETZAVEH), S(HC_KI_TISA),
    S(HC_VAYAKHEL), S(HC_PEKUDEI), S(HC_VAYIKRA), S(HC_TZAV),
    S(HC_SHEMINI), S(HC_TAZRIA), S(HC_METZORA), YT,
    S(HC_ACHAREI_MOT), S(HC_KEDOSHIM), S(HC_EMOR), S(HC_BEHAR), S(HC_BECHUKOTAI),
    S(HC_BAMIDBAR), YT,
    S(HC_NASO), S(HC_BEHAALOTECHA), S(HC_SHELACH), S(HC_KORACH), D(HC_CHUKAT,HC_BALAK),
    S(HC_PINCHAS), D(HC_MATOT,HC_MASEI), S(HC_DEVARIM), S(HC_VAETCHANAN), S(HC_EIKEV),
    S(HC_REEH), S(HC_SHOFTIM), S(HC_KI_TEITZEI), S(HC_KI_TAVO), D(HC_NITZAVIM,HC_VAYEILECH),
    SCH_END
};
/* N : Sat/Short/Tue — both (55) */
static const hc_reading SCH_S73[] = {
    YT, S(HC_HAAZINU), YT, YT,
    S(HC_BEREISHIT), S(HC_NOACH), S(HC_LECH_LECHA), S(HC_VAYERA), S(HC_CHAYEI_SARAH),
    S(HC_TOLDOT), S(HC_VAYETZE), S(HC_VAYISHLACH), S(HC_VAYESHEV), S(HC_MIKETZ),
    S(HC_VAYIGASH), S(HC_VAYECHI), S(HC_SHEMOT), S(HC_VAERA), S(HC_BO), S(HC_BESHALACH),
    S(HC_YITRO), S(HC_MISHPATIM), S(HC_TERUMAH), S(HC_TETZAVEH), S(HC_KI_TISA),
    S(HC_VAYAKHEL), S(HC_PEKUDEI), S(HC_VAYIKRA), S(HC_TZAV),
    S(HC_SHEMINI), S(HC_TAZRIA), S(HC_METZORA), YT,
    S(HC_ACHAREI_MOT), S(HC_KEDOSHIM), S(HC_EMOR), S(HC_BEHAR), S(HC_BECHUKOTAI),
    S(HC_BAMIDBAR), S(HC_NASO), S(HC_BEHAALOTECHA), S(HC_SHELACH), S(HC_KORACH),
    S(HC_CHUKAT), S(HC_BALAK), S(HC_PINCHAS), D(HC_MATOT,HC_MASEI),
    S(HC_DEVARIM), S(HC_VAETCHANAN), S(HC_EIKEV), S(HC_REEH),
    S(HC_SHOFTIM), S(HC_KI_TEITZEI), S(HC_KI_TAVO), D(HC_NITZAVIM,HC_VAYEILECH),
    SCH_END
};

/* ── year-type dispatch table ─────────────────────────────────────────────── */

typedef struct {
    int rosh;           /* day of week of Rosh Hashana (1=Sun..7=Sat) */
    int year_type;      /* 0=SHORT 1=NORMAL 2=FULL */
    int pesach;         /* day of week of Pesach */
    int leap;           /* 0 or 1 */
    const hc_reading *sched_israel;
    const hc_reading *sched_diaspora;
} year_type_entry;

static const year_type_entry YEAR_TYPES[] = {
    /* regular years */
    {2, 2, 5, 0, SCH_F25_I,  SCH_F25_D},  /* A Mon/Full/Thu */
    {2, 0, 3, 0, SCH_F25_I,  SCH_F25_I},  /* B Mon/Short/Tue */
    {3, 1, 5, 0, SCH_F25_I,  SCH_F25_D},  /* C Tue/Normal/Thu */
    {5, 2, 1, 0, SCH_F51,    SCH_F51  },  /* D Thu/Full/Sun */
    {5, 1, 7, 0, SCH_N57_I,  SCH_N57_D},  /* E Thu/Normal/Sat */
    {7, 2, 3, 0, SCH_F73,    SCH_F73  },  /* F Sat/Full/Tue */
    {7, 0, 1, 0, SCH_S71,    SCH_S71  },  /* G Sat/Short/Sun */
    /* leap years */
    {2, 2, 7, 1, SCH_F27_I,  SCH_F27_D},  /* H Mon/Full/Sat */
    {2, 0, 5, 1, SCH_S25_I,  SCH_S25_D},  /* I Mon/Short/Thu */
    {3, 1, 7, 1, SCH_N37_I,  SCH_N37_D},  /* J Tue/Normal/Sat */
    {5, 2, 3, 1, SCH_F53,    SCH_F53  },  /* K Thu/Full/Tue */
    {5, 0, 1, 1, SCH_S51,    SCH_S51  },  /* L Thu/Short/Sun */
    {7, 2, 5, 1, SCH_F75_I,  SCH_F75_D},  /* M Sat/Full/Thu */
    {7, 0, 3, 1, SCH_S73,    SCH_S73  },  /* N Sat/Short/Tue */
};
#define NUM_YEAR_TYPES ((int)(sizeof(YEAR_TYPES)/sizeof(YEAR_TYPES[0])))

/* ── helpers ─────────────────────────────────────────────────────────────── */

/* Hebrew day-of-week: 1=Sun..7=Sat (same convention as Java getDayOfWeek) */
static int abs_day_to_dow(long abs_day)
{
    /* abs_day=1 is Monday; adjust so that result 1=Sun..7=Sat */
    int d = (int)(abs_day % 7);
    return d == 0 ? 7 : d;
}

/* abs day of Rosh Hashana — forward-declared from hebrew.c via internal linkage;
   we replicate the public API path instead */
static long rosh_hashana_abs(int year)
{
    hc_date d;
    d.calendar_type = HEBREW;
    d.year  = year;
    d.month = 7;
    d.day   = 1;
    /* hc_convert from HEBREW to HEBREW is a no-op; use abs_date through hc_cal_impl */
    hc_cal_impl *impl = get_calendar(HEBREW);
    return impl->abs_date(year, 7, 1);
}

static int heb_year_type_ordinal(int year)
{
    /* 0=SHORT 1=NORMAL 2=FULL */
    return (int)hc_get_heb_year_type(year);
}

static int is_leap(int year)
{
    return hc_is_leap_year(year, HEBREW);
}

/* First Saturday on or after Rosh Hashana */
static long first_shabbat_of_year(int year)
{
    long rh = rosh_hashana_abs(year);
    int  dow = abs_day_to_dow(rh); /* 1=Sun..7=Sat */
    long offset = (dow == 7) ? 0 : (7 - dow);
    return rh + offset;
}

/* Count of entries in a schedule (not counting SCH_END) */
static int schedule_length(const hc_reading *sch)
{
    int n = 0;
    while (sch[n].p1 != (hc_parsha)0xFF) n++;
    return n;
}

/* ── public API ──────────────────────────────────────────────────────────── */

int hc_get_parsha(hc_date *date, int in_israel, hc_reading *reading)
{
    if (!date || !reading) return -1;

    /* convert to Hebrew to get the year */
    hc_date hdate = *date;
    if (hdate.calendar_type != HEBREW) {
        if (hc_convert(&hdate, HEBREW) != 0) return -1;
    }

    /* get the absolute day of the input date */
    hc_cal_impl *impl = get_calendar(date->calendar_type);
    long abs_input = impl->abs_date(date->year, date->month, date->day);

    /* must be a Saturday */
    if (abs_day_to_dow(abs_input) != 7) return -1;

    int year = hdate.year;
    long first_shab = first_shabbat_of_year(year);

    /* if this Shabbat is before the first Shabbat of the current year, use previous year */
    if (abs_input < first_shab) {
        year--;
        first_shab = first_shabbat_of_year(year);
    }

    long week_offset = (abs_input - first_shab) / 7;

    /* find the year-type entry */
    int rosh_dow  = abs_day_to_dow(rosh_hashana_abs(year));
    int yt        = heb_year_type_ordinal(year);
    int pesach_dow= abs_day_to_dow(rosh_hashana_abs(year) +
                        /* Nisan 15 offset computed via hc_cal_impl */
                        (get_calendar(HEBREW)->abs_date(year, 1, 15) - rosh_hashana_abs(year)));
    /* simpler: abs of Nisan 15 */
    long abs_pesach = get_calendar(HEBREW)->abs_date(year, 1, 15);
    pesach_dow = abs_day_to_dow(abs_pesach);
    int leap = is_leap(year);

    const hc_reading *sch = NULL;
    for (int i = 0; i < NUM_YEAR_TYPES; i++) {
        year_type_entry e = YEAR_TYPES[i];
        if (e.rosh == rosh_dow && e.year_type == yt && e.pesach == pesach_dow && e.leap == leap) {
            sch = in_israel ? e.sched_israel : e.sched_diaspora;
            break;
        }
    }
    if (!sch) return -1;

    int len = schedule_length(sch);
    if (week_offset < 0 || week_offset >= len) {
        reading->p1 = HC_PARSHA_NONE;
        reading->p2 = HC_PARSHA_NONE;
        return 0;
    }

    *reading = sch[week_offset];
    return 0;
}
