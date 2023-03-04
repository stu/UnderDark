#include "headers.h"

bool LoadMap(uContext *ctx)
{
	bool ok = true;
	char fname[32];
	char key[32];
	uint8_t data[512];
	uint8_t rle_row[512];

	INIFILE *ini;
	uint16_t r, c;
	uint16_t count;
	char *cfgFile;

	screen_line_clear(screen_get_height()-1, 0);

	// release anything basemap
	ClearMap(ctx);
	SetLoadSaveStatus(true, "Map Load");

	sprintf(fname, "z_map%03u.sav", ctx->locn.level);
	cfgFile = GetConfigurationFile(fname);
	ini = INI_load(cfgFile);

	if(ini == NULL)
		ok = false;

	if(ini != NULL)
	{
		if(ok == true) ctx->map->max_rows = get_u16(ini, "map", "mr", &ok);
		if(ok == true) ctx->map->max_cols = get_u16(ini, "map", "mc", &ok);
		if(ok == true) ctx->locn.row = get_u16(ini, "map", "r", &ok);
		if(ok == true) ctx->locn.col = get_u16(ini, "map", "c", &ok);
		if(ok == true) ctx->map->recovery = get_u8(ini, "map", "q", &ok);

		for(r = 0; ok == true && r < MAX_MAP_HEIGHT; r++)
		{
			sprintf(key, "Map Row %u", r);
			SetLoadSaveStatus(true, key);

			sprintf(key, "row_%03u_data", r);
			load_rle(ini, "map", key, rle_row, MAX_MAP_WIDTH);
			for(c=0; c < MAX_MAP_WIDTH; c++)
			{
				ctx->map->cells[(r * MAX_MAP_WIDTH) + c].tile.tile = rle_row[c];
			}

			sprintf(key, "row_%03u_flag", r);
			load_rle(ini, "map", key, rle_row, MAX_MAP_WIDTH);
			for(c=0; c < MAX_MAP_WIDTH; c++)
			{
				ctx->map->cells[(r * MAX_MAP_WIDTH) + c].tile.flags = rle_row[c];
			}
		}

		SetLoadSaveStatus(true, "Items");
		// load map items
		count = get_u16(ini, "items", "count", &ok);
		for(c=0; ok == true && c < count; c++)
		{
			uMapItem *it;
			sprintf(key, "item_%03u", c);

			it = (uMapItem*)calloc(1, sizeof(uMapItem));
			assert(it != NULL);
			assert(ctx->map->lstItems != NULL);

			if(ok == true) it->row = get_u8(ini, key, "row", &ok);
			if(ok == true) it->col = get_u8(ini, key, "col", &ok);
			if(ok == true) it->type = get_u8(ini, key, "type", &ok);
			if(ok == true)
			{
				switch(it->type)
				{
					case eDT_Gold:
						it->data.i_gold.gold = get_i16(ini, key, "data", &ok);
						break;
					case eDT_Weapon:
					case eDT_Item:
					case eDT_Food:
					case eDT_Potion:
					case eDT_Armour:
						it->data.i_item.idx = get_u16(ini, key, "data0", &ok);
						it->data.i_item.count = get_i16(ini, key, "data1", &ok);

						load_ue64(ini, key, "data2", data, 512);
						it->data.i_item.has_item_data = false;
						if(data[0] =='t')
							it->data.i_item.has_item_data = true;

						load_ue64(ini, key, "data3", data, 512);
						ok = reconstruct_item_attributes(&it->data.i_item.item, (char*)data);
						break;
				}
			}

			if(ok == true)
				dlist_ins(ctx->map->lstItems, it);
			else
				free(it);
		}

		SetLoadSaveStatus(true, "NPCs");
		// load monsters
		count = get_u16(ini, "monsters", "count", &ok);
		for(c=0; ok == true && c < count; c++)
		{
			uMapMonster *mm;

			sprintf(key, "monster_%03u", c);

			mm = (uMapMonster*)calloc(1, sizeof(uMapMonster));
			assert(mm != NULL);
			assert(ctx->map->lstMonsters != NULL);

			if(ok == true) mm->row = get_u8(ini, key, "row", &ok);
			if(ok == true) mm->col = get_u8(ini, key, "col", &ok);
			if(ok == true) mm->monster_id = get_u16(ini, key, "mid", &ok);
			if(ok == true) mm->gen_id = get_u16(ini, key, "gid", &ok);
			if(ok == true) mm->disposition = get_u8(ini, key, "d", &ok);
			if(ok == true) mm->health = get_i16(ini, key, "h", &ok);
			if(ok == true) mm->armour = get_i16(ini, key, "a", &ok);
			if(ok == true) mm->last_player_row = get_u8(ini, key, "lpr", &ok);
			if(ok == true) mm->last_player_col = get_u8(ini, key, "lpc", &ok);

			if(ok == true)
				dlist_ins(ctx->map->lstMonsters, mm);
			else
				free(mm);
		}

		SetLoadSaveStatus(true, "Triggers");
		// load triggers
		if(ok == true) count = get_u16(ini, "triggers", "count", &ok);

		for(c=0; ok == true && c < count; c++)
		{
			uMapTrigger *mt;

			mt = (uMapTrigger*)calloc(1, sizeof(uMapTrigger));
			assert(mt != NULL);
			assert(ctx->map->lstTriggers != NULL);

			sprintf(key, "trigger_%03u", c);
			if(ok == true) mt->row = get_u8(ini, key, "row", &ok);
			if(ok == true) mt->col = get_u8(ini, key, "col", &ok);
			if(ok == true) mt->type = get_u8(ini, key, "type", &ok);

			if(ok == true)
			{
				switch(mt->type)
				{
					case eTrigger_SignOnWall:
						ok = load_ue64(ini, key, "data", data, 512);
						if(ok == true)
							mt->data.t_sign_on_wall.msg = strdup((char *)data);
						else
							mt->data.t_sign_on_wall.msg = strdup("<< bad message >>");
						break;
				}
			}

			if(ok == true)
				dlist_ins(ctx->map->lstTriggers, mt);
			else
				free(mt);
		}

		INI_unload(ini);
		if(workstationCactus == false)
		{
			remove(cfgFile);
		}
	}

	free(cfgFile);

	return ok;
}


