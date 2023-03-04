#include "headers.h"

void DeathScreen(uContext *ctx, char *msg)
{
	char nbuff[64];
	char sbuff1[32];
	char sbuff2[32];
	char *p;
	uint16_t i;

	ScreenChangeMode(SCREEN_MODE_80X25);
	hide_cursor();

	toggle_swap_draw_to_backbuffer();

	screen_clear(0x07);

	i = 4;

	memset(nbuff, 0x20, 32);

	sprintf(nbuff + ((MAX_NAME_LEN - strlen((char*)ctx->player->name)) / 2), "%s", ctx->player->name);
	p = strchr(nbuff, 0x0);
	*p = 0x20;
	nbuff[MAX_NAME_LEN] = 0;

	memset(sbuff2, 0x20, 16);
	sprintf(sbuff1, "%"PRIu32"", ctx->player->score);
	sprintf(sbuff2 + ((7 - strlen(sbuff1)) / 2), "%s", sbuff1);
	p = strchr(sbuff2, 0);
	*p = 0x20;
	sbuff2[7] = 0;

	msg_hl(i, 5, 7, 0x0E," %s %s on level %"PRIu16"", ctx->player->name, msg, ctx->locn.level);

	i += 2;

	msg_hl(i, 5, 7, 0xE, "                                         ____ "); i+= 1;
	msg_hl(i, 5, 7, 0xE, "                                        |    |");i+= 1;
	msg_hl(i, 5, 7, 0xE, "                              __________|    |__________");i+= 1;
	msg_hl(i, 5, 7, 0xE, "                             | %s |", nbuff);i+= 1;
	msg_hl(i, 5, 7, 0xE, "                             |__________      __________|");i+= 1;
	msg_hl(i, 5, 7, 0xE, "                                        |    |");i+= 1;
	msg_hl(i, 5, 7, 0xE, "                   _________            |    |");i+= 1;
	msg_hl(i, 5, 7, 0xE, "                  /  R I P  \\           |    |");i+= 1;
	msg_hl(i, 5, 7, 0xE, "                  |         |           |    |");i+= 1;
	msg_hl(i, 5, 7, 0xE, "                  | %s |           |    |", sbuff2);i+= 1;
	msg_hl(i, 5, 7, 0xE, "                  |         |           |    |");i+= 1;
	msg_hl(i, 5, 7, 0xE, "                  |         |           |    |");i+= 1;
	msg_hl(i, 5, 7, 0xE, "                  |         |           |    |");i+= 1;
	msg_hl(i, 5, 7, 0xE, "                  |         |           |    |");i+= 1;
	msg_hl(i, 5, 7, 0xE, "^^..^...;;^^;;..^...^;;.^^.....^.;;..~~~...^^^^^....~~;;;~~;;;;;....");//i+= 1;

	msg_hl(screen_get_height()-1, (screen_get_width()-12)/2, 7, 0xF,  "Press `ESCAPE` to continue");

	toggle_swap_draw_to_backbuffer();
	restore_from_backbuffer();

	while(get_key() != DOS_KEY_ESCAPE)
	{
		//
	}

	CreateScore(ctx, msg);
}
