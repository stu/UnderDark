#include "headers.h"

void FreeMonster(void *x)
{
	uMonster *i = (uMonster *)x;

	if(i == NULL)
	{
		return;
	}

	memset(i, 0x0, sizeof(uMonster));
	free(i);
}


static uMonster *NewMonster(uContext *ctx, char *name, uint8_t tile, uint8_t colour, int16_t life, int16_t armour, int16_t xp, uint8_t type, uint8_t disposition,
							uint8_t attack_type, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t attack_level, uint8_t defend_level,
							uint8_t dropclass, uint8_t min_level, uint8_t max_level)
{
	uMonster *m = calloc(1, sizeof(uMonster));
	assert(m != NULL);

	strncpy((char *)m->name, name, MAX_MONSTER_NAME);
	m->type = type;
	m->tile = tile;
	m->colour = colour;
	m->disposition = disposition;
	m->attack_type = attack_type;
	m->attack_dice[0] = d1;
	m->attack_dice[1] = d2;
	m->attack_dice[2] = d3;
	m->life = life;
	m->armour = armour;
	m->xp = xp;
	m->attack_level = attack_level;
	m->defend_level = defend_level;
	m->drop_class = dropclass;
	m->min_level = min_level;
	m->max_level = max_level;

	m->idx = 1 + dlist_size(ctx->lstMonsters);

	dlist_ins(ctx->lstMonsters, m);

	return m;
}

int16_t BuildMonsters(uContext *ctx)
{
#include "monster.gen"

	return 0;
}


uMonster *GetMonsterByName(uContext *ctx, char *name)
{
	DLElement *e = dlist_head(ctx->lstMonsters);
	while(e != NULL)
	{
		uMonster *m = dlist_data(e);
		e = dlist_next(e);

		if(stricmp((char *)m->name, name) == 0)
		{
			return m;
		}
	}

	return NULL;
}

uMonster *GetMonsterByID(uContext *ctx, uint16_t id)
{
	DLElement *e = dlist_head(ctx->lstMonsters);
	while(e != NULL)
	{
		uMonster *m = dlist_data(e);
		e = dlist_next(e);

		if(m->idx == id)
		{
			return m;
		}
	}

	return NULL;
}

uMapMonster *GetMonsterOnMap(uContext *ctx, uint16_t row, uint16_t col)
{
	DLElement *e = dlist_head(ctx->map->lstMonsters);
	while(e != NULL)
	{
		uMapMonster *d = dlist_data(e);
		e = dlist_next(e);

		if(d->row == row && d->col == col)
		{
			return d;
		}
	}

	return NULL;
}

uMapMonster *BuildMonster(uContext *ctx, uMonster *m, uint16_t r, uint16_t c)
{
	uMapMonster *mm;

	mm = (uMapMonster *)calloc(1, sizeof(uMapMonster));
	assert(mm != NULL);
	mm->row = r;
	mm->col = c;
	mm->disposition = m->disposition;
	mm->health = m->life;
	mm->armour = m->armour;
	mm->monster_id = m->idx;
	mm->last_player_row = UINT8_MAX;
	mm->last_player_col = UINT8_MAX;
	dlist_ins(ctx->map->lstMonsters, mm);

	ctx->mgen_count += 1;
	mm->gen_id = ctx->mgen_count;

	return mm;
}

