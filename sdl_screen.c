#include <stdint.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_endian.h>

#include <string.h>
#include "sdl.h"
#include "keycodes.h"

bool backbuffer = false;

static void DoFrame(void)
{
	SDL_RenderPresent(g_renderer);
}

void hide_cursor(void)
{
}

void screen_change_mode(uint8_t c)
{
}

void screen_set_startmode(uint8_t c)
{
}

void screen_we_have_cga_mda(uint8_t c)
{
}

void ScreenChangeMode(uint8_t c)
{
}

// called by actions.c
void InitScreen(void)
{
	backbuffer = false;
}

void ShutdownScreen(void)
{
}

void screen_line_clear(uint16_t row, uint8_t colour)
{
	SDL_Rect rect;
	uint8_t c = (colour >> 4) & 0xF;

	rect.x = 0;
	rect.y = row * get_font_height();
	rect.w = window_get_width();
	rect.h = get_font_height();

	SDL_SetRenderDrawColor(g_renderer, (dos_colours[c] >> 16) & 0xFF, (dos_colours[c] >> 8) & 0xFF, (dos_colours[c] & 0xFF), SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(g_renderer, &rect);
}

void screen_print(uint16_t row, uint16_t col, uint8_t colour, uint8_t *s)
{
	PrintText(row, col, s, colour);
}

void screen_outch(uint16_t row, uint16_t col, uint8_t colour, uint8_t s)
{
	PrintChar(row, col, s, colour);
}

void screen_print_hl(uint16_t row, uint16_t col, uint8_t colour, uint8_t colour_hl, uint8_t *s)
{
	uint8_t c=colour;
	bool hl=false;

	while(*s != 0)
	{
		if(*s == '`')
		{
			s += 1;

			if(hl == false)
				c = colour_hl;
			else
				c = colour;

			hl = !hl;
		}
		else
		{
			PrintChar(row, col, *s, c);
			col += 1;
			s += 1;
		}
	}
}

void screen_clear(uint8_t colour)
{
	uint8_t c = (colour >> 4) & 0xF;

	// we clear in top half of colour as background!
	SDL_SetRenderDrawColor(g_renderer, (dos_colours[c] >> 16) & 0xFF, (dos_colours[c] >> 8) & 0xFF, (dos_colours[c] & 0xFF), SDL_ALPHA_OPAQUE);
	SDL_RenderClear(g_renderer);
}

void save_to_backbuffer(void)
{
}

void restore_from_backbuffer(void)
{
}


void toggle_swap_draw_to_backbuffer(void)
{
	if(backbuffer == true)
	{
		DoFrame();
		backbuffer = false;
	}
	else
	{
		backbuffer = true;
	}
}


void RemoveEvents(void)
{
	SDL_Event evt;

	while(SDL_PollEvent(&evt))
	{
		//
	}
}

// translate SDL codes into DOS codes.
uint32_t GetKeypress(void)
{
	SDL_Event event;

	RemoveEvents();
	while(1)
	{
		uint32_t or_flags = 0;

		//SDL_PollEvent
		if(SDL_WaitEvent(&event) == 1)
		{
			switch(event.type)
			{
				// SDL_QUIT is a window close event
				case SDL_QUIT:
					exit(1);
					break;

				case SDL_KEYDOWN:
					if((event.key.keysym.mod & KMOD_SHIFT) != 0)
					{
						or_flags |= 0x8000000;
					}

					if((event.key.keysym.mod & KMOD_CTRL) != 0)
					{
						or_flags |= 0x4000000;
					}

					if((event.key.keysym.mod & KMOD_ALT) != 0)
					{
						or_flags |= 0x2000000;
					}

					switch(event.key.keysym.sym)
					{
						case SDLK_PLUS:
						case SDLK_KP_PLUS:
						case SDLK_MINUS:
						case SDLK_KP_MINUS:
							//case SDLK_RCTRL:
							//case SDLK_RSHIFT:
						case SDLK_CAPSLOCK:
							//case SDLK_LALT:
							//case SDLK_LCTRL:
							//case SDLK_LSHIFT:
							//case SDLK_RALT:
						case SDLK_TAB:
						case SDLK_ESCAPE:
						case SDLK_DOWN:
						case SDLK_UP:
						case SDLK_LEFT:
						case SDLK_RIGHT:
						case SDLK_F1:
						case SDLK_F2:
						case SDLK_F3:
						case SDLK_F4:
						case SDLK_F5:
						case SDLK_F6:
						case SDLK_F7:
						case SDLK_F8:
						case SDLK_F9:
						case SDLK_F10:
						case SDLK_F11:
						case SDLK_F12:

						case SDLK_KP_ENTER:
						case SDLK_RETURN:
						case SDLK_RETURN2:
							//return(uint32_t)((event.key.keysym.mod << 16) + (event.key.keysym.sym & 0xFFFF));
							return (uint32_t)(event.key.keysym.sym) | or_flags;
							break;

						default:
							if((((uint32_t)(event.key.keysym.sym)) & 0xFFFFFF) >= 0x20 && (((uint32_t)(event.key.keysym.sym)) & 0xFFFFFF) < 0x7F)
							{
								return (uint32_t)(event.key.keysym.sym) | or_flags;
							}
							else
							{
								return 0;
							}

							break;
					}
					break;

			}
		}
		else
		{
			/* give cycles back to system */
			SDL_Delay(10);
		}
	}

	return (0);
}

uint16_t get_key(void)
{
	uint32_t q;
	uint8_t c;

	q = 0;
	//while((q & 0xFFFFFFFF) == 0)
	while(q == 0)
	{
		q = GetKeypress();
	}

	c = q & 0xFF;
	//switch(q & 0xF0FFFFFF)
	switch(q)
	{
		case SDLK_F1:
			return DOS_KEY_F1;
		case SDLK_F2:
			return DOS_KEY_F2;
		case SDLK_F3:
			return DOS_KEY_F3;
		case SDLK_F4:
			return DOS_KEY_F4;
		case SDLK_F5:
			return DOS_KEY_F5;
		case SDLK_F6:
			return DOS_KEY_F6;
		case SDLK_F7:
			return DOS_KEY_F7;
		case SDLK_F8:
			return DOS_KEY_F8;
		case SDLK_F9:
			return DOS_KEY_F9;
		case SDLK_F10:
			return DOS_KEY_F10;

		case SDLK_NUMLOCKCLEAR:
			break;

		case SDLK_KP_3:
		case SDLK_PAGEDOWN:
			return DOS_KEY_PAGE_DN;
		case SDLK_KP_9:
		case SDLK_PAGEUP:
			return DOS_KEY_PAGE_UP;
		case SDLK_KP_7:
		case SDLK_HOME:
			return DOS_KEY_HOME;
		case SDLK_KP_1:
		case SDLK_END:
			return DOS_KEY_END;

		case SDLK_KP_8:
		case SDLK_UP:
			return DOS_KEY_UP;
		case SDLK_KP_2:
		case SDLK_DOWN:
			return DOS_KEY_DOWN;
		case SDLK_KP_4:
		case SDLK_LEFT:
			return DOS_KEY_LEFT;
		case SDLK_KP_6:
		case SDLK_RIGHT:
			return DOS_KEY_RIGHT;

		case SDLK_DELETE:
			return DOS_KEY_DELETE;
		case SDLK_ESCAPE:
			return DOS_KEY_ESCAPE;

		case SDLK_KP_ENTER:
		case SDLK_RETURN2:
		case SDLK_RETURN:
			return DOS_KEY_ENTER;

		case SDLK_COMMA + 0x8000000:
		case SDLK_LESS:
			return '<';
			break;

		case SDLK_PERIOD + 0x8000000:
		case SDLK_GREATER:
			return '>';
			break;

		case SDLK_SLASH + 0x8000000:
		case SDLK_QUESTION:
			return '?';
			break;

		default:
			if(c >= 'a' && c <= 'z' && (q & 0x8000000) != 0)
			{
				return 'A' + (c - 'a');
			}
			else if(c >= 'A' && c <= 'Z' && (q & 0x8000000) != 0)
			{
				return 'a' + (c - 'A');
			}
			else
			{
				return (q&0xFF) | (q >> 12)&0xF000;
			}
			break;
	}

	return GetKeypress() & 0xFFFF;
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