static bool LoadPlayer(uContext *ctx)
{
	INIFILE *ini;
	bool rc = false;
	uint16_t q;
	uint16_t c;
	char key[32];
	char lbuff[1024];
	char *cfgFile;

	SetLoadSaveStatus(true, "Player");
	cfgFile = GetConfigurationFile(FILE_PLAYER);
	ini = INI_load(cfgFile);

	if(ini != NULL)
	{
		rc = true;
		rc = load_ue64(ini, "player", "name", ctx->player->name, MAX_NAME_LEN);
		if(rc == true) ctx->player->score = get_u32(ini, "player", "s", &rc);
		if(rc == true) ctx->player->race = get_u8(ini, "player", "r", &rc);
		if(rc == true) ctx->player->money = get_u16(ini, "player", "m", &rc);
		if(rc == true) ctx->player->armour = get_i16(ini, "player", "a", &rc);
		if(rc == true) ctx->player->level = get_u8(ini, "player", "l", &rc);
		if(rc == true) ctx->player->experience = get_u16(ini, "player", "x", &rc);
		if(rc == true) ctx->player->level_up_points = get_u8(ini, "player", "u", &rc);
		if(rc == true) ctx->player->life = get_i16(ini, "player", "l0", &rc);
		if(rc == true) ctx->player->max_life = get_i16(ini, "player", "l1", &rc);
		if(rc == true) ctx->player->mana = get_i16(ini, "player", "m0", &rc);
		if(rc == true) ctx->player->max_mana = get_i16(ini, "player", "m1", &rc);

		if(rc == true) ctx->player->stats[eStat_Speed] = get_u8(ini, "player", "st0", &rc);
		if(rc == true) ctx->player->stats[eStat_DeepVision] = get_u8(ini, "player", "st1", &rc);
		if(rc == true) ctx->player->stats[eStat_Dexterity] = get_u8(ini, "player", "st2", &rc);
		if(rc == true) ctx->player->stats[eStat_Willpower] = get_u8(ini, "player", "st3", &rc);
		if(rc == true) ctx->player->stats[eStat_Strength] = get_u8(ini, "player", "st4", &rc);

		if(rc == true) ctx->player->base_stats[eStat_Speed] = get_u8(ini, "player", "bst0", &rc);
		if(rc == true) ctx->player->base_stats[eStat_DeepVision] = get_u8(ini, "player", "bst1", &rc);
		if(rc == true) ctx->player->base_stats[eStat_Dexterity] = get_u8(ini, "player", "bst2", &rc);
		if(rc == true) ctx->player->base_stats[eStat_Willpower] = get_u8(ini, "player", "bst3", &rc);
		if(rc == true) ctx->player->base_stats[eStat_Strength] = get_u8(ini, "player", "bst4", &rc);

		if(rc == true) ctx->player->skills[eSkill_ItemLore] = get_u8(ini, "player", "sk0", &rc);
		if(rc == true) ctx->player->skills[eSkill_Magic] = get_u8(ini, "player", "sk1", &rc);
		if(rc == true) ctx->player->skills[eSkill_RangedWeapons] = get_u8(ini, "player", "sk2", &rc);
		if(rc == true) ctx->player->skills[eSkill_MeleeWeapons] = get_u8(ini, "player", "sk3", &rc);

		if(rc == true) ctx->player->loadout = get_u8(ini, "player", "lo", &rc);
		if(rc == true) ctx->player->item_left_weap = get_u8(ini, "player", "i0", &rc);
		if(rc == true) ctx->player->item_right_weap = get_u8(ini, "player", "i1", &rc);
		if(rc == true) ctx->player->item_body = get_u8(ini, "player", "i2", &rc);
		if(rc == true) ctx->player->item_head = get_u8(ini, "player", "i3", &rc);
		if(rc == true) ctx->player->item_left_hand = get_u8(ini, "player", "i4", &rc);
		if(rc == true) ctx->player->item_right_hand = get_u8(ini, "player", "i5", &rc);

		if(rc == true) ctx->player->loadout_item_left_weap = get_u8(ini, "player", "loi0", &rc);
		if(rc == true) ctx->player->loadout_item_right_weap = get_u8(ini, "player", "loi1", &rc);
		if(rc == true) ctx->player->loadout_item_body = get_u8(ini, "player", "loi2", &rc);
		if(rc == true) ctx->player->loadout_item_head = get_u8(ini, "player", "loi3", &rc);
		if(rc == true) ctx->player->loadout_item_left_hand = get_u8(ini, "player", "loi4", &rc);
		if(rc == true) ctx->player->loadout_item_right_hand = get_u8(ini, "player", "loi5", &rc);

		if(rc == true) ctx->player->target_idx = get_u16(ini, "player", "ti", &rc);
		if(rc == true) ctx->player->monster_fov_selected_idx = get_u8(ini, "player", "mfsi", &rc);
		if(rc == true) ctx->player->monster_fov_idx = get_u8(ini, "player", "mfi", &rc);

		SetLoadSaveStatus(true, "Counters");
		q = INI_get_unumber(ini, "counters", "count", 0, INT16_MAX, 0);
		for(c = 0; rc == true && c < q && c < MAX_COUNTER_ITEMS; c++)
		{
			sprintf(key, "counter_%03u", c);
			ctx->player->counters[c].effect = get_u16(ini, key, "effect", &rc);
			ctx->player->counters[c].rounds = get_u16(ini, key, "rounds", &rc);
			ctx->player->counters[c].adjust = get_i16(ini, key, "value", &rc);
		}

		SetLoadSaveStatus(true, "Inventory");
		for(c = 0; rc == true && c < MAX_INVENTORY_ITEMS; c++)
		{
			uItem *item;
			uint16_t idx;

			sprintf(key, "item_%03u", c);
			ctx->player->items[c].count = 0;
			ctx->player->items[c].item.idx = 0;

			idx = 0;
			if(rc == true) ctx->player->items[c].count = get_u8(ini, key, "count", &rc);
			if(rc == true) idx = get_u16(ini, key, "index", &rc);

			if(rc == true && idx > 0)
			{
				// copy item
				item = FindItemByID(ctx, idx);
				assert(item != NULL);
				GenerateCopy(item, &ctx->player->items[c].item);
				assert(ctx->player->items[c].item.idx == idx);

				// now suck in changed attributes
				load_ue64(ini, key, "attr", (uint8_t*)lbuff, 1024);
				if(rc == true)
				{
					rc = reconstruct_item_attributes(&ctx->player->items[c].item, lbuff);
				}
			}
		}

		INI_unload(ini);
		if(workstationCactus == false)
			remove(cfgFile);
	}

	free(cfgFile);

	return rc;
}

