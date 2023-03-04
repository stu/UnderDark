#include "headers.h"

#include "inc_ini.h"

char *__msg_buffer;
bool workstationCactus = false;

void TestConfigFile(void)
{
	INIFILE *ini;
	char *cfgFile;

	cfgFile = GetConfigurationFile(CONFIG_FILE);
	printf("Configuration : %s\n", cfgFile);

	ini = INI_load(cfgFile);
	if(ini == NULL)
	{
		ini = INI_loadbuff((char*)inc_ini, inc_ini_SIZE);
		INI_save(cfgFile, ini);
	}

	INI_unload(ini);
	free(cfgFile);
}

void msg(uint16_t row, uint16_t col, uint16_t cA, char *strX, ...)
{
	va_list args;

	if(__msg_buffer == NULL)
	{
		return;
	}

	if(strX != NULL)
	{
		va_start(args, strX);
		// void gets around watcom binding error..
		vsprintf(__msg_buffer, strX, (void *)args);
		va_end(args);
	}
	else
	{
		__msg_buffer = "";
	}

	screen_print(row, col, cA, (uint8_t *)__msg_buffer);
}


void msg_hl(uint16_t row, uint16_t col, uint16_t cA, uint16_t cB, char *strX, ...)
{
	va_list args;

	if(__msg_buffer == NULL)
	{
		return;
	}

	if(strX != NULL)
	{
		va_start(args, strX);
		// void gets around watcom binding error..
		vsprintf(__msg_buffer, strX, (void *)args);
		va_end(args);
	}
	else
	{
		__msg_buffer = "";
	}

	screen_print_hl(row, col, cA, cB, (uint8_t *)__msg_buffer);
}



static char *ConfigGetPlayerName(void)
{
	INIFILE *ini;
	char *p;
	char *q;
	char *cfgFile = GetConfigurationFile(CONFIG_FILE);
	assert(cfgFile != NULL);

	ini = INI_load(cfgFile);
	free(cfgFile);
	if(ini != NULL)
	{
		p = INI_get(ini, "underdark", "name");

		if(p != NULL)
		{
			q = strdup(p);
		}
		else
		{
			q = CreatePlayerName();
		}

		INI_unload(ini);
	}
	else
	{
		q = CreatePlayerName();
	}

	return q;
}


static bool ConfigGetBool(char *type)
{
	INIFILE *ini;
	char *p;
	bool rc = false;
	char *cfgFile = GetConfigurationFile(CONFIG_FILE);
	assert(cfgFile != NULL);

	ini = INI_load(cfgFile);
	free(cfgFile);

	if(ini != NULL)
	{
		p = INI_get(ini, "underdark", type);
		if(p != NULL)
		{
			if(stricmp(p, "true") == 0 || stricmp(p, "yes") == 0)
			{
				rc = true;
			}
		}

		INI_unload(ini);
	}

	return rc;
}

void ClearPlayer(uContext *ctx)
{
	dlist_empty(ctx->lstMessages);

	memset(ctx->player, 0x0, sizeof(uPlayer));

	ctx->player->item_head = EMPTY_ITEM_SLOT;
	ctx->player->item_body = EMPTY_ITEM_SLOT;
	ctx->player->item_left_weap = EMPTY_ITEM_SLOT;
	ctx->player->item_right_weap = EMPTY_ITEM_SLOT;
	ctx->player->item_left_hand = EMPTY_ITEM_SLOT;
	ctx->player->item_right_hand = EMPTY_ITEM_SLOT;

	ctx->player->loadout_item_right_weap = EMPTY_ITEM_SLOT;
	ctx->player->loadout_item_left_weap = EMPTY_ITEM_SLOT;

	ctx->player->target_idx = UINT16_MAX;
}

