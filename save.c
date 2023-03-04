#include "headers.h"

static void SaveMapMonsters(uContext *ctx, INIFILE *ini)
{
	DLElement *e;
	uint16_t q;
	char key0[32];

	save_u16(ini, "monsters", "count", 0);

	q = 0;
	assert(ctx->map->lstMonsters != NULL);
	e = dlist_head(ctx->map->lstMonsters);
	while(e != NULL)
	{
		uMapMonster *mm = dlist_data(e);
		e = dlist_next(e);
		sprintf(key0, "monster_%03u", q);

		save_u8(ini, key0, "row", mm->row);
		save_u8(ini, key0, "col", mm->col);

		save_u16(ini, key0, "mid", mm->monster_id);
		save_u16(ini, key0, "gid", mm->gen_id);

		save_u8(ini, key0, "d", mm->disposition);

		save_i16(ini, key0, "h", mm->health);
		save_i16(ini, key0, "a", mm->armour);

		save_u8(ini, key0, "lpc", mm->last_player_col);
		save_u8(ini, key0, "lpr", mm->last_player_row);

		q += 1;
	}

	save_u16(ini, "monsters", "count", q);
}

static void SaveTriggers(uContext *ctx, INIFILE *ini)
{
	DLElement *e;
	uint16_t q;
	char key0[32];

	save_u16(ini, "triggers", "count", 0);

	q = 0;
	e = dlist_head(ctx->map->lstTriggers);
	while(e != NULL)
	{
		uMapTrigger *trig = dlist_data(e);
		e = dlist_next(e);

		sprintf(key0, "trigger_%03u", q);
		save_u8(ini, key0, "row", trig->row);
		save_u8(ini, key0, "col", trig->col);
		save_u8(ini, key0, "type", trig->type);

		switch(trig->type)
		{
			case eTrigger_SignOnWall:
				save_str(ini, key0, "data", (uint8_t*)trig->data.t_sign_on_wall.msg);
				break;
		}

		q += 1;
	}

	save_u16(ini, "triggers", "count", q);
}

static void SaveMapItems(uContext *ctx, INIFILE *ini)
{
	DLElement *e;
	char val[32];
	uint16_t q;
	char key0[32];
	char value[256];

	INI_UpdateItem(ini, "items", "count", "0");

	q = 0;
	e = dlist_head(ctx->map->lstItems);
	while(e != NULL)
	{
		uMapItem *mi = dlist_data(e);
		e = dlist_next(e);
		sprintf(key0, "item_%03u", q);

		sprintf(val, "%"PRIu8"", mi->row);
		INI_UpdateItem(ini, key0, "row", val);
		sprintf(val, "%"PRIu8"", mi->col);
		INI_UpdateItem(ini, key0, "col", val);
		sprintf(val, "%"PRIu8"", mi->type);
		INI_UpdateItem(ini, key0, "type", val);

		switch(mi->type)
		{
			case eDT_Gold:
				save_i16(ini, key0, "data", mi->data.i_gold.gold);
				break;
			case eDT_Weapon:
			case eDT_Potion:
			case eDT_Armour:
			case eDT_Item:
			case eDT_Food:
				save_u16(ini, key0, "data0", mi->data.i_item.idx);
				save_i16(ini, key0, "data1", mi->data.i_item.count);
				INI_UpdateItem(ini, key0, "data2", mi->data.i_item.has_item_data == false ? "f": "t");
				memset(value, 0, 256);
				if(deconstruct_item_attribute(&mi->data.i_item.item, value) == true)
				{
					save_str(ini, "items", "data3", (uint8_t*)value);
				}
				break;
		}

		q += 1;
	}

	if(q > 0)
	{
		sprintf(val, "%"PRIu16"", q);
		INI_UpdateItem(ini, "items", "count", val);
	}
}

