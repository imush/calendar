import net.hebrewcalendar.*;
import java.time.*;
import java.time.temporal.*;
import java.util.*;

/**
 * Java oracle for cross-validation against the C library.
 * Outputs one line per SD check and one per ZM check in a stable format
 * that the C cross_validate binary can be diffed against.
 *
 * Format:
 *   SD YYYY-MM-DD {D|I} NAME1,NAME2,...|-
 *   ZM YYYY-MM-DD LOCNAME ha sa al tz ch ss t3
 *
 * Times are minutes from local-standard-time midnight (matching C's tz_offset_h parameter).
 * "N" is output for polar unavailable times.
 */
public class CrossValidate {

    /* { name, lat, lon, tz_h, inIsrael, inJerusalem } */
    static final Object[][] LOCS = {
        { "JER",  31.7767,   35.2345,  2, true,  true  },
        { "NYC",  40.7128,  -74.0060, -5, false, false  },
        { "LAX",  34.0522, -118.2437, -8, false, false  },
        { "MOW",  55.7558,   37.6173,  3, false, false  },
        { "LED",  59.9311,   30.3609,  3, false, false  },
        { "PBY",  70.2553, -148.3375, -9, false, false  },
    };

    public static void main(String[] args) {
        LocalDate start = LocalDate.of(1950, 1, 1);
        LocalDate end   = LocalDate.of(2051, 1, 1);

        StringBuilder sb = new StringBuilder(128);

        for (LocalDate ld = start; ld.isBefore(end); ld = ld.plusDays(1)) {
            IDate greg = ICalendar.GREGORIAN.fromYMD(
                    ld.getYear(), ld.getMonthValue(), ld.getDayOfMonth());
            String dateStr = ld.toString(); // YYYY-MM-DD

            // ── Special days ─────────────────────────────────────────────────
            for (int isr = 0; isr <= 1; isr++) {
                boolean inIsrael = isr == 1;
                List<String> names = new ArrayList<>();
                for (JewishSpecialDay sd : JewishSpecialDay.values()) {
                    if (sd.applies(inIsrael) && sd.matches(greg)) {
                        names.add(sd.name());
                    }
                }
                Collections.sort(names);
                sb.setLength(0);
                sb.append("SD ").append(dateStr).append(' ')
                  .append(inIsrael ? 'I' : 'D').append(' ')
                  .append(names.isEmpty() ? "-" : String.join(",", names));
                System.out.println(sb);
            }

            // ── Zmanim ───────────────────────────────────────────────────────
            for (Object[] loc : LOCS) {
                String  name        = (String)  loc[0];
                double  lat         = (double)  loc[1];
                double  lon         = (double)  loc[2];
                int     tzH         = (int)     loc[3];
                boolean inIsrael    = (boolean) loc[4];
                boolean inJerusalem = (boolean) loc[5];

                ZoneOffset tz = ZoneOffset.ofHours(tzH);
                Location location = new Location(lat, lon, 0.0, tz.getId(), inIsrael, inJerusalem);
                Zmanim zm = new Zmanim(ld, location);
                ZonedDateTime midnight = ld.atStartOfDay(tz);

                sb.setLength(0);
                sb.append("ZM ").append(dateStr).append(' ').append(name).append(' ')
                  .append(fmt(zm.getSunrise().getTime(), midnight))            .append(' ')
                  .append(fmt(zm.getSunset().getTime(), midnight))             .append(' ')
                  .append(fmt(zm.getDawn().getTime(), midnight))                .append(' ')
                  .append(fmt(zm.getNightfallAlterRebbe().getTime(), midnight)).append(' ')
                  .append(fmt(zm.getChatzot().getTime(), midnight))            .append(' ')
                  .append(fmt(zm.getLatestShema().getTime(), midnight))        .append(' ')
                  .append(fmt(zm.getNightfallMediumStars().getTime(), midnight));
                System.out.println(sb);
            }
        }
    }

    static String fmt(ZonedDateTime t, ZonedDateTime localMidnight) {
        if (t == null) return "N";
        long secs = ChronoUnit.SECONDS.between(localMidnight, t);
        return String.valueOf((int) Math.round(secs / 60.0));
    }
}
