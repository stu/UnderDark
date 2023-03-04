#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

char *unspace(char *s)
{
	while(*s == ' ' || *s == '\t')
		s++;

	return s;
}

void bit_set_bit(uint8_t *ptr, size_t pos, bool state)
{
	uint8_t mask = 0x80;

	mask >>= (pos & 0x7);
	ptr += (pos >> 3);

	if(state == true)
	{
		ptr[0] |= mask;
	}
	else
	{
		ptr[0] &= ~mask;
	}

}

bool bit_get_bit(uint8_t *ptr, size_t pos)
{
	uint8_t mask;
	uint8_t c;

	ptr += (pos >> 3);

	c = pos & 0x7;
	mask = 0x80;
	mask >>= c;

	c = *ptr;
	c &= mask;

	if(c == 0)
	{
		return false;
	}

	return true;
}


bool only_numbers(char *s)
{
	char *q = s;

	if(s == NULL)
		return false;

	while(*q != 0)
	{
		// only numbers!
		if(*q < 0x30 || *q > 0x39)
			return false;

		q++;
	}

	return true;
}

uint32_t get_number_in_string(char *s, int16_t len)
{
	char buff[16];

	memset(buff, 0x0, 16);
	memmove(buff, s, len);

	return strtoul(buff, NULL, 10);
}

bool StringEndsInS(char *text)
{
	char *p;

	if(text == NULL)
		return false;

	p = strchr(text, 0);
	if(strlen(text) > 1)
		p -= 1;

	if(*p == 's')
		return true;

	return false;
}