void SaveMap(uContext *ctx)
{
	char key[64];
	uint8_t value[512];
	uint8_t data[512];
	char fname[32];
	INIFILE *ini;
	uint16_t r, c, q;
	char *cfgFile;

	screen_line_clear(screen_get_height()-1, 0);

	sprintf(fname, "z_map%03u.sav", ctx->locn.level);
	cfgFile = GetConfigurationFile(fname);

	ini = INI_EmptyINF();

	save_u16(ini, "map", "mr", ctx->map->max_rows);
	save_u16(ini, "map", "mc", ctx->map->max_cols);
	save_u16(ini, "map", "r", ctx->locn.row);
	save_u16(ini, "map", "c", ctx->locn.col);
	save_u8(ini, "map", "q", ctx->map->recovery);

	for(r=0; r < MAX_MAP_HEIGHT; r++)
	{
		sprintf(key, "Map Row %u", r);
		SetLoadSaveStatus(false, key);

		for(c=0; c < MAX_MAP_WIDTH; c++)
		{
			data[c] = ctx->map->cells[(MAX_MAP_WIDTH * r) + c].tile.tile;
		}
		q = rle_pack_line(value, 512, data, MAX_MAP_WIDTH);
		encode64((uint8_t *)value, q, (char *)data, 512);
		sprintf(key, "row_%03u_data", r);
		INI_UpdateItem(ini, "map", key, (char *)data);

		for(c=0; c < MAX_MAP_WIDTH; c++)
		{
			data[c] = ctx->map->cells[(MAX_MAP_WIDTH * r) + c].tile.flags;
		}
		q = rle_pack_line(value, 512, data, MAX_MAP_WIDTH);
		encode64((uint8_t*)value, q, (char*)data, 512);
		sprintf(key, "row_%03u_flag", r);
		INI_UpdateItem(ini, "map", key, (char*)data);
	}

	SetLoadSaveStatus(false, "Triggers");
	SaveTriggers(ctx, ini);
	SetLoadSaveStatus(false, "NPC's");
	SaveMapMonsters(ctx, ini);
	SetLoadSaveStatus(false, "Loot");
	SaveMapItems(ctx, ini);

	remove(cfgFile);
	SetLoadSaveStatus(false, "Map");
	INI_save(cfgFile, ini);
	INI_unload(ini);

	free(cfgFile);
}