void RollPlayer(uContext *ctx, bool first_time)
{
	char *name;
	uItem *i;

	uint16_t q;

	ClearPlayer(ctx);

	add_msg(ctx, "Welcome to the `Underdark` %s %s", VersionStringFull(), REL_STR);

	name = ConfigGetPlayerName();
	strncpy((char *)ctx->player->name, name, MAX_NAME_LEN);
	free(name);

	ctx->player->level = 1;

	ctx->player->stats[eStat_Strength] = 1 + xrnd(3);
	ctx->player->stats[eStat_Dexterity] = 1 + xrnd(3);
	ctx->player->stats[eStat_Speed] = 1 + xrnd(3);
	ctx->player->stats[eStat_Willpower] = 1 + xrnd(3);
	ctx->player->stats[eStat_DeepVision] = 3;

	ctx->player->life = 30 + xrnd(15);
	ctx->player->mana = 30 + xrnd(15);
	ctx->player->money = 30 + xrnd(15);

	ctx->player->skills[eSkill_MeleeWeapons] = 1;
	ctx->player->skills[eSkill_RangedWeapons] = 1;
	ctx->player->skills[eSkill_Magic] = 1;
	ctx->player->skills[eSkill_ItemLore] = 1;

	if(first_time == false)
		ctx->player->race = xrnd(eRace_MAX);
	else
		ctx->player->race = eRace_Orc;

	switch(ctx->player->race)
	{
		case eRace_Human:
			ctx->player->life = 40 + xrnd(25);
			ctx->player->mana = 40 + xrnd(25);
			ctx->player->money = 40 + xrnd(25);
			break;

		case eRace_Elf:
			ctx->player->stats[eStat_Strength] -= 2;
			ctx->player->stats[eStat_Dexterity] += 1;
			ctx->player->stats[eStat_Speed] += 1;
			ctx->player->stats[eStat_Willpower] += 1;
			ctx->player->stats[eStat_DeepVision] += 2;

			ctx->player->skills[eSkill_Magic] += 1;
			break;

		case eRace_Dwarf:
			ctx->player->stats[eStat_Strength] += 2;
			ctx->player->stats[eStat_Dexterity] -= 1;
			ctx->player->stats[eStat_Speed] -= 1;
			ctx->player->stats[eStat_Willpower] += 1;
			ctx->player->stats[eStat_DeepVision] += 3;

			ctx->player->skills[eSkill_ItemLore] += 1;
			break;

		case eRace_Orc:
			ctx->player->stats[eStat_Strength] += 3;
			ctx->player->stats[eStat_Dexterity] += 0;
			ctx->player->stats[eStat_Speed] -= 1;
			ctx->player->stats[eStat_Willpower] -= 1;
			ctx->player->stats[eStat_DeepVision] += 1;
			ctx->player->skills[eSkill_MeleeWeapons] += 1;
			break;

		case eRace_Gnoll:
			ctx->player->stats[eStat_Strength] -= 1;
			ctx->player->stats[eStat_Dexterity] += 1;
			ctx->player->stats[eStat_Speed] += 2;
			ctx->player->stats[eStat_Willpower] -= 1;
			ctx->player->stats[eStat_DeepVision] += 1;

			ctx->player->skills[eSkill_RangedWeapons] += 1;
			break;
	}

	for(q = 0; q < eStat_MAX; q++)
	{
		// dont cap deepvision
		if(ctx->player->stats[q] < 1 || ctx->player->stats[q] > 5 && q != eStat_DeepVision)
		{
			ctx->player->stats[q] = 3;
		}
	}

	// everyone gets 1 random skill boost
	switch(xrnd(4))
	{
		case 0:
			ctx->player->skills[eSkill_MeleeWeapons] += 1;
			break;
		case 1:
			ctx->player->skills[eSkill_RangedWeapons] += 1;
			break;
		case 2:
			ctx->player->skills[eSkill_Magic] += 1;
			break;
		case 3:
			ctx->player->skills[eSkill_ItemLore] += 1;
			break;
	}

	// where really cheating here! :) were using this as a temp variable for the play screen
	if(first_time == false)
		ctx->player->counters[0].rounds = xrnd(5);
	else
		ctx->player->counters[0].rounds = 4;

	switch(ctx->player->counters[0].rounds)
	{
		// adventurer
		case 0:
			ctx->player->skills[eSkill_MeleeWeapons] += 1;
			break;

			// hunter
		case 1:
			ctx->player->skills[eSkill_RangedWeapons] += 1;
			break;

			// rogue
		case 2:
			ctx->player->skills[eSkill_ItemLore] += 1;
			break;

			// wizard
		case 3:
			ctx->player->skills[eSkill_Magic] += 1;
			break;

			// barbarian
		case 4:
			ctx->player->skills[eSkill_MeleeWeapons] += 1;
			ctx->player->life += 15 + xrnd(15);
			break;

	}

	// if every random is right, one of your skills will equal 4!
	for(q = 0; q < eSkill_MAX; q++)
	{
		if(ctx->player->skills[q] >= 4)
		{
			add_msg(ctx, "The stars have aligned with your fate!");
		}
	}


	if(first_time == true || chance(5))
	{
		add_msg(ctx, "`%s` felt lucky today!", ctx->player->name);
		switch(xrnd(7))
		{
			case 0:
				ctx->player->stats[eStat_Strength] += 1;
				break;

			case 1:
				ctx->player->stats[eStat_Speed] += 1;
				break;

			case 2:
				ctx->player->stats[eStat_Dexterity] += 1;
				break;

			case 3:
				ctx->player->stats[eStat_Willpower] += 1;
				break;

			case 4:
				ctx->player->stats[eStat_DeepVision] += 1;
				break;

			case 5:
				ctx->player->life += xrnd(25);
				break;

			case 6:
				ctx->player->mana += xrnd(25);
				break;
		}
	}

	ctx->player->max_life = ctx->player->life;
	ctx->player->max_mana = ctx->player->mana;

	for(q = 0; q < eStat_MAX; q++)
	{
		ctx->player->base_stats[q] = ctx->player->stats[q];
	}

	ctx->player->target_idx = UINT16_MAX;
	ctx->player->monster_fov_selected_idx = UINT8_MAX;


	// where really cheating here! :) were using this as a temp variable for the play screen
	switch(ctx->player->counters[0].rounds)
	{
		// adventurer
		case 0:
			// give everyone a torch or brass lantern
			if(chance(10))
			{
				AddItemToInventory(ctx, FindItemByName(ctx, ITEM_NAME_BRASS_LANTERN), 1);
			}
			else
			{
				AddItemToInventory(ctx, FindItemByName(ctx, ITEM_NAME_TORCH), 1);
			}
			i = AddItemToInventory(ctx, FindItemByName(ctx, ITEM_NAME_LONG_SWORD), 1);
			EquipItem(ctx, i, true);
			i = AddItemToInventory(ctx, FindItemByName(ctx, ITEM_NAME_LEATHER_ARMOUR), 1);
			EquipItem(ctx, i, true);
			break;

			// hunter
		case 1:
			i = AddItemToInventory(ctx, FindItemByName(ctx, ITEM_NAME_SHORT_BOW), 1);
			EquipItem(ctx, i, true);
			i = AddItemToInventory(ctx, FindItemByName(ctx, ITEM_NAME_ARROWS), 50 + xrnd(25));
			EquipItem(ctx, i, true);
			i = AddItemToInventory(ctx, FindItemByName(ctx, ITEM_NAME_LEATHER_ARMOUR), 1);
			EquipItem(ctx, i, true);
			AddItemToInventory(ctx, FindItemByName(ctx, ITEM_NAME_SKINNING_KNIFE), 1);
			// skinning knife alt loadout
			ctx->player->loadout_item_left_weap = 3;
			break;

			// rogue
		case 2:
			AddItemToInventory(ctx, FindItemByName(ctx, ITEM_NAME_TORCH), 1);
			i = AddItemToInventory(ctx, FindItemByName(ctx, ITEM_NAME_SHORT_SWORD), 1);
			EquipItem(ctx, i, true);
			i = AddItemToInventory(ctx, FindItemByName(ctx, ITEM_NAME_LEATHER_ARMOUR), 1);
			EquipItem(ctx, i, true);
			AddItemToInventory(ctx, FindItemByName(ctx, EPB_NAME_HEAL1), 1);
			break;

			// wizard
		case 3:
			i = AddItemToInventory(ctx, FindItemByName(ctx, ITEM_NAME_RING_PROTECTION), 1);
			EquipItem(ctx, i, true);
			i = AddItemToInventory(ctx, FindItemByName(ctx, ITEM_NAME_PURPLE_SPANDEX), 1);
			EquipItem(ctx, i, true);
			i = AddItemToInventory(ctx, FindItemByName(ctx, ITEM_NAME_TANTO_KNIFE), 1);
			EquipItem(ctx, i, true);
			i = AddItemToInventory(ctx, FindItemByName(ctx, ITEM_NAME_WAND_MISSILES), 1);
			EquipItem(ctx, i, true);

			q = 1 + xrnd(3);
			while(q > 0)
			{
				AddItemToInventory(ctx, FindItemByName(ctx, EPB_NAME_MANA_REFILL1), 1);
				q -= 1;
			}
			break;

			// barbarian
		case 4:
			AddItemToInventory(ctx, FindItemByName(ctx, ITEM_NAME_TORCH), 1);
			i = AddItemToInventory(ctx, FindItemByName(ctx, "axe 'vermin' +1/rats"), 1);
			EquipItem(ctx, i, true);
			i = AddItemToInventory(ctx, FindItemByName(ctx, "axe 'tide' +1/rats"), 1);
			EquipItem(ctx, i, true);

			AddItemToInventory(ctx, FindItemByName(ctx, EPB_NAME_HEAL1), 1);
			AddItemToInventory(ctx, FindItemByName(ctx, EPB_NAME_HEAL1), 1);
			break;
	}
}

