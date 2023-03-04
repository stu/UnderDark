#include "headers.h"


const char base64_encoding[] = "0123456789.ABCDEFGHIJKLMNOPQRSTUVWXYZ-abcdefghijklmnopqrstuvwxyz";


static uint8_t dec64(uint8_t v)
{
	if(v >= '0' && v <= '9')
	{
		return v - '0';
	}
	else if(v == '.')
	{
		return 10;
	}
	else if(v >= 'A' && v <= 'Z')
	{
		return (v - 'A') + 11;
	}
	else if(v == '-')
	{
		return 37;
	}
	else if(v >= 'a' && v <= 'z')
	{
		return (v - 'a') + 38;
	}

	return 0;
}

void decode64(const char *str, uint8_t *buff, int16_t bufflen)
{
	uint8_t a, b, c, d;
	uint32_t v;

	uint16_t i;
	memset(buff, 0, bufflen);

	i = 0;
	while(*str != 0)// && i < bufflen)
	{
		a = 0;
		b = 0;
		c = 0;
		d = 0;
		v = 0;

		assert(i < bufflen);

		if(*str != 0)
		{
			a = dec64(*str++);
		}
		if(*str != 0)
		{
			b = dec64(*str++);
		}
		if(*str != 0)
		{
			c = dec64(*str++);
		}
		if(*str != 0)
		{
			d = dec64(*str++);
		}

		v |= a;

		v <<= 6;
		v |= b;

		v <<= 6;
		v |= c;

		v <<= 6;
		v |= d;

		// --aaaaaa --bbbbbb --cccccc --dddddd
		// -------- aaaaaabb bbbbcccc ccdddddd

		if(i + 1 < bufflen - 1)
		{
			buff[i++] = (v >> 16) & 0xFF;
		}
		if(i + 1 < bufflen - 1)
		{
			buff[i++] = (v >> 8) & 0xFF;
		}
		if(i + 1 < bufflen - 1)
		{
			buff[i++] = (v >> 0) & 0xFF;
		}
	}
}

void encode64(const uint8_t *str, uint16_t slen, char *buff, int16_t bufflen)
{
	uint8_t a, b, c, d;
	uint32_t x;
	uint16_t i;
	int16_t qlen;

	bool has_a, has_b, has_c, has_d;

	memset(buff, 0, bufflen);

	i = 0;
	qlen = 0;
	while(qlen < slen)
	{
		has_a = false;
		has_b = false;
		has_c = false;
		has_d = false;

		x = 0;
		if(qlen < slen)
		{
			x = *str++;
			qlen += 1;
			has_a = true;
			has_b = true;
		}

		x <<= 8;
		if(qlen < slen)
		{
			x |= *str++;
			qlen += 1;
			has_b = true;
			has_c = true;
		}

		x <<= 8;
		if(qlen < slen)
		{
			x |= *str++;
			qlen += 1;
			has_c = true;
			has_d = true;
		}

		// -------- xxxxxxxx yyyyyyyy zzzzzzzz
		// --xxxxxx --xxyyyy --yyyyzz --zzzzzz
		a = (x >> 18) & 0x3F;
		b = (x >> 12) & 0x3F;
		c = (x >> 6) & 0x3F;
		d = (x >> 0) & 0x3F;

		assert(i < bufflen);

		if(i <= bufflen - 1 && has_a == true)
		{
			buff[i++] = base64_encoding[a];
		}

		if(i <= bufflen - 1 && has_b == true)
		{
			buff[i++] = base64_encoding[b];
		}

		if(i <= bufflen - 1 && has_c == true)
		{
			buff[i++] = base64_encoding[c];
		}

		if(i <= bufflen - 1 && has_d == true)
		{
			buff[i++] = base64_encoding[d];
		}

	}
}


void ClearSaveFiles(void)
{
	uint16_t i;
	char fn[32];
	char *fnx;
	if(workstationCactus == true)
		return;
	fnx = GetConfigurationFile(FILE_PLAYER);
	remove(fnx);
	free(fnx);

	fnx = GetConfigurationFile(FILE_CORE);
	remove(fnx);
	free(fnx);

	for(i=eFlag_GenLevel0; i < eFlag_GenLevel_Bottom; i++)
	{
		sprintf(fn, "z_map%03u.sav", i);
		fnx = GetConfigurationFile(fn);
		remove(fnx);
		free(fnx);
	}
}


bool verify_file(char *fn)
{
	FILE *fp;

	fp = fopen(fn, "rb");
	if(fp != NULL)
	{
		fclose(fp);
		return true;
	}

	return false;
}


//static int8_t get_i8(INIFILE *ini, char *group, char *key, bool *rc)
//{
//	char *p;
//	int8_t value = 0;
//
//	*rc = false;
//
//	p = INI_get(ini, group, key);
//	if(p != NULL)
//	{
//		value = strtol(p, NULL, 10);
//		*rc = true;
//	}
//
//	return value;
//}

uint8_t get_u8(INIFILE *ini, char *group, char *key, bool *rc)
{
	char *p;
	uint8_t value = 0;

	*rc = false;

	p = INI_get(ini, group, key);
	if(p != NULL)
	{
		value = strtoul(p, NULL, 10);
		*rc = true;
	}

	return value;
}

int16_t get_i16(INIFILE *ini, char *group, char *key, bool *rc)
{
	char *p;
	int16_t value = 0;

	*rc = false;

	p = INI_get(ini, group, key);
	if(p != NULL)
	{
		value = strtol(p, NULL, 10);
		*rc = true;
	}

	return value;
}