void SaveGame(uContext *ctx)
{
	uint16_t j;
	uint16_t q;

	char key[64];
	char value[256];

	DLElement *e;
	INIFILE *ini;
	char *cfgFile;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Core info

	SetLoadSaveStatus(false, "Core");
	ini = INI_EmptyINF();

	save_u16(ini, "core", "mgc", ctx->mgen_count);
	save_u32(ini, "core", "mv", ctx->moves);

	save_u8(ini, "core", "ll", ctx->locn.level);
	save_u8(ini, "core", "lr", ctx->locn.row);
	save_u8(ini, "core", "lc", ctx->locn.col);
	save_u8(ini, "core", "lm", ctx->locn.map);

	save_u16(ini, "core", "dtc", ctx->draw_top_col);
	save_u16(ini, "core", "dtr", ctx->draw_top_row);
	save_u16(ini, "core", "dc", ctx->draw_col);
	save_u16(ini, "core", "dr", ctx->draw_row);

	SetLoadSaveStatus(false, "Flags");
	save_u16(ini, "flags", "count", eFLAGS_MAX);
	for(q=0; q < eFLAGS_MAX; q++)
	{
		sprintf(key, "flag_%02u", q);
		INI_UpdateItem(ini, "flags", key, bit_get_bit(ctx->game_flags, q) == 0 ? "f": "t");
	}

	SetLoadSaveStatus(false, "Messages");
	save_u16(ini, "messages", "count", 0);

	q = 0;
	e = dlist_head(ctx->lstMessages);
	while(e != NULL)
	{
		uMessage *m = dlist_data(e);
		e = dlist_next(e);

		sprintf(key, "message_%03u", q);
		save_u8(ini, key, "count", m->count);
		save_str(ini, key, "data", (uint8_t*)m->msg);
		q += 1;
	}
	save_u16(ini, "messages", "count", q);

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// Global Item modifications to master list.
	// (eg: identified items, generated names)

	SetLoadSaveStatus(false, "Items");
	save_u16(ini, "items", "count", 0);

	e = dlist_head(ctx->lstItems);
	q = 0;
	while(e != NULL)
	{
		uItem *item = dlist_data(e);
		e = dlist_next(e);

		sprintf(key, "item_%03u", q);

		memset(value, 0, 256);
		if(deconstruct_item_attribute(item, value) == true)
		{
			save_u16(ini, key, "idx", item->idx);
			save_str(ini, key, "data", (uint8_t*)value);
			q += 1;
		}
	}

	save_u16(ini, "items", "count", q);

	cfgFile = GetConfigurationFile(FILE_CORE);
	remove(cfgFile);
	INI_save(cfgFile, ini);
	INI_unload(ini);
	free(cfgFile);


	SetLoadSaveStatus(false, "Player");
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// do player data
	ini = INI_EmptyINF();

	save_str(ini, "player", "name", ctx->player->name);
	save_u32(ini, "player", "s", ctx->player->score);
	save_u8(ini, "player", "r", ctx->player->race);
	save_u16(ini, "player", "m", ctx->player->money);
	save_i16(ini, "player", "a", ctx->player->armour);
	save_u8(ini, "player", "l", ctx->player->level);
	save_u16(ini, "player", "x", ctx->player->experience);
	save_u8(ini, "player", "u", ctx->player->level_up_points);
	save_i16(ini, "player", "l0", ctx->player->life);
	save_i16(ini, "player", "l1", ctx->player->max_life);
	save_i16(ini, "player", "m0", ctx->player->mana);
	save_i16(ini, "player", "m1", ctx->player->max_mana);

	save_u8(ini, "player", "st0", ctx->player->stats[eStat_Speed]);
	save_u8(ini, "player", "st1", ctx->player->stats[eStat_DeepVision]);
	save_u8(ini, "player", "st2", ctx->player->stats[eStat_Dexterity]);
	save_u8(ini, "player", "st3", ctx->player->stats[eStat_Willpower]);
	save_u8(ini, "player", "st4", ctx->player->stats[eStat_Strength]);

	save_u8(ini, "player", "bst0", ctx->player->base_stats[eStat_Speed]);
	save_u8(ini, "player", "bst1", ctx->player->base_stats[eStat_DeepVision]);
	save_u8(ini, "player", "bst2", ctx->player->base_stats[eStat_Dexterity]);
	save_u8(ini, "player", "bst3", ctx->player->base_stats[eStat_Willpower]);
	save_u8(ini, "player", "bst4", ctx->player->base_stats[eStat_Strength]);

	save_u8(ini, "player", "sk0", ctx->player->skills[eSkill_ItemLore]);
	save_u8(ini, "player", "sk1", ctx->player->skills[eSkill_Magic]);
	save_u8(ini, "player", "sk2", ctx->player->skills[eSkill_RangedWeapons]);
	save_u8(ini, "player", "sk3", ctx->player->skills[eSkill_MeleeWeapons]);

	save_u8(ini, "player", "lo", ctx->player->loadout);

	save_u8(ini, "player", "i0", ctx->player->item_left_weap);
	save_u8(ini, "player", "i1", ctx->player->item_right_weap);
	save_u8(ini, "player", "i2", ctx->player->item_body);
	save_u8(ini, "player", "i3", ctx->player->item_head);
	save_u8(ini, "player", "i4", ctx->player->item_left_hand);
	save_u8(ini, "player", "i5", ctx->player->item_right_hand);

	save_u8(ini, "player", "loi0", ctx->player->loadout_item_left_weap);
	save_u8(ini, "player", "loi1", ctx->player->loadout_item_right_weap);
	save_u8(ini, "player", "loi2", ctx->player->loadout_item_body);
	save_u8(ini, "player", "loi3", ctx->player->loadout_item_head);
	save_u8(ini, "player", "loi4", ctx->player->loadout_item_left_hand);
	save_u8(ini, "player", "loi5", ctx->player->loadout_item_right_hand);

	save_u16(ini, "player", "ti", ctx->player->target_idx);
	save_u8(ini, "player", "mfsi", ctx->player->monster_fov_selected_idx);
	save_u8(ini, "player", "mfi", ctx->player->monster_fov_idx);


	SetLoadSaveStatus(false, "Counters");
	save_u16(ini, "counters", "count", 0);
	q = 0;
	for(j=0; j<MAX_COUNTER_ITEMS; j++)
	{
		sprintf(key, "counter_%03u", q);
		if(ctx->player->counters[j].effect != 0)
		{
			save_u16(ini, key, "effect", ctx->player->counters[j].effect);
			save_u16(ini, key, "rounds", ctx->player->counters[j].rounds);
			save_i16(ini, key, "value", ctx->player->counters[j].adjust);
			q += 1;
		}
	}
	save_u16(ini, "counters", "count", q);

	SetLoadSaveStatus(false, "Inventory");
	for(j=0; j < MAX_INVENTORY_ITEMS; j++)
	{
		sprintf(key, "item_%03u", j);
		save_u8(ini, key, "count", ctx->player->items[j].count);
		save_u16(ini, key, "index", ctx->player->items[j].item.idx);
		// bust item!
		memset(value, 0, 128);
		if(deconstruct_item_attribute(&ctx->player->items[j].item, value) == true)
		{
			q += 1;
			save_str(ini, key, "attr", (uint8_t*)value);
		}
	}

	cfgFile = GetConfigurationFile(FILE_PLAYER);
	remove(cfgFile);
	INI_save(cfgFile, ini);
	INI_unload(ini);
	free(cfgFile);

	SaveMap(ctx);
}