void SetStatusRow(uContext *ctx)
{
	if(screen_get_height() < 40)
	{
		ctx->status_row = screen_get_height() - 8;
	}
	else
	{
		ctx->status_row = screen_get_height() - 15;
	}
}

uContext *NewContext(void)
{
	uint16_t i;
	uContext *ctx = calloc(1, sizeof(uContext));
	assert(ctx != NULL);

	ctx->maxMessages = DEFAULT_MAX_MESSAGES;
	ctx->lstMessages = NewDList(FreeMessage);
	assert(ctx->lstMessages != NULL);
	ctx->lstItems = NewDList(FreeItem);
	assert(ctx->lstItems != NULL);
	ctx->lstMonsters = NewDList(FreeMonster);
	assert(ctx->lstMonsters != NULL);

	ctx->auto_get_money = ConfigGetBool("auto_get_money");
	ctx->auto_open_doors = ConfigGetBool("auto_open_doors");

	ctx->player = (uPlayer *)calloc(1, sizeof(uPlayer));
	assert(ctx->player != NULL);
	ctx->map = (uMap *)calloc(1, sizeof(uMap));
	assert(ctx->map != NULL);

	ctx->map->lstTriggers = NewDList(FreeMapTrigger);
	assert(ctx->map->lstTriggers != NULL);
	ctx->map->lstMonsters = NewDList(FreeMapMonster);
	assert(ctx->map->lstMonsters != NULL);
	ctx->map->lstItems = NewDList(FreeMapItem);
	assert(ctx->map->lstItems != NULL);

	SetDefaultKeys(ctx);

	for(i = 0; i < eFLAGS_MAX; i++)
	{
		bit_set_bit(ctx->game_flags, i, false);
	}

	return ctx;
}

