#include "headers.h"

#define KEY_WITH_NO_KEY		"<none>"

// same order as in structs.h
struct udtKeyNameMatrix
{
	char *ini_name;
	uint16_t internal_key;
	char *desc;
} key_matrix[]=
{
	{ "key_up", eKey_Up, "Up" },
	{ "key_down", eKey_Down, "Down" },
	{ "key_left", eKey_Left, "Left" },
	{ "key_right", eKey_Right, "Right" },
	{ "key_up_right", eKey_UpRight, "Up Right" },
	{ "key_up_left", eKey_UpLeft, "Up Left" },
	{ "key_down_right", eKey_DownRight, "Down Right" },
	{ "key_down_left", eKey_DownLeft, "Down Left" },

	{ "key_inventory", eKey_Inventory, "Inventory" },
	{ "key_pass", eKey_PassTurn, "Pass Turn" },
	{ "key_ascend", eKey_Ascend, "Ascend Stairs" },
	{ "key_descend", eKey_Descend, "Descend Stairs" },
	{ "key_open", eKey_Open, "Open" },
	{ "key_close", eKey_Close, "Close" },
	{ "key_pickup", eKey_Pickup, "Pickup" },
	{ "key_target", eKey_Target, "Target" },
	{ "key_fire", eKey_Fire, "Fire At Target" },
	{ "key_loadout", eKey_LoadOutToggle, "Loadout Toggle" },

	{ "key_version", eKey_Version, "Version" },
	{ "key_cancel", eKey_Cancel, "Cancel" },
	{ "key_quit", eKey_Quit, "Quit" },
	{ "key_save", eKey_Save, "Save" },

	{ NULL, UINT16_MAX, NULL }
};

void SetDefaultKeys(uContext *ctx)
{
	uint16_t i;

	for(i=0; i < eKey_MAX; i++)
	{
		ctx->keys[i] = 0;
	}


	ctx->keys[eKey_Up]=DOS_KEY_UP;
	ctx->keys[eKey_Down]=DOS_KEY_DOWN;
	ctx->keys[eKey_Left]=DOS_KEY_LEFT;
	ctx->keys[eKey_Right]=DOS_KEY_RIGHT;

	ctx->keys[eKey_UpLeft]=DOS_KEY_HOME;
	ctx->keys[eKey_DownLeft]=DOS_KEY_END;
	ctx->keys[eKey_UpRight]=DOS_KEY_PAGE_UP;
	ctx->keys[eKey_DownRight]=DOS_KEY_PAGE_DN;

	ctx->keys[eKey_Inventory]='i';
	ctx->keys[eKey_Ascend]='>';
	ctx->keys[eKey_Descend]='<';
	ctx->keys[eKey_Open]='O';
	ctx->keys[eKey_Close]='C';
	ctx->keys[eKey_Pickup]=',';
	ctx->keys[eKey_Cancel]=DOS_KEY_ESCAPE;
	ctx->keys[eKey_Target]='t';
	ctx->keys[eKey_Fire]='f';
	ctx->keys[eKey_LoadOutToggle] = 'l';
	ctx->keys[eKey_Version]='V';
	ctx->keys[eKey_PassTurn]='.';
	ctx->keys[eKey_Quit] = 'Q';
	ctx->keys[eKey_Save] = 's';
}

static uint16_t KeyStringToKey(char *key)
{
	uint16_t i;

	for(i=0; key_matrix[i].ini_name != NULL; i++)
	{
		if(stricmp(key, key_matrix[i].ini_name) ==  0)
			return key_matrix[i].internal_key;
	}

	return UINT16_MAX;
}

static char* KeyCodeToAscii(uint16_t k)
{
	if(k == DOS_KEY_UP) return "up";
	if(k == DOS_KEY_DOWN) return "down";
	if(k == DOS_KEY_LEFT) return "left";
	if(k == DOS_KEY_RIGHT) return "right";
	if(k == DOS_KEY_HOME) return "home";
	if(k == DOS_KEY_END) return "end";
	if(k == DOS_KEY_PAGE_UP) return "page_up";
	if(k == DOS_KEY_PAGE_DN) return "page_down";
	if(k == DOS_KEY_DELETE) return "delete";
	if(k == DOS_KEY_ESCAPE) return "escape";
	if(k == DOS_KEY_ENTER) return "enter";

	return "";
}

