#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "hconverter.h"
#include "hc_internal.h"

static char **cmd_tokenize(char *cmd)
{
	char** ret = (char **)malloc(7 * sizeof(char));
	int j;

	for (char* p = cmd; *p; p++) {
		*p = tolower(*p);
		if (*p == '\n')
			*p = '\0';
	}

	ret[0] = strtok(cmd, " .-");
	for (j = 1; j < 7; j++) {
		ret[j] = strtok(NULL, " .-");
	}
	return ret;
}

static hc_calendar_type parse_cal_type(char *str) {
	if (strcmp(str, "hebrew") == 0 || strcmp(str, "h") == 0)
		return HEBREW;
	if (strcmp(str, "gregorian") == 0 || strcmp(str, "g") == 0)
		return GREGORIAN;
	if (strcmp(str, "julian") == 0 || strcmp(str, "j") == 0)
		return JULIAN;

	return NONE;
}

static char* dow_string(const int dow)
{
	switch(dow) {
	case SATURDAY: return "SATURDAY";
	case SUNDAY: return "SUNDAY";
	case MONDAY: return "MONDAY";
	case TUESDAY: return "TUESDAY";
	case WEDNESDAY: return "WEDNESDAY";
	case THURSDAY: return "THURSDAY";
	case FRIDAY: return "FRIDAY";
	default: return "NULL";
	}
}

static char* heb_type_string(const int t)
{
	switch(t) {
	case SHORT_HEB_YEAR: return "SHORT";
	case FULL_HEB_YEAR: return "FULL";
	case NORMAL_HEB_YEAR: return "REGULAR";
	default: return "NULL";
	}
}

static int run_cmd(char** cmd_tokenized)
{
	hc_calendar_type convert_from, convert_to;
	char *cmd = cmd_tokenized[0];
	int year, month, day;
	hc_date d;
	heb_time t;


	if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0 || strcmp(cmd, "q") == 0)
		exit(0);

	if (strcmp(cmd, "isleap") == 0 || strcmp(cmd, "is_leap") == 0) {
		if (cmd_tokenized[1] == NULL)
			convert_to = HEBREW;
		else
			convert_to = parse_cal_type(cmd_tokenized[1]);
		if (convert_to == NONE)
			return -1;

		if (cmd_tokenized[2] == NULL)
			return -1;
		year = strtol(cmd_tokenized[2], NULL, 0);
		printf("%d", get_calendar(convert_to)->is_leap_year(year));
		return 0;
	}

	if (strcmp(cmd, "type") == 0 || strcmp(cmd, "t") == 0) {
		if (cmd_tokenized[1] == NULL)
			return -1;
		year = strtol(cmd_tokenized[1], NULL, 0);
		printf("%d", hc_get_heb_year_type(year));
		return 0;
	}

	if (strcmp(cmd, "keviut") == 0 || strcmp(cmd, "k") == 0 ||
			strcmp(cmd, "kevius") == 0) {
		int leap, rh, pesach, ck;
		if (cmd_tokenized[1] == NULL)
			return -1;
		year = strtol(cmd_tokenized[1], NULL, 0);
		hc_compute_keviut(year, &rh, &pesach, &ck, &leap);
		printf("Rosh Hashana %s, Pesach %s, Cheshvan/Kislev %s, leap %s",
			dow_string(rh), dow_string(pesach), heb_type_string(ck), leap ? "YES" : "NO");
		return 0;
	}

	if (strcmp(cmd, "molad") == 0 || strcmp(cmd, "m") == 0) {
		if (cmd_tokenized[1] == NULL)
			convert_to = GREGORIAN;
		else
			convert_to = parse_cal_type(cmd_tokenized[1]);
		if (convert_to == NONE)
			return -1;

		if (cmd_tokenized[2] == NULL)
			return -1;
		year = strtol(cmd_tokenized[2], NULL, 0);
		if (cmd_tokenized[3] == NULL)
			month = 7;
		else
			month = strtol(cmd_tokenized[3], NULL, 0);
		hc_compute_molad(year, month, convert_to, &d, &t);
		printf("%4d-%02d-%02d %02d:%04d", d.year, d.month, d.day, t.hour, t.part);
		return 0;
	}

	if (strcmp(cmd, "convert") == 0 || strcmp(cmd, "c") == 0) {
		if (cmd_tokenized[1] == NULL || (convert_from = parse_cal_type(cmd_tokenized[1])) == NONE)
			return -1;

		if (cmd_tokenized[2] == NULL || (convert_to = parse_cal_type(cmd_tokenized[2])) == NONE)
			return -1;

		if (cmd_tokenized[3] == NULL || (year = strtol(cmd_tokenized[3], NULL, 0)) < 1)
		 	return -1;
		if (cmd_tokenized[4] == NULL || (month = strtol(cmd_tokenized[4], NULL, 0)) < 1)
		 	return -1;
		if (cmd_tokenized[5] == NULL || (day = strtol(cmd_tokenized[5], NULL, 0)) < 1)
		 	return -1;
		d.calendar_type = convert_from;
		d.day = day;
		d.year = year;
		d.month = month;
		if (!hc_check(&d)) {
			printf("Invalid date");
			return -1;
		}
		hc_convert(&d, convert_to);
		printf("%4d-%02d-%02d", d.year, d.month, d.day);
		return 0;
	}

	if (strcmp(cmd, "absolute") == 0 || strcmp(cmd, "a") == 0 || strcmp(cmd, "abs") == 0) {
		hc_cal_impl* cal;
		long a;
		if (cmd_tokenized[1] == NULL || (convert_from = parse_cal_type(cmd_tokenized[1])) == NONE)
			return -1;

		if (cmd_tokenized[2] == NULL || (year = strtol(cmd_tokenized[2], NULL, 0)) < 1)
		 	return -1;
		if (cmd_tokenized[3] == NULL || (month = strtol(cmd_tokenized[3], NULL, 0)) < 1)
		 	return -1;
		if (cmd_tokenized[4] == NULL || (day = strtol(cmd_tokenized[4], NULL, 0)) < 1)
		 	return -1;
		cal = get_calendar(convert_from);
		a = cal->abs_date(year, month, day);
		printf("Absolute day: %lu", a);
		return 0;
	}

	return -1;
}

int main(int argc, char **argv)
{
	char ** cmd_tokenized = argv;

	if (argc < 2 || strcmp(argv[1], "-i") == 0) {
		char *cmd = malloc(1081*sizeof(char));
		while(1) {
			printf("Enter command: ");
			fgets(cmd, 1080, stdin);
			cmd_tokenized = cmd_tokenize(cmd);
			run_cmd(cmd_tokenized);
			free(cmd_tokenized);
			printf("\n");
		}
		free(cmd);
	} else {
		cmd_tokenized = malloc(7*sizeof(char*));
		memcpy(cmd_tokenized, argv+1, 7 * sizeof(char*));
		run_cmd(cmd_tokenized);
		printf("\n");
		free(cmd_tokenized);
	}
}