static void FreePlayer(uPlayer *p)
{
	if(p == NULL)
	{
		return;
	}

	memset(p, 0x0, sizeof(uPlayer));
	free(p);
}

void ReleaseContext(uContext *ctx)
{
	if(ctx == NULL)
	{
		return;
	}

	if(ctx->map != NULL)
	{
		FreeMap(ctx);
	}

	FreeDList(ctx->lstMessages);
	FreeDList(ctx->lstItems);
	FreeDList(ctx->lstMonsters);

	if(ctx->player != NULL)
	{
		FreePlayer(ctx->player);
		ctx->player = NULL;
	}

	memset(ctx, 0x0, sizeof(uContext));
	free(ctx);
}

bool IsFirstTime(void)
{
	INIFILE *ini;
	char *p;
	bool ft = false;
	char *cfgFile = GetConfigurationFile(CONFIG_FILE);
	assert(cfgFile != NULL);

	ini = INI_load(cfgFile);
	if(ini != NULL)
	{
		p = INI_get(ini, "underdark", "first_time");

		ft = true;
		if(p != NULL && stricmp(p, "false") == 0)
		{
			ft = false;
		}

		INI_UpdateItem(ini, "underdark", "first_time", "false");
		INI_save(cfgFile, ini);
		INI_unload(ini);
	}

	free(cfgFile);

	return ft;
}