char* DirectionalAscii(uint16_t k)
{
	if(k == eKey_Up) return "up";
	if(k == eKey_Down) return "down";
	if(k == eKey_Left) return "left";
	if(k == eKey_Right) return "right";
	if(k == eKey_UpLeft) return "up-left";
	if(k == eKey_UpRight) return "up-right";
	if(k == eKey_DownLeft) return "down-left";
	if(k == eKey_DownRight) return "down-right";
	if(k == eKey_Cancel) return "cancel";

	return "";
}

uint16_t DOSKeyToInternal(uContext *ctx, uint16_t k)
{
	uint16_t i;

	for(i=0; i < eKey_MAX; i++)
	{
		if(ctx->keys[i] == k)
			return i;
	}

	return eKey_MAX;
}

static uint16_t KeyValueToValue(char *value)
{
	if(stricmp(value, "up") == 0) return DOS_KEY_UP;
	if(stricmp(value, "down") == 0) return DOS_KEY_DOWN;
	if(stricmp(value, "left") == 0) return DOS_KEY_LEFT;
	if(stricmp(value, "right") == 0) return DOS_KEY_RIGHT;

	if(stricmp(value, "page_up") == 0) return DOS_KEY_PAGE_UP;
	if(stricmp(value, "page_down") == 0) return DOS_KEY_PAGE_DN;
	if(stricmp(value, "home") == 0) return DOS_KEY_HOME;
	if(stricmp(value, "end") == 0) return DOS_KEY_END;
	if(stricmp(value, "delete") == 0) return DOS_KEY_DELETE;
	if(stricmp(value, "escape") == 0) return DOS_KEY_ESCAPE;

	if(stricmp(value, "space") == 0) return ' ';
	if(stricmp(value, "enter") == 0) return 0x0D;

	if(strlen(value) == 1)
	{
		if(*value >= 0x20 && *value <= 0x7F)
			return *value;
	}

	return UINT16_MAX;
}

uint16_t SetupKey(uContext *ctx, char *key, char *value)
{
	uint16_t id;
	uint16_t val;

	if(key == NULL || value == NULL)
	{
		return 0;
	}

	id = KeyStringToKey(key);
	if(id == UINT16_MAX)
	{
		printf("Unknown key bind %s\n", key);
		return 1;
	}

	val = KeyValueToValue(value);
	if(val == UINT16_MAX)
	{
		printf("Unknown key value %s on setting %s\n", value, key);
		return 1;
	}

	ctx->keys[id] = val;
	return 0;
}


uint16_t ProcessKeybinds(uContext *ctx)
{
	INIFILE *ini;
	uint16_t i;
	uint16_t j;
	uint16_t k;
	uint16_t q;
	char *cfgFile;
	j = 0;

	cfgFile = GetConfigurationFile(CONFIG_FILE);
	assert(cfgFile != NULL);

	ini = INI_load(cfgFile);
	free(cfgFile);

	if(ini != NULL)
	{
		j = 0;
		for(i=0; key_matrix[i].ini_name != NULL; i++)
		{
			j += SetupKey(ctx, key_matrix[i].ini_name, INI_get(ini, "underdark", key_matrix[i].ini_name));
		}

		INI_unload(ini);
	}

	if(j == 0)
	{
		for(k=0; k < eKey_MAX && j == 0; k++)
		{
			for(i=0; i < eKey_MAX && j == 0; i++)
			{
				if(ctx->keys[k] != 0)
				{
					if(k != i && ctx->keys[k] == ctx->keys[i])
					{
						char *k1="";
						char *k2="";

						for(q=0; key_matrix[q].ini_name != NULL; q++)
						{
							if(key_matrix[q].internal_key == k)
							{
								k1 = key_matrix[q].ini_name;
								q = INT16_MAX;
								break;
							}
						}

						for(q=0; key_matrix[q].ini_name != NULL; q++)
						{
							if(key_matrix[q].internal_key == i)
							{
								k2 = key_matrix[q].ini_name;
								q = INT16_MAX;
								break;
							}
						}

						printf("Key '%s' has same code as key '%s'\n", k1, k2);
						j += 1;
					}
				}
			}
		}
	}

	return j;
}