bool load_rle(INIFILE *ini, char *group, char *key, uint8_t *out_buff, int16_t out_len)
{
	char *p;
	uint8_t *x;
	bool rc = false;
	int16_t c = out_len;

	if(c < 2048)
		c = 2048;

	memset(out_buff, 0x0, out_len);

	p = INI_get(ini, group, key);
	if(p != NULL)
	{
		x = calloc(1, c);
		if(x != NULL)
		{
			decode64(p, x, c);
			unpack_rle(x, out_len, out_buff);
			rc = true;
		}
	}

	return rc;
}


bool load_ue64(INIFILE *ini, char *group, char *key, uint8_t *out_buff, int16_t out_len)
{
	char *p;
	uint8_t *x;
	int16_t c = out_len;
	bool rc = false;

	if(c < 2048)
		c = 2048;

	memset(out_buff, 0x0, out_len);
	p = INI_get(ini, group, key);
	if(p != NULL)
	{
		x = calloc(1, c);
		if(x != NULL)
		{
			decode64(p, out_buff, out_len);
			rc = true;
		}
	}

	return rc;
}

uint16_t get_u16(INIFILE *ini, char *group, char *key, bool *rc)
{
	char *p;
	uint16_t value = 0;

	*rc = false;

	p = INI_get(ini, group, key);
	if(p != NULL)
	{
		value = strtoul(p, NULL, 10);
		*rc = true;
	}

	return value;
}

uint32_t get_u32(INIFILE *ini, char *group, char *key, bool *rc)
{
	char *p;
	uint32_t value = 0;

	*rc = false;

	p = INI_get(ini, group, key);
	if(p != NULL)
	{
		value = strtoul(p, NULL, 10);
		*rc = true;
	}

	return value;
}

static uint16_t count_run(uint8_t *data, uint16_t max)
{
	uint16_t q = 0;

	while(q < max)
	{
		if(data[q] != data[0])
			return q;

		q += 1;
	}

	return q;
}

void unpack_rle(const uint8_t *src, uint16_t dlen, uint8_t *dest)
{
	uint16_t idx;
	uint8_t c;

	memset(dest, 0x0, dlen);

	idx = 0;
	while(idx < dlen)
	{
		c = *src;
		src += 1;

		if(c >= 0x80)
		{
			c = (c&0x7F) + 1;

			while(c > 0)
			{
				dest[idx] = *src;
				idx += 1;
				c -= 1;
			}

			src += 1;
		}
		else
		{
			c += 1;
			memmove(dest+idx, src, c);
			idx += c;
			src += c;
		}
	}
}

uint16_t rle_pack_line(uint8_t *dest, uint16_t dest_len, uint8_t *src, uint16_t src_len)
{
	uint16_t count;
	uint16_t data;
	uint16_t runc;
	uint16_t c;

	memset(dest, 0x0, dest_len);

	count = 0;
	data = count + 1;
	c = 0;

	while(c < src_len && data < dest_len)
	{
		runc = count_run(src + c, src_len - c);

		// cant have a run of zero.. so -1 our run!
		if(runc > 0x80)
		{
			runc = 0x80;
		}

		// takes 2 bytes to encode a run, so do a min of 3...
		if(runc > 3)
		{
			if(dest[count] != 0)
			{
				// close off count
				dest[count] -= 1;

				// we have previous run
				// close of previous run
				count = data;
				data += 1;
			}

			dest[count] = 0x80 | (runc-1);
			dest[data] = src[c];
			count += 2;
			data = count + 1;

			c += runc;

			dest[count] = 0;
		}
		else
		{
			dest[count] += 1;
			dest[data++] = src[c++];

			// do we need to start a new run counter
			if(dest[count] == 0x80)
			{
				// close off count!
				dest[count] -= 1;

				count = data;
				data = count + 1;
				dest[count] = 0;
			}
		}
	}

	if(dest[count] == 0 && (count + 1 == data))
	{
		// remove last counter byte (with zero data)
		data -= 1;
	}
	else
	{
		// non run, close off count!
		dest[count] -= 1;
	}

	return data;
}


void SetLoadSaveStatus(bool loading, char *s)
{
	toggle_swap_draw_to_backbuffer();

	//screen_line_clear(screen_get_height()-1, 0x7);
	if(loading == true)
		screen_print(screen_get_height()-1, 0, 0x0E, (uint8_t*)"Loading :");
	else
		screen_print(screen_get_height()-1, 0, 0x0E, (uint8_t*)" Saving :");

	screen_print(screen_get_height()-1, 10, 0x0E, (uint8_t*)s);
	// less flicker than clearning whole line and drawing text
	screen_print(screen_get_height()-1, 10 + strlen(s), 0x0E, (uint8_t*)"            ");

	toggle_swap_draw_to_backbuffer();
	restore_from_backbuffer();
}


void save_str(INIFILE *ini, char *group, char *key, uint8_t *s)
{
	char qq[1024];

	encode64(s, strlen((char*)s), qq, 1024);
	INI_UpdateItem(ini, group, key, qq);
}

void save_i16(INIFILE *ini, char *group, char *key, int16_t value)
{
	char qq[32];

	sprintf(qq, "%"PRIi16"", value);
	INI_UpdateItem(ini, group, key, qq);
}

void save_u8(INIFILE *ini, char *group, char *key, uint8_t value)
{
	char qq[32];
	sprintf(qq, "%"PRIu8"", value);
	INI_UpdateItem(ini, group, key, qq);
}

void save_u16(INIFILE *ini, char *group, char *key, uint16_t value)
{
	char qq[32];
	sprintf(qq, "%"PRIu16"", value);
	INI_UpdateItem(ini, group, key, qq);
}

void save_u32(INIFILE *ini, char *group, char *key, uint32_t value)
{
	char qq[32];
	sprintf(qq, "%"PRIu32"", value);
	INI_UpdateItem(ini, group, key, qq);
}
