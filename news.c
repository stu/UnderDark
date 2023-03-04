#include "headers.h"

#include "inc_nws.h"
#include "inc_wlc.h"

static void FreeNewsLine(void *x)
{
	if(x == NULL)
		return;

	free(x);
}

static DList *LoadNews(char *filename)
{
	DList *lst;
	char buff[128];
	uint8_t *p;
	uint8_t *ptr;
	uint16_t fsize;
	uint16_t idx;

	if(stricmp(filename, "news.txt") == 0)
	{
		ptr = inc_nws;
		fsize = inc_nws_SIZE;
	}
	else
	{
		ptr = inc_wlc;
		fsize = inc_wlc_SIZE;
	}

	idx = 0;
	lst = NewDList(FreeNewsLine);

	buff[0] = 1;
	while(idx < fsize && dlist_size(lst) < 512)
	{
		int c;
		memset(buff, 0x0, 128);

		p = (uint8_t *)buff;
		c = 0;
		while(c < 128 && ptr[idx] != 0x0A && ptr[idx] != 0x0D)
		{
			*p++ = ptr[idx++];
		}

		if(ptr[idx] == 0x0A)
		{
			idx += 1;
		}
		else if(ptr[idx] == 0x0D && ptr[idx + 1] == 0x0A)
		{
			idx += 2;
		}

		buff[screen_get_width()] = 0;
		dlist_ins(lst, strdup(buff));
	}

	return lst;
}

void DisplayFixedFile(char *filename, char *title)
{
	DList *lst;
	DLElement *e;
	uint16_t top;
	uint16_t i;
	bool quit = false;
	bool key_ok;

	lst = LoadNews(filename);

	top = 0;
	quit = false;

	while(quit == false)
	{
		// display
		i = 0;
		e = dlist_head(lst);
		while(i < top && e != NULL)
		{
			e = dlist_next(e);
			i += 1;
		}

		toggle_swap_draw_to_backbuffer();
		screen_clear(0x7);

		screen_line_clear(0, 0x70);
		screen_print(0,2, 0x70, (uint8_t*)title);

		for(i=0; i < screen_get_height()-2; i++)
		{
			screen_line_clear(1 + i, 0x3);
			if(e != NULL)
			{
				screen_print_hl(1 + i, 0, 3, 0x0b, dlist_data(e));
				e = dlist_next(e);
			}
		}

		screen_line_clear(screen_get_height()-1, 0x70);
		screen_print(screen_get_height()-1,0, 0x70, (uint8_t*)" Cursor Up + Down, Page Up/Down to scroll, ESC to close");

		if(dlist_size(lst) < screen_get_height()-2)
			sprintf(__msg_buffer, "%5u / %-5u", dlist_size(lst), dlist_size(lst));
		else
			sprintf(__msg_buffer, "%5u / %-5u", top + screen_get_height()-2, dlist_size(lst));

		screen_print(screen_get_height()-1, screen_get_width() - strlen(__msg_buffer), 0x70, (uint8_t*)__msg_buffer);

		toggle_swap_draw_to_backbuffer();
		restore_from_backbuffer();

		key_ok = false;
		while(key_ok == false)
		{
			key_ok = true;
			switch(gkey(NULL))
			{
				default:
					key_ok = false;
					break;

				case 'q':
				case 'Q':
				case DOS_KEY_ESCAPE:
					quit = true;
					break;

				case DOS_KEY_HOME:
					if(top > 0)
						top = 0;
					else
						key_ok = false;
					break;

				case DOS_KEY_END:
					if(dlist_size(lst) > screen_get_height()-2)
						top = dlist_size(lst) - (screen_get_height() - 2);
					else
						top = 0;
					break;

				case DOS_KEY_UP:
					if(top > 0)
						top -= 1;
					else
						key_ok = false;
					break;

				case DOS_KEY_DOWN:
					if( (screen_get_height()-2) + top < dlist_size(lst))
					{
						top += 1;
					}
					else
						key_ok = false;
					break;

				case DOS_KEY_PAGE_UP:
					if(top > screen_get_height() - 2)
						top -= (screen_get_height() - 2);
					else if(top > 0)
						top = 0;
					else
						key_ok = false;
					break;

				case DOS_KEY_PAGE_DN:
					top += (screen_get_height() - 2);
					if(top > dlist_size(lst) - (screen_get_height() - 2))
					{
						top = dlist_size(lst) - (screen_get_height() - 2);
					}
					break;
			}
		}
	}

	FreeDList(lst);
}