void KeyGetInit(uKeyGet *k)
{
	memset(k, 0x0, sizeof(uKeyGet));
}

void KeyGetAdd8(uKeyGet *k, uint8_t c)
{
	k->keys8[k->k8_idx] = c;
	k->k8_idx += 1;
}

void KeyGetAdd16(uKeyGet *k, uint16_t c)
{
	k->keys16[k->k16_idx] = c;
	k->k16_idx += 1;
}

uint16_t KeyGetKey(uKeyGet *k)
{
	uint16_t key;
	uint16_t i;

	while(1)
	{
		key = get_key();

		if((key&0xFF) == '\r')
		{
			key = '\n';
			key &= 0xFF;
		}

		if((key&0xFF) != 0)
		{
			key &= 0xFF;
		}

		for(i=0; i < k->k8_idx; i++)
		{
			if(k->keys8[i] == (key&0xFF))
			{
				k->value = key;
				return key;
			}
		}

		for(i=0; i < k->k16_idx; i++)
		{
			if(k->keys16[i] == key)
			{
				k->value = key;
				return key;
			}
		}
	}
}

char *KeyGetToString(uKeyGet *k)
{
	char buff[256];
	uint16_t q;
	uint16_t i;

	memset(buff, 0, 256);
	q = 0;

	for(i=0; i < k->k8_idx; i++)
	{
		if(k->keys8[i] != 0)
			buff[q++] = k->keys8[i];
	}

	for(i=0; i < k->k16_idx; i++)
	{
		if(i == 0 && q > 0)
		{
			strcat(buff, ", ");
		}

		strcat(buff, KeyCodeToAscii(k->keys16[i]));
		q += 1;
	}

	return strdup(buff);
}

static void PutKeyStringInto(char *kbuff, uint16_t key)
{
	char *p;

	if(key == 0)
	{
		sprintf(kbuff, KEY_WITH_NO_KEY);
		return;
	}

	p = KeyCodeToAscii(key);
	if(*p == 0)
		sprintf(kbuff, "%c", key);
	else
		sprintf(kbuff, "%s", p);
}


static void DoConfig_Move(uContext *ctx)
{
	char kbuff[64];
	bool quit = false;
	uint16_t key;
	uint16_t nkey;
	uint16_t i;

	while(quit == false)
	{
		toggle_swap_draw_to_backbuffer();

		screen_clear(0x7);
		screen_line_clear(0, 0x70);
		msg(0,2, 0x70, "Keyboard Configuration - Movement");

		PutKeyStringInto(kbuff, ctx->keys[eKey_Up]); 			msg_hl( 2, 2, HL_CLR,"`a` - `%s` Up", kbuff);
		PutKeyStringInto(kbuff, ctx->keys[eKey_Down]); 		msg_hl( 3, 2, HL_CLR,"`b` - `%s` Down", kbuff);
		PutKeyStringInto(kbuff, ctx->keys[eKey_Left]); 		msg_hl( 4, 2, HL_CLR,"`c` - `%s` Left", kbuff);
		PutKeyStringInto(kbuff, ctx->keys[eKey_Right]); 		msg_hl( 5, 2, HL_CLR,"`d` - `%s` Right", kbuff);
		PutKeyStringInto(kbuff, ctx->keys[eKey_UpLeft]);	 	msg_hl( 6, 2, HL_CLR,"`e` - `%s` Up Left", kbuff);
		PutKeyStringInto(kbuff, ctx->keys[eKey_UpRight]); 	msg_hl( 7, 2, HL_CLR,"`f` - `%s` Up Right", kbuff);
		PutKeyStringInto(kbuff, ctx->keys[eKey_DownLeft]); 	msg_hl( 8, 2, HL_CLR,"`g` - `%s` Down Left", kbuff);
		PutKeyStringInto(kbuff, ctx->keys[eKey_DownRight]); 	msg_hl( 9, 2, HL_CLR,"`h` - `%s` Down Right", kbuff);

		msg_hl(11, 2, HL_CLR, "`Q`uit");
		msg_hl(13, 2, HL_CLR, "Select [`abcdefghqQ`]");

		toggle_swap_draw_to_backbuffer();
		restore_from_backbuffer();

		key = gkey("abcdefghiqQ");
		switch(key)
		{
			default:
				toggle_swap_draw_to_backbuffer();
				msg_hl(15, 2, HL_CLR, "Press key to bind it");
				toggle_swap_draw_to_backbuffer();
				restore_from_backbuffer();
				nkey = gkey(NULL);
				for(i=0; i<eKey_MAX; i++)
				{
					if(ctx->keys[i] == nkey)
						ctx->keys[i] = 0;
				}

				switch(key)
				{
					case 'a': ctx->keys[eKey_Up] = nkey; break;
					case 'b': ctx->keys[eKey_Down] = nkey; break;
					case 'c': ctx->keys[eKey_Left] = nkey; break;
					case 'd': ctx->keys[eKey_Right] = nkey; break;
					case 'e': ctx->keys[eKey_UpLeft] = nkey; break;
					case 'f': ctx->keys[eKey_UpRight] = nkey; break;
					case 'g': ctx->keys[eKey_DownLeft] = nkey; break;
					case 'h': ctx->keys[eKey_DownRight] = nkey; break;
				}
				break;

			case 'q':
			case 'Q':
				quit = true;
				break;
		}
	}
}