/// Also gens drops
void KillMonster(uContext *ctx, uMapMonster *mm)
{
	DLElement *e;
	uMapMonster *x;

	// NOTE: for every monster killed get three bonus rest points back!
	ctx->map->recovery += 3;

	e = dlist_head(ctx->map->lstMonsters);
	while(e != NULL)
	{
		x = dlist_data(e);

		if(x == mm)
		{
			uMonster *mon;

			mon = GetMonsterByID(ctx, mm->monster_id);
			switch(mon->drop_class)
			{
				case eDC_Nothing:
					break;

					// large money is an always drop from mimics + leprechauns
				case eDC_LargeMoney:
					GenLargeGold(ctx, mm->row, mm->col);
					break;

				case eDC_SmallMoney:
					// 10% drop money
					if(chance(10))
					{
						GenSmallGold(ctx, mm->row, mm->col);
					}
					break;

				case eDC_LowMagicWeaponArmour:
					if(chance(15))
					{
						switch(xrnd(5))
						{
							// gold
							case 0:
								GenSmallGold(ctx, mm->row, mm->col);
								break;

								// weapons
							case 1:
								GenItemByLevel(ctx, mm->row, mm->col, GEARDROP_L_WEAPON, 0);
								break;

								// armour
							case 2:
								GenItemByLevel(ctx, mm->row, mm->col, GEARDROP_L_ARMOUR, 0);
								break;

								// magic and stuff
							case 3:
								GenItemByLevel(ctx, mm->row, mm->col, GEARDROP_MAGIC, 0);
								break;

							case 4:
								GenItemByLevel(ctx, mm->row, mm->col, GEARDROP_TREASURE, 0);
								break;
						}
					}
					break;

				case eDC_LowWeaponArmour:
					if(chance(15))
					{
						switch(xrnd(3))
						{
							// gold
							case 0:
								GenSmallGold(ctx, mm->row, mm->col);
								break;

								// weapons
							case 1:
								GenItemByLevel(ctx, mm->row, mm->col, GEARDROP_L_WEAPON, 0);
								break;

								// armour
							case 2:
								GenItemByLevel(ctx, mm->row, mm->col, GEARDROP_L_ARMOUR, 0);
								break;
						}
					}
					break;

					// TODO gen midlevel stuff
				case eDC_MidWeaponArmour:
					if(chance(15))
					{
						switch(xrnd(4))
						{
							// gold
							case 0:
								GenSmallGold(ctx, mm->row, mm->col);
								break;

								// weapons
							case 1:
								GenItemByLevel(ctx, mm->row, mm->col, GEARDROP_M_WEAPON, 0);
								break;

								// armour
							case 2:
								GenItemByLevel(ctx, mm->row, mm->col, GEARDROP_M_ARMOUR, 0);
								break;

							case 3:
								GenItemByLevel(ctx, mm->row, mm->col, GEARDROP_TREASURE, 0);
								break;
						}
					}
					break;

				case eDC_MidMagicWeaponArmour:
					if(chance(15))
					{
						switch(xrnd(5))
						{
							// gold
							case 0:
								GenSmallGold(ctx, mm->row, mm->col);
								break;

								// weapons
							case 1:
								GenItemByLevel(ctx, mm->row, mm->col, GEARDROP_M_WEAPON, 0);
								break;

								// armour
							case 2:
								GenItemByLevel(ctx, mm->row, mm->col, GEARDROP_M_ARMOUR, 0);
								break;

								// magic and stuff
							case 3:
								GenItemByLevel(ctx, mm->row, mm->col, GEARDROP_MAGIC, 0);
								break;

							case 4:
								GenItemByLevel(ctx, mm->row, mm->col, GEARDROP_TREASURE, 0);
								break;
						}
					}
					break;

				case eDC_HighWeaponArmour:
					if(chance(15))
					{
						switch(xrnd(4))
						{
							// gold
							case 0:
								GenSmallGold(ctx, mm->row, mm->col);
								break;

								// weapons
							case 1:
								GenItemByLevel(ctx, mm->row, mm->col, GEARDROP_H_WEAPON, 0);
								break;

								// armour
							case 2:
								GenItemByLevel(ctx, mm->row, mm->col, GEARDROP_H_ARMOUR, 0);
								break;

							case 3:
								GenItemByLevel(ctx, mm->row, mm->col, GEARDROP_TREASURE, 0);
								break;
						}
					}
					break;
				case eDC_HighMagicWeaponArmour:
					if(chance(15))
					{
						switch(xrnd(5))
						{
							// gold
							case 0:
								GenSmallGold(ctx, mm->row, mm->col);
								break;

								// weapons
							case 1:
								GenItemByLevel(ctx, mm->row, mm->col, GEARDROP_H_WEAPON, 0);
								break;

								// armour
							case 2:
								GenItemByLevel(ctx, mm->row, mm->col, GEARDROP_H_ARMOUR, 0);
								break;

								// magic and stuff
							case 3:
								GenItemByLevel(ctx, mm->row, mm->col, GEARDROP_MAGIC, 0);
								break;

							case 4:
								GenItemByLevel(ctx, mm->row, mm->col, GEARDROP_TREASURE, 0);
								break;
						}
					}
					break;

				case eDC_Win:
					bit_set_bit(ctx->game_flags, eFlag_GameWin, true);
					GenWin(ctx, mm->row, mm->col);
					break;
			}

			dlist_remove(ctx->map->lstMonsters, e, (void **)&x);
			FreeMapMonster(x);
			return;
		}

		e = dlist_next(e);
	}
}



