#include <stdio.h>
#include "hconverter.h"
#include "hc_internal.h"

int main()
{
	int i=100;
	int yr, mh, dy;

	hc_date date;
	hc_heb_time time;

	while (i) {

		printf("\n Enter query type to test one of the following functions, 0 to exit:\n1. gregAbs\n2. hebAbs\n3. gregDate");
		printf("\n4. hebDate\n5. isGregLeap\n6. isHebLeap\n7. yearType\n8. molad\n9. rosh\n10. multParts\n\n ====> ");

		scanf("%d",&i);

		switch(i) {
		case 1:
			printf("\n Enter Gregorian year: ");
			scanf("%d",&yr);
			printf(" month: ");
			scanf("%d",&mh);
			printf(" day: ");
			scanf("%d",&dy);

			printf("%s", "\n\n Absolute date ");
			printf("%lu", greg_impl->abs_date(yr, mh, dy));
			printf("%s","\n");
			break;
		case 2:
			printf("\n Enter Hebrew year: ");
			scanf("%d",&yr);
			printf(" month: ");
			scanf("%d",&mh);
			printf(" day: ");
			scanf("%d",&dy);

			printf("%s", "\n\n Absolute date ");
			printf("%lu", heb_impl->abs_date(yr,mh,dy));
			printf("%s","\n");
			break;
		case 3:
			printf("\nEnter absolute day: ");
			scanf("%d",&dy);
			greg_impl->compute_date(dy, &date);

			printf("%s%d%s%d%s%d%s", "Gregorian date (YMD): ", date.year,"-",date.month,"-",date.day,"\n");
			break;
		case 4:
			printf("\nEnter absolute day: ");
			scanf("%d",&dy);
			heb_impl->compute_date(dy, &date);
			printf("%s%d%s%d%s%d%s","Hebrew date (YMD): ", date.year, "-", date.month,"-", date.day,"\n");
			break;
		case 5:
			printf("\n Enter Gregorian year: ");
			scanf("%d",&yr);
			printf("%s%d%s","is_leap value ", greg_impl->is_leap_year(yr),"\n");
			break;
		case 6:
			printf("\n Enter Hebrew year: ");
			scanf("%d",&yr);
			printf("%s%d%s","isHebLeap value ", heb_impl->is_leap_year(yr),"\n");
			break;
		case 7:
			printf("\n Enter Hebrew year: ");
			scanf("%d",&yr);
			printf("%s%d%s","yearType value ", hc_get_heb_year_type(yr),"\n");
			break;
		case 8:
			printf("\n Enter Hebrew year: ");
			scanf("%d",&yr);
			hc_compute_molad_rosh_hashana(yr, GREGORIAN, &date, &time);
			printf("%s%d%s%d%s%d%s%d%s%d%s","Molad value: day ", date.year,
					"-", date.month,
					"-", date.day,
					" hour ", time.hour,
					" part ", time.part,
					"\n");
			break;
		case 9:
			printf("\n Enter Hebrew year: ");
			scanf("%d", &yr);
			set_hc_date(&date, HEBREW, yr, 7, 1);
			long absd = heb_impl->abs_date(yr, 7, 1);
			hc_convert(&date, GREGORIAN);
			printf("%s%ld%s","abs day ", absd,"\n");
			printf("%s%d%s%d%s%d%s","Greg day ", date.year,
					"-", date.month,
					"-", date.day,
					"\n");
			break;
		}
	}
}