static void DoConfig_Other(uContext *ctx)
{
	char kbuff[64];
	bool quit = false;
	uint16_t key;
	uint16_t nkey;
	uint16_t i;
	uint16_t row;
	uint16_t j;

	uint16_t qkeys[]= {
		eKey_Version, eKey_Inventory, eKey_PassTurn, eKey_Ascend,
		eKey_Descend, eKey_Open, eKey_Close, eKey_Pickup, eKey_Target,
		eKey_Fire, eKey_LoadOutToggle, eKey_Cancel, eKey_Quit, eKey_Save,
		UINT16_MAX
	};

	char zkeys[32];


	while(quit == false)
	{
		toggle_swap_draw_to_backbuffer();

		screen_clear(0x7);
		screen_line_clear(0, 0x70);
		msg(0,2, 0x70, "Keyboard Configuration - Other");

		memset(zkeys, 0, 32);

		for(i=0, row = 0; qkeys[i] != UINT16_MAX; i++, row++)
		{
			for(j=0; key_matrix[j].internal_key != UINT16_MAX; j++)
			{
				if(key_matrix[j].internal_key == qkeys[i])
					break;
			}

			PutKeyStringInto(kbuff, ctx->keys[qkeys[i]]);
			msg_hl(2 + row, 2, HL_CLR,"`%c` - `%s` %s", 'a' + i, kbuff, key_matrix[j].desc);
			zkeys[i] = 'a' + i;
		}

		row += 2;

		zkeys[i] = 'q';
		msg_hl(row + 2, 2, HL_CLR, "`Q`uit");
		msg_hl(row + 4, 2, HL_CLR, "Select [`%s`]", zkeys);

		row += 6;

		toggle_swap_draw_to_backbuffer();
		restore_from_backbuffer();

		key = gkey(zkeys);
		if(key == 'q')
			quit = true;
		else
		{
			toggle_swap_draw_to_backbuffer();
			msg_hl(row, 2, HL_CLR, "Press key to bind it");
			toggle_swap_draw_to_backbuffer();
			restore_from_backbuffer();

			nkey = gkey(NULL);
			for(i = 0; i < eKey_MAX; i++)
			{
				if(ctx->keys[i] == nkey)
					ctx->keys[i] = 0;
			}

			ctx->keys[qkeys[key - 'a']] = nkey;
		}
	}
}


