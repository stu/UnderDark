#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include "strings.h"

uint8_t *get_current_datetime(void)
{
	char buff[32];
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);

	snprintf(buff, 32, "%04u%02u%02u%02u%02u",
			 tm->tm_year + 1900,
			 tm->tm_mon + 1,
			 tm->tm_mday,
			 tm->tm_hour,
			 tm->tm_min
	);

	return (uint8_t *)strdup(buff);
}

uint8_t *datetime_tostring(char *dt)
{
	char buff[80];
	char *months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

	int16_t y;
	int16_t mth;
	int16_t d;
	int16_t h;
	int16_t mn;

	y = 1986;
	mth = 1;
	d = 1;
	h = 8;
	mn = 0;

	if(dt != NULL && strlen(dt) == 12 && only_numbers(dt) == true)
	{
		y = get_number_in_string(dt, 4);
		mth = get_number_in_string(dt + 4, 2);
		d = get_number_in_string(dt + 6, 2);
		h = get_number_in_string(dt + 8, 2);
		mn = get_number_in_string(dt + 10, 2);
	}

	sprintf(buff, "%s %u %u %u:%02u%s", months[mth - 1], d, y, h >= 13 ? h - 12 : h, mn, h >= 13 ? "pm" : "am");

	return (uint8_t *)strdup(buff);
}
