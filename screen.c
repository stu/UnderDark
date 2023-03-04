#ifndef __linux__
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>
#include <string.h>
#include <i86.h>

#include "screen.h"

#ifdef __386__
uint8_t *p_screen = NULL;
uint8_t *p_backbuffer = NULL;
#else
uint8_t far
*
p_screen = NULL;
uint8_t far
*
p_backbuffer = NULL;
#endif

uint16_t screen_width;
uint16_t screen_height;

uint8_t cga_mda_flag;
uint8_t start_mode;

extern void set_curpos(uint8_t row, uint8_t col);

extern uint16_t xget_key(void);
#pragma aux xget_key = \
	"xor ax,ax" \
	"int 0x16" \
	value [ax];

#pragma aux set_curpos = \
    "mov ah,2" \
    "xor bx,bx" \
    "int 0x10" \
    parm[dh][dl] \
    modify[ax bx cx dx];

void set_cursor_position(uint16_t r, uint16_t c)
{
	set_curpos(r, c);
}

extern void set_video_mode_80x25(void);

#pragma aux set_video_mode_80x25 = \
    "mov ax,3"    \
    "int 0x10"        \
    modify[ax];

extern void set_video_mode_80x28(void);

#pragma aux set_video_mode_80x28 = \
    "mov ax,0x1202" \
    "mov bx,0x30" \
    "int 0x10" \
    "mov ax,3" \
    "int 0x10" \
    "mov ax,0x1111"    \
    "xor bx,bx"        \
    "int 0x10"        \
    modify[ax bx];

extern void set_video_mode_80x50(void);

#pragma aux set_video_mode_80x50 = \
    "mov ax,0x1202" \
    "mov bx,0x30" \
    "int 0x10" \
    "mov ax,0x3"    \
    "int 0x10"        \
    "mov ax,0x1112"    \
    "xor bx,bx"        \
    "int 0x10"        \
    modify[ax bx cx dx];

extern void set_video_mode_80x43(void);

#pragma aux set_video_mode_80x43 = \
    "mov ax,0x0003"    \
    "int 0x10"        \
    "mov ax,0x1114"    \
    "xor bx,bx"        \
    "int 0x10"        \
    "mov ax,0x1112"    \
    "xor bx,bx"        \
    "int 0x10"        \
    "mov ax,0x1201" \
    "mov bx,0x30"    \
    "int 0x10"        \
    "mov ax,0x0003"    \
    "int 0x10"        \
    "mov ax,0x1112"    \
    "xor bx,bx"        \
    "int 0x10"        \
    modify[ax bx];


void screen_set_startmode(uint8_t c)
{
	start_mode = c;
}

void screen_we_have_cga_mda(uint8_t c)
{
	cga_mda_flag = c;
}

void ScreenChangeMode(uint8_t c)
{
#ifdef __386__
	uint8_t *low_seg = (uint8_t *)0x400;
	p_screen = (uint8_t *)0xB8000;
#else
	uint8_t
	far *low_seg = MK_FP(0x40, 0);
	p_screen = MK_FP(0xB800, 0);
#endif

	switch(c)
	{
		case SCREEN_MODE_DEFAULT:
			break;

		case SCREEN_MODE_80X25:
			set_video_mode_80x25();
			break;

		case SCREEN_MODE_80X28:
			set_video_mode_80x28();
			break;

		case SCREEN_MODE_80X43:
			set_video_mode_80x43();
			break;

		case SCREEN_MODE_80X50:
			set_video_mode_80x50();
			break;
	}

	screen_width = low_seg[0x4A];
	screen_height = 1 + low_seg[0x84];

#ifdef __386__
	p_backbuffer = (uint8_t *)0xB8000 + ((screen_width * screen_height) * 2);
#else
	p_backbuffer = MK_FP(0xB800, ((screen_width * screen_height) * 2));
#endif

	// disable blink!
	if(cga_mda_flag == 0)
	{
		disable_blink();
	}

	screen_clear(0x7);
	toggle_swap_draw_to_backbuffer();
	screen_clear(0x7);
	toggle_swap_draw_to_backbuffer();
}

// called by actions.c
void InitScreen(void)
{
	ScreenChangeMode(start_mode);
}

void ShutdownScreen(void)
{
	screen_clear(0x7);

	if(cga_mda_flag == 0)
	{
		enable_blink();
	}

	p_backbuffer = NULL;
}

uint16_t screen_get_width(void)
{
	return screen_width;
}

uint16_t screen_get_height(void)
{
	return screen_height;
}

void screen_line_clear(uint16_t row, uint8_t colour)
{
	int16_t i;

	for(i = 0; i < screen_width; i++)
	{
		p_screen[(row * (screen_width << 1)) + (i << 1) + 0] = ' ';
		p_screen[(row * (screen_width << 1)) + (i << 1) + 1] = colour;
	}
}

void screen_print(uint16_t row, uint16_t col, uint8_t colour, uint8_t *s)
{
	uint16_t q;

	q = (row * (screen_width << 1)) + (col << 1);

	while(*s != 0)
	{
		p_screen[q++] = *s++;
		p_screen[q++] = colour;
	}
}

void screen_print_hl(uint16_t row, uint16_t col, uint8_t colour, uint8_t colour_hl, uint8_t *s)
{
	uint16_t q;
	uint8_t clr;

	q = (row * (screen_width << 1)) + (col << 1);

	clr = colour;
	while(*s != 0)
	{
		if(*s == '`')
		{
			if(clr == colour)
			{
				clr = colour_hl;
			}
			else
			{
				clr = colour;
			}

			s++;
		}
		else
		{
			p_screen[q++] = *s++;
			p_screen[q++] = clr;
		}
	}
}

void screen_outch(uint16_t row, uint16_t col, uint8_t colour, uint8_t s)
{
	p_screen[(row * (screen_width << 1)) + (col << 1) + 0] = s;
	p_screen[(row * (screen_width << 1)) + (col << 1) + 1] = colour;
}

void screen_clear(uint8_t colour)
{
	uint16_t i;
	uint16_t j;

	for(i = 0; i < screen_height; i++)
	{
		for(j = 0; j < screen_width; j++)
		{
			p_screen[(i * (screen_width << 1)) + (j << 1) + 0] = ' ';
			p_screen[(i * (screen_width << 1)) + (j << 1) + 1] = colour;
		}
	}
}

void save_to_backbuffer(void)
{
	memmove(p_backbuffer, p_screen, (screen_width * screen_height) << 1);
}

void restore_from_backbuffer(void)
{
	memmove(p_screen, p_backbuffer, (screen_width * screen_height) << 1);
}


void toggle_swap_draw_to_backbuffer(void)
{
#ifdef __386__
	uint8_t *q;
#else
	uint8_t
	far *q;
#endif
	q = p_screen;
	p_screen = p_backbuffer;
	p_backbuffer = q;
}

uint16_t get_key(void)
{
	uint16_t k;

	k = xget_key();
	if(k == 0x011B)
		k = 0x1B;

	return k;
}

uint16_t gkey(char *validkeys)
{
	uint16_t key;

	while(1)
	{
		key = get_key();

		if((key & 0xFF) == '\r')
		{
			key = '\n';
			key &= 0xFF;
		}

		if((key & 0xFF) != 0)
		{
			key &= 0xFF;
		}

		if(validkeys == NULL)
		{
			return key;
		}

		if(validkeys != NULL && (key & 0xFF) != 0)
		{
			if(strchr(validkeys, key & 0xFF) != NULL)
			{
				return key;
			}
		}

		if((key & 0xFF) == 0)
		{
			return key;
		}
	}
}
#endif
