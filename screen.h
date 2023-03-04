#ifndef SCREEN_H
#define SCREEN_H
#ifdef __cplusplus
extern "C"{
#endif


enum eSCREEN_MODE
{
	SCREEN_MODE_DEFAULT = 0,
	SCREEN_MODE_80X25,
	SCREEN_MODE_80X28,
	SCREEN_MODE_80X43,
	SCREEN_MODE_80X50
};

#ifndef __linux__
extern uint16_t get_key(void);

extern void hide_cursor(void);
#pragma aux hide_cursor = \
	"mov ah,1" \
	"mov cx,0x2607" \
	"int 0x10" \
	modify [ax bx cx dx];

extern void show_cursor(void);
#pragma aux show_cursor = \
	"mov ah,1" \
	"mov cx,0x0607" \
	"int 0x10" \
	modify [ax bx cx dx];

extern void disable_blink(void);
#pragma aux disable_blink = \
	"mov ax,0x1003"	\
	"xor bx,bx"		\
	"int 10h"		\
	modify    [ax bx];

extern void enable_blink(void);
#pragma aux enable_blink = \
	"mov ax,0x1003"	\
	"xor bx,bx"		\
	"mov bl,1"		\
	"int 0x10"		\
	modify    [ax bx];

#else
extern void hide_cursor(void);
extern uint16_t get_key(void);
#endif

extern void set_cursor_position(uint16_t r, uint16_t c);
extern void screen_we_have_cga_mda(uint8_t c);
extern void screen_set_startmode(uint8_t c);

extern void InitScreen(void);
extern void ShutdownScreen(void);
extern void ScreenChangeMode(uint8_t c);

extern uint16_t screen_get_width(void);
extern uint16_t screen_get_height(void);

extern void screen_clear(uint8_t colour);
extern void screen_line_clear(uint16_t row, uint8_t colour);
extern void screen_print(uint16_t row, uint16_t col, uint8_t colour, uint8_t *s);
extern void screen_print_hl(uint16_t row, uint16_t col, uint8_t colour, uint8_t colour_hl, uint8_t *s);
extern void screen_outch(uint16_t row, uint16_t col, uint8_t colour, uint8_t s);

extern void save_to_backbuffer(void);
extern void restore_from_backbuffer(void);

extern void toggle_swap_draw_to_backbuffer(void);

extern uint16_t gkey(char *validkeys);


#ifdef __cplusplus
};
#endif
#endif        //  #ifndef SCREEN_H