// this chunk is called at top level and only at start, so nothing should be modified
// at this point.
bool LoadGame(uContext *ctx)
{
	INIFILE *ini;
	bool ok;
	uint16_t i;
	char key[32];
	char *p;
	uint16_t count;
	char lbuff[1024];
	char *cfgFile;

	assert(ctx->player != NULL);
	assert(ctx->map != NULL);

	// load game.
	ClearPlayer(ctx);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Load Core data
	SetLoadSaveStatus(true, "Core");

	cfgFile = GetConfigurationFile(FILE_CORE);
	ini = INI_load(cfgFile);

	if(ini == NULL)
	{
		free(cfgFile);
		return false;
	}

	ok = true;

	ctx->mgen_count = get_u16(ini, "core", "mgc", &ok);
	if(ok == true) ctx->moves = get_u32(ini, "core", "mv", &ok);
	if(ok == true) ctx->locn.level = get_u8(ini, "core", "ll", &ok);
	if(ok == true) ctx->locn.row = get_u8(ini, "core", "lr", &ok);
	if(ok == true) ctx->locn.col = get_u8(ini, "core", "lc", &ok);
	if(ok == true) ctx->locn.map = get_u8(ini, "core", "lm", &ok);
	if(ok == true) ctx->draw_top_col = get_u16(ini, "core", "dtc", &ok);
	if(ok == true) ctx->draw_top_row = get_u16(ini, "core", "dtr", &ok);
	if(ok == true) ctx->draw_col = get_u16(ini, "core", "dc", &ok);
	if(ok == true) ctx->draw_row = get_u16(ini, "core", "dr", &ok);

	for(i=0; i < eFLAGS_MAX && ok == true; i++)
	{
		sprintf(key, "flag_%02u", i);
		p = INI_get(ini, "flags", key);
		if(p != NULL)
		{
			bit_set_bit(ctx->game_flags, i, p[0] == 'f' ? 0 : 1);
		}
	}

	SetLoadSaveStatus(true, "Messages");
	count = get_u16(ini,"messages", "count", &ok);
	for(i=0; i < count && ok == true; i++)
	{
		uMessage *m = (uMessage*)calloc(1, sizeof(uMessage));
		assert(m != NULL);
		assert(ctx->lstMessages != NULL);

		sprintf(key, "message_%03u", i);
		m->count = get_u8(ini, key, "count", &ok);
		if(ok == true)
		{
			ok = load_ue64(ini, key, "data", (uint8_t*)lbuff, 1024);
			if(ok == true)
			{
				m->msg = strdup((char *)lbuff);
			}
			else
			{
				m->msg = strdup("<<Missing Message>>");
			}

			dlist_ins(ctx->lstMessages, m);
		}
		else
		{
			free(m);
		}
	}

	SetLoadSaveStatus(true, "Items");
	count = get_u16(ini, "items", "count", &ok);
	for(i=0; i < count && ok == true; i++)
	{
		uItem *item;
		uint16_t idx;

		sprintf(key, "item_%03u", i);

		idx = get_u16(ini, key, "idx", &ok);
		if(ok == true)
		{
			item = FindItemByID(ctx, idx);
			if(item != NULL)
			{
				ok = load_ue64(ini, key, "data", (uint8_t*)lbuff, 1024);
				if(ok == true)
				{
					ok = reconstruct_item_attributes(item, lbuff);
				}
				else
				{
					ok = false;
				}
			}
			else
			{
				ok = false;
			}
		}
	}

	INI_unload(ini);
	if(workstationCactus == false)
	{
		remove(cfgFile);
	}
	free(cfgFile);

	if(ok == true)
		ok = LoadPlayer(ctx);

	if(ok == true)
		ok = LoadMap(ctx);

	// incase screen has changed resolution
	calc_draw_offseet(ctx);

	return ok;
}