static void SaveConfig(uContext *ctx)
{
	INIFILE *ini;
	uint16_t i;
	char kbuff[64];
	char *cfgFile;

	cfgFile = GetConfigurationFile(CONFIG_FILE);
	assert(cfgFile != NULL);
	ini = INI_load(cfgFile);
	if(ini == NULL)
	{
		ini = INI_EmptyINF();
	}

	for(i=0; key_matrix[i].ini_name != NULL; i++)
	{
		PutKeyStringInto(kbuff, ctx->keys[i]);
		if(stricmp(kbuff, KEY_WITH_NO_KEY) != 0)
			INI_UpdateItem(ini, "underdark", key_matrix[i].ini_name, kbuff);
	}

	if(ctx->auto_open_doors == true)
		INI_UpdateItem(ini, "underdark", "auto_open_doors", "true");
	else
		INI_UpdateItem(ini, "underdark", "auto_open_doors", "false");

	if(ctx->auto_get_money == true)
		INI_UpdateItem(ini, "underdark", "auto_get_money", "true");
	else
		INI_UpdateItem(ini, "underdark", "auto_get_money", "false");

	INI_save(cfgFile, ini);
	INI_unload(ini);
	free(cfgFile);
}

static void DoConfig_Options(uContext *ctx)
{
	bool quit = false;

	while(quit == false)
	{
		toggle_swap_draw_to_backbuffer();

		screen_clear(0x7);
		screen_line_clear(0, 0x70);
		msg(0,2, 0x70, "Option Configuration");

		msg_hl(2, 2, HL_CLR,"`a` - Auto open doors (`%s`)", ctx->auto_open_doors == true ? "true" : "false");
		msg_hl(3, 2, HL_CLR,"`b` - Auto pickup money (`%s`)", ctx->auto_get_money == true ? "true" : "false");

		msg_hl(10, 2, HL_CLR, "`Q`uit");
		msg_hl(12, 2, HL_CLR, "Select [`abqQ`]");

		toggle_swap_draw_to_backbuffer();
		restore_from_backbuffer();

		switch(gkey("abqQ"))
		{
			case 'a':
				ctx->auto_open_doors = !ctx->auto_open_doors;
				break;
			case 'b':
				ctx->auto_get_money = !ctx->auto_get_money;
				break;
			case 'q':
				quit = true;
				break;

		}
	}
}

void ConfigureKeys(uContext *ctx)
{
	bool quit = false;
	bool k;

	uint16_t key;

	while(quit == false)
	{
		toggle_swap_draw_to_backbuffer();

		screen_clear(0x7);

		screen_line_clear(0, 0x70);
		msg(0,2, 0x70, "Keyboard Configuration");

		k = false;
		for(key = 0; key < eKey_MAX; key++)
		{
			if(key <= eKey_DownLeft && ctx->keys[key] == 0)
			{
				k = true;
			}
		}
		msg_hl(4,2, HL_CLR, "`1` - Configure Movement keys `%s`", k == false ? "": "Some keys unconfigured");

		k = false;
		for(key = 0; key < eKey_MAX; key++)
		{
			if(key > eKey_DownLeft && ctx->keys[key] == 0)
			{
				k = true;
			}
		}

		msg_hl(5,2, HL_CLR, "`2` - Configure Other keys `%s`", k == false ? "": "Some keys unconfigured");
		msg_hl(6,2, HL_CLR, "`3` - Configure Options");

		msg_hl(8,2, HL_CLR, "`R` - Reset all keys");
		msg_hl(9, 2, HL_CLR, "`S` - Save configuration");
		msg_hl(11, 2, HL_CLR, "`Q`uit");

		msg_hl(13, 2, HL_CLR, "Select [`12rqs`]");

		for(key=0; key < eKey_MAX; key++)
		{
			if(ctx->keys[key] == 0)
			{
				msg(15, 2, 0x0E, "Warning! Some game keys are not configured. Please edit configuration.");
				key = eKey_MAX;
			}
		}

		toggle_swap_draw_to_backbuffer();
		restore_from_backbuffer();

		key = gkey("123qQrRsS");
		switch(key)
		{
			case 'q':
			case 'Q':
				quit = true;
				break;

			case '1':
				DoConfig_Move(ctx);
				break;

			case '2':
				DoConfig_Other(ctx);
				break;

			case '3':
				DoConfig_Options(ctx);
				break;

			case 's':
			case 'S':
				SaveConfig(ctx);
				msg(15, 2, 0xE, "Keys configuration saved. Press any key");
				gkey(NULL);
				break;

			case 'r':
			case 'R':
				SetDefaultKeys(ctx);
				msg(15, 2, 0xE, "Keys have been reset. Press any key");
				gkey(NULL);
				break;

		}
	}
}


