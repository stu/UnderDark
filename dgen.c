#include "headers.h"

#define STACK_SIZE 512
static uint16_t stackPtr, maxPtr;
static uint16_t stack[STACK_SIZE];

struct
{
	uint8_t monster_quad;
	uint8_t item_quad;
	uint8_t trap_quad;
	uint8_t max_row;
	uint8_t max_col;
	bool stairs_down;
	bool stairs_up;
} generational_matrix[] =
	{
		// level 0
		{0, 0, 0, 0, 0, false, false},

		// level 1
		{6, 6, 6, 80, 80, true, false},
		// level 2
		{6, 6, 6, 95, 95, true, true},
		// level 3
		{7, 6, 6, 110, 110, true, true},
		// level 4
		{7, 6, 6, 120, 120, true, true},
		// level 5
		{8, 6, 6, 130, 130, true, true},
		// level 6
		{8, 6, 6, 140, 140, true, true},
		// level 7
		{9, 6, 6, 150, 150, true, true},
		// level 8
		{9, 6, 6, 160, 160, true, true},
		// level 9
		{10, 6, 6, 170, 170, true, true},
		// level 10
		{10, 6, 6, 180, 180, false, true},
	};

static void spush(uint16_t coord)
{
	assert(stackPtr < STACK_SIZE);

	stack[stackPtr] = coord;
	stackPtr += 1;

	if(stackPtr > maxPtr)
	{
		maxPtr = stackPtr;
	}
}

static uint16_t spop(void)
{
	if(stackPtr == 0)
	{
		return UINT16_MAX;
	}

	stackPtr -= 1;
	return stack[stackPtr];
}

static bool IsOkFillHere(uContext *ctx, uint16_t col, uint16_t row, uint8_t **map)
{
	if(col < ctx->map->max_cols && row < ctx->map->max_rows)
	{
		if(map[row][col] == 1)
		{
			return true;
		}
	}

	return false;

//	if (col >= ctx->map->max_cols-1 || row >= ctx->map->max_rows-1)
//		return false;
//
//	if (map[row][col] != 1)
//		return false;
//
//	return true;
}

static void FloodFillLine(uContext *ctx, uint16_t col, uint16_t row, uint8_t **map)
{
	uint16_t c;
	uint16_t newspanUp, newspanDown;

	if(IsOkFillHere(ctx, col, row, map) == false)
	{
		return;
	}

	/* Scan for left border */
	for(c = col - 1; IsOkFillHere(ctx, c, row, map) == true; c--)
	{
		//
	}

	newspanUp = newspanDown = 1;

	for(c++; IsOkFillHere(ctx, c, row, map) == true; c++)
	{
		map[row][c] = 100;

		if(IsOkFillHere(ctx, c, row - 1, map) == true)
		{
			if(newspanUp == 1)
			{
				spush(c + ((row - 1) << 8));
				newspanUp = 0;
			}
		}
		else
		{
			newspanUp = 1;
		}

		if(IsOkFillHere(ctx, c, row + 1, map) == true)
		{
			if(newspanDown == 1)
			{
				spush(c + ((row + 1) << 8));
				newspanDown = 0;
			}
		}
		else
		{
			newspanDown = 1;
		}
	}
}

static void FloodFill(uContext *ctx, uint16_t col, uint16_t row, uint8_t **map)
{
	stackPtr = 0;
	maxPtr = 0;

	assert(col < 256 && row < 256);

	spush(col + (row << 8));

	for(;;)
	{
		uint16_t c = spop();

		/* Exit if stack is empty */
		if(c == UINT16_MAX)
		{
			break;
		}

		col = c & 0xFF;
		row = c >> 8;

		FloodFillLine(ctx, col, row, map);
	}

	stackPtr = 0;
}

static void cell_generation(uContext *ctx, uint8_t **grid, uint16_t r1_cutoff, uint16_t r2_cutoff)
{
	int16_t max_cols;
	int16_t max_rows;

	int16_t c, r, rows_ii, cols_jj;

	max_cols = ctx->map->max_cols - 1;
	max_rows = ctx->map->max_rows - 1;

	for(r = 2; r < max_rows - 2; r++)
	{
		for(c = 2; c < max_cols - 2; c++)
		{
			int16_t adjcount_r1 = 0, adjcount_r2 = 0;

			for(rows_ii = -1; rows_ii <= 1; rows_ii++)
			{
				for(cols_jj = -1; cols_jj <= 1; cols_jj++)
				{
					if(grid[r + rows_ii][c + cols_jj] != 1)
					{     // floor
						adjcount_r1++;
					}
				}
			}

			for(rows_ii = r - 2; rows_ii <= r + 2; rows_ii++)
			{
				for(cols_jj = c - 2; cols_jj <= c + 2; cols_jj++)
				{
					if(abs(rows_ii - r) == 2 && abs(cols_jj - c) == 2)
					{
						continue;
					}

					if(rows_ii < 2 || cols_jj < 2 || rows_ii >= max_rows - 2 || cols_jj >= max_cols - 2)
					{
						continue;
					}

					if(grid[rows_ii][cols_jj] != 1)
					{         // floor
						adjcount_r2++;
					}
				}
			}

			if(adjcount_r1 >= r1_cutoff || adjcount_r2 <= r2_cutoff)
			{
				dig_cell(ctx, r, c, eTile_SmoothStoneWall, TILE_FLAG_OPAQUE);
			}
			else
			{
				dig_cell(ctx, r, c, eTile_StoneFloor, 0);
			}
		}
	}

	for(r = 0; r < max_rows; r++)
	{
		for(c = 0; c < max_cols; c++)
		{
			if(get_cell_tile(ctx, r, c) == eTile_StoneFloor)
			{
				grid[r][c] = 1;
			}
			else
			{
				grid[r][c] = 0;
			}
		}
	}
}

static void cellgrow(uContext *ctx)
{
	uint8_t **cmap;
	uint16_t NEST_SIZE_COLS;
	uint16_t NEST_SIZE_ROWS;
	uint16_t col, row;
	uint16_t tcount;

	uint16_t saved_y, saved_x;
	uint16_t qq;

	NEST_SIZE_COLS = ctx->map->max_cols - 1;
	NEST_SIZE_ROWS = ctx->map->max_rows - 1;

	qq = (NEST_SIZE_COLS / 3) * (NEST_SIZE_ROWS / 3);
	qq *= 2;
	//qq = (NEST_SIZE_COLS / 2) * (NEST_SIZE_ROWS / 2);

	cmap = malloc(sizeof(uint8_t *) * NEST_SIZE_ROWS + 1);
	assert(cmap != NULL);

	for(row = 0; row < NEST_SIZE_ROWS; row++)
	{
		cmap[row] = malloc(NEST_SIZE_COLS + 1);
		assert(cmap[row] != NULL);
	}

	tcount = 0;
	while(tcount < qq)
	{
		// clear out our map size!
		for(row = 0; row < NEST_SIZE_ROWS; row++)
		{
			for(col = 0; col < NEST_SIZE_COLS; col++)
			{
				cmap[row][col] = 0;
				//set_tile(ctx, row, col, t_Wall);
				dig_cell(ctx, row, col, eTile_SmoothStoneWall, TILE_FLAG_OPAQUE);
			}
		}

		// mark 45% as floor?
		{
			uint16_t fillout;

			// mark out 80% as floor
			fillout = NEST_SIZE_COLS;
			fillout *= NEST_SIZE_ROWS;
			fillout /= 100;
			fillout *= 80;

			while(fillout > 0)
			{
				col = 2 + xrnd(NEST_SIZE_COLS - 4);
				row = 2 + xrnd(NEST_SIZE_ROWS - 4);

				if(row >= 2 && row < (NEST_SIZE_ROWS - 2) && col >= 2 && col < (NEST_SIZE_COLS - 2))
				{
					cmap[row][col] = 1;
					//set_tile(ctx, row, col, t_Floor);
					dig_cell(ctx, row, col, eTile_StoneFloor, 0);
					fillout -= 1;
				}
			}
		}
		// 14 means what? I forget...

		// was 15, reduced to 5... lets test it out
		for(col = 0; col < 15; col++)
		{
			cell_generation(ctx, cmap, 5, 2);
		} // 5, 2
		cell_generation(ctx, cmap, 5, 0); // 5,0
		col = 0;
		while(col == 0)
		{
			uint16_t pr, pc;

			pr = NEST_SIZE_ROWS / 4;
			pc = NEST_SIZE_COLS / 4;

			pr = pr + xrnd(NEST_SIZE_ROWS / 2);
			pc = pc + xrnd(NEST_SIZE_COLS / 2);
			if(cmap[pr][pc] == 1)
			{
				saved_x = pc;
				saved_y = pr;
				col = 1;
			}
		}

		stackPtr = 0;
		maxPtr = 0;
		FloodFill(ctx, saved_x, saved_y, cmap);
		tcount = 0;
		for(row = 0; row < NEST_SIZE_ROWS; row++)
		{
			for(col = 0; col < NEST_SIZE_COLS; col++)
			{
				if(cmap[row][col] == 100)
				{
					tcount += 1;
				}
				else
				{
					cmap[row][col] = 0;
					//set_tile(ctx, row, col, t_Wall);
					dig_cell(ctx, row, col, eTile_SmoothStoneWall, TILE_FLAG_OPAQUE);
				}
			}
		}
		//add_msg(ctx, "Max Stack Pointer = %i, tcount = %i", maxPtr, tcount);
	}

	for(row = 0; row < NEST_SIZE_ROWS; row++)
	{
		free(cmap[row]);
	}
	free(cmap);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void clear_player_level_seen(uContext *ctx)
{
	uint16_t r;
	uint16_t c;

	for(r = 0; r < MAX_MAP_HEIGHT; r++)
	{
		for(c = 0; c < MAX_MAP_WIDTH; c++)
		{
			ctx->map->cells[(MAX_MAP_WIDTH * r) + c].tile.flags &= ~TILE_FLAG_SEEN;
		}
	}
}

uint8_t get_cell_tile(uContext *ctx, uint16_t r, uint16_t c)
{
	if(r < MAX_MAP_HEIGHT && c < MAX_MAP_WIDTH)
	{
		return ctx->map->cells[(MAX_MAP_WIDTH * r) + c].tile.tile;
	}

	return UINT8_MAX;
}

void set_cell_tile(uContext *ctx, uint16_t r, uint16_t c, uint8_t tile)
{
	if(r < MAX_MAP_HEIGHT && c < MAX_MAP_WIDTH)
	{
		ctx->map->cells[(MAX_MAP_WIDTH * r) + c].tile.tile = tile;
	}
}

uint8_t get_cell_flags(uContext *ctx, uint16_t r, uint16_t c)
{
	if(r < MAX_MAP_HEIGHT && c < MAX_MAP_WIDTH)
	{
		return ctx->map->cells[(MAX_MAP_WIDTH * r) + c].tile.flags;
	}

	return UINT8_MAX;
}

void set_cell_flags(uContext *ctx, uint16_t r, uint16_t c, uint8_t flags)
{
	if(r < MAX_MAP_HEIGHT && c < MAX_MAP_WIDTH)
	{
		ctx->map->cells[(MAX_MAP_WIDTH * r) + c].tile.flags = flags;
	}
}


void or_cell_flags(uContext *ctx, uint16_t r, uint16_t c, uint8_t flags)
{
	if(r < MAX_MAP_HEIGHT && c < MAX_MAP_WIDTH)
	{
		ctx->map->cells[(MAX_MAP_WIDTH * r) + c].tile.flags |= flags;
	}
}

void dig_cell(uContext *ctx, uint16_t r, uint16_t c, uint8_t tile_type, uint8_t tile_flags)
{
	if(r < MAX_MAP_HEIGHT && c < MAX_MAP_WIDTH)
	{
		ctx->map->cells[(MAX_MAP_WIDTH * r) + c].tile.tile = tile_type;
		ctx->map->cells[(MAX_MAP_WIDTH * r) + c].tile.flags = tile_flags;
	}
}

static void digbox_at(uContext *ctx, uint16_t r, uint16_t c, uint16_t h, uint16_t w, uint8_t tile, uint8_t flags)
{
	uint16_t hh, ww;

	for(hh = 0; hh < h; hh++)
	{
		for(ww = 0; ww < w; ww++)
		{
			dig_cell(ctx, r + hh, c + ww, tile, flags);
		}
	}
}


static uint16_t scan_block_floor_count(uContext *ctx, uint16_t h, uint16_t w, uint16_t r, uint16_t c)
{
	uint16_t rr, cc;
	uint16_t t = 0;

	for(rr = 0; rr < h; rr++)
	{
		for(cc = 0; cc < w; cc++)
		{
			if(get_cell_tile(ctx, (r + rr), (c + cc)) == eTile_StoneFloor)
			{
				t += 1;
			}
		}
	}

	return t;
}

static void GenGold(uContext *ctx, uint16_t row, uint16_t col, int16_t amount)
{
	uMapItem *i;

	i = (uMapItem *)calloc(1, sizeof(uMapItem));
	assert(i != NULL);
	i->row = row;
	i->col = col;
	i->type = eDT_Gold;
	i->data.i_gold.gold = amount;
	dlist_ins(ctx->map->lstItems, i);
}

void GenLargeGold(uContext *ctx, uint16_t row, uint16_t col)
{
	GenGold(ctx, row, col, 50 + xrnd(50));
}

void GenSmallGold(uContext *ctx, uint16_t row, uint16_t col)
{
	GenGold(ctx, row, col, 3 + xrnd(25));
}

void GenWin(uContext *ctx, uint16_t row, uint16_t col)
{
	uMapItem *i;
	uItem *item = NULL;

	item = FindItemByName(ctx, ITEM_NAME_CROWN_DARKNESS);
	i = (uMapItem *)calloc(1, sizeof(uMapItem));
	assert(i != NULL);
	i->row = row;
	i->col = col;
	i->type = eDT_Armour;
	i->data.i_item.has_item_data = false;
	i->data.i_item.idx = item->idx;
	dlist_ins(ctx->map->lstItems, i);

	add_msg(ctx, "You see the fabled `%s` fall to the floor!", item->name);
	add_msg(ctx, "Congratulations, you have slain the undead king and nearly won the game!");
}

void GenItemByLevel(uContext *ctx, uint16_t row, uint16_t col, uint16_t flags, uint16_t flags2)
{
	uMapItem *i;
	uItem *item = NULL;
	uint16_t arridx[256];
	int16_t idx;
	DLElement *e = dlist_head(ctx->lstItems);

	idx = 0;
	while(e != NULL && idx < 256)
	{
		uint16_t f;

		item = dlist_data(e);
		e = dlist_next(e);
		f = GetItemAttributeValue_Int(item, eIA_i_DropClass, 0);
		// any flag is ok. no flags not ok.
		// so magic|l_weapon will pull magic items or l_weapons
		if((f & flags) != 0)
		{
			arridx[idx++] = item->idx;
		}

		if(flags2 != 0 && (f & flags2) != 0)
		{
			arridx[idx++] = item->idx;
		}
	}

	i = (uMapItem *)calloc(1, sizeof(uMapItem));
	assert(i != NULL);
	i->row = row;
	i->col = col;

	i->type = eDT_Weapon;
	// create new item from template.
	i->data.i_item.has_item_data = false;

	assert(idx != 0);
	item = FindItemByID(ctx, arridx[xrnd(idx)]);

	if(item != NULL)
	{
		switch(item->base_type)
		{
			case eIT_Consumable:
				if(GetItemAttributeValue_Int(item, eIA_i_PotionBase, INT16_MAX) != INT16_MAX)
				{
					i->type = eDT_Potion;
				}
				else
				{
					i->type = eDT_Food;
				}
				break;

			default:
			case eIT_Torch:
				i->type = eDT_Item;
				break;

			case eIT_Ring:
			case eIT_Shield:
			case eIT_Helmet:
			case eIT_BodyArmour:
				i->type = eDT_Armour;
				break;

			case eIT_1h_Weapon:
			case eIT_2h_Weapon:
			case eIT_AmmunitionForBow:
				i->type = eDT_Weapon;

				// all arrows are stackable
				if(GetItemAttributeValue_Bool(item, eIA_b_Stackable, false) == true)
				{
					i->data.i_item.count = 25 + xrnd(25);
				}
				break;
		}
		i->data.i_item.idx = item->idx;
		dlist_ins(ctx->map->lstItems, i);
	}
}


static void quad_gen_items(uContext *ctx, uint16_t level)
{
	uint16_t zqr, zqc;
	uint16_t qr, qc;
	uint16_t r, c;

	uint16_t gr, gc;

	uint16_t flags2;

	// defaults
	zqr = generational_matrix[level].item_quad;
	zqc = generational_matrix[level].item_quad;

	qr = ctx->map->max_rows / zqr;
	qc = ctx->map->max_rows / zqc;

	flags2 = 0;

	if(level < 7)
	{
		// pull for class leading
		if(ctx->player->skills[eSkill_MeleeWeapons] > ctx->player->skills[eSkill_RangedWeapons]
		   && ctx->player->skills[eSkill_MeleeWeapons] > ctx->player->skills[eSkill_Magic])
		{
			if(level < 4)
			{
				flags2 = GEARDROP_L_ARMOUR | GEARDROP_L_WEAPON;
			}
			else
			{
				flags2 = GEARDROP_M_ARMOUR | GEARDROP_M_WEAPON;
			}
		}
		else if(ctx->player->skills[eSkill_RangedWeapons] > ctx->player->skills[eSkill_MeleeWeapons]
				&& ctx->player->skills[eSkill_RangedWeapons] > ctx->player->skills[eSkill_Magic])
		{
			if(level < 4)
			{
				flags2 = GEARDROP_L_ARMOUR | GEARDROP_L_WEAPON;
			}
			else
			{
				flags2 = GEARDROP_M_ARMOUR | GEARDROP_M_WEAPON;
			}
		}
		else if(ctx->player->skills[eSkill_Magic] > ctx->player->skills[eSkill_MeleeWeapons]
				&& ctx->player->skills[eSkill_Magic] > ctx->player->skills[eSkill_RangedWeapons])
		{
			flags2 = GEARDROP_MAGIC;
		}
	}

	for(r = 0; r < qr; r++)
	{
		for(c = 0; c < qc; c++)
		{
			if(scan_block_floor_count(ctx, qr, qc, qr * r, qc * c) > (qr * qc) / 3)
			{
				while(1)
				{
					gr = (qr * r) + xrnd(qr);
					gc = (qc * c) + xrnd(qc);
					if(get_cell_tile(ctx, gr, gc) == eTile_StoneFloor)
					{
						break;
					}
				}

				assert(get_cell_tile(ctx, gr, gc) == eTile_StoneFloor);

				if(level < 4)
				{
					if(chance(20) == true)
					{
						GenSmallGold(ctx, gr, gc);
					}
					else
					{
						GenItemByLevel(ctx, gr, gc, GEARDROP_MAGIC | GEARDROP_TREASURE | GEARDROP_L_ARMOUR | GEARDROP_L_WEAPON, flags2);
					}
				}
				else if(level >= 4 && level < 7)
				{
					if(chance(20) == true)
					{
						GenSmallGold(ctx, gr, gc);
					}
					else
					{
						GenItemByLevel(ctx, gr, gc, GEARDROP_MAGIC | GEARDROP_TREASURE | GEARDROP_M_ARMOUR | GEARDROP_M_WEAPON, flags2);
					}
				}
				else if(level >= 7)
				{
					if(chance(20) == true)
					{
						GenSmallGold(ctx, gr, gc);
					}
					else
					{
						GenItemByLevel(ctx, gr, gc, GEARDROP_MAGIC | GEARDROP_TREASURE | GEARDROP_H_ARMOUR | GEARDROP_H_WEAPON, flags2);
					}
				}
			}
			else
			{
				// nope
			}
		}
	}
}

static bool IsNextTo(uContext *ctx, uint16_t r, uint16_t c, uint8_t tile)
{
	if(get_cell_tile(ctx, r - 1, c - 1) == tile)
	{ return true; }
	if(get_cell_tile(ctx, r - 1, c) == tile)
	{ return true; }
	if(get_cell_tile(ctx, r - 1, c + 1) == tile)
	{ return true; }

	if(get_cell_tile(ctx, r, c - 1) == tile)
	{ return true; }
	if(get_cell_tile(ctx, r, c + 1) == tile)
	{ return true; }

	if(get_cell_tile(ctx, r + 1, c - 1) == tile)
	{ return true; }
	if(get_cell_tile(ctx, r + 1, c) == tile)
	{ return true; }
	if(get_cell_tile(ctx, r + 1, c + 1) == tile)
	{ return true; }

	return false;
}

static void quad_gen_traps(uContext *ctx, uint16_t level)
{
	uint16_t zqr, zqc;
	uint16_t qr, qc;
	uint16_t r, c;

	uint16_t gr, gc;

	// defaults
	zqr = generational_matrix[level].trap_quad;
	zqc = generational_matrix[level].trap_quad;

	qr = ctx->map->max_rows / zqr;
	qc = ctx->map->max_rows / zqc;

	for(r = 0; r < qr; r++)
	{
		for(c = 0; c < qc; c++)
		{
			if(scan_block_floor_count(ctx, qr, qc, qr * r, qc * c) > (qr * qc) / 3)
			{
				uMapTrigger *trigger;

				while(1)
				{
					gr = (qr * r) + xrnd(qr);
					gc = (qc * c) + xrnd(qc);
					if(get_cell_tile(ctx, gr, gc) == eTile_StoneFloor)
					{
						break;
					}
				}

				assert(get_cell_tile(ctx, gr, gc) == eTile_StoneFloor);

				trigger = calloc(1, sizeof(uMapTrigger));
				assert(trigger != NULL);

				trigger->row = gr;
				trigger->col = gc;
				trigger->type = eTrigger_Trap;
				trigger->data.t_trap.type = xrnd(eT_MAX + 1);

				switch(trigger->data.t_trap.type)
				{
					// change to create none (anything >= et_MAX)
					default:
						free(trigger);
						break;

					case eT_SpoonTrap:
					case eT_SpikeTrap:
					case eT_DartTrap:
					case eT_BearTrap:
						dlist_ins(ctx->map->lstTriggers, trigger);
						break;
					case eT_Teleport:
						while(1)
						{
							trigger->data.t_trap.dest_row = 2 + xrnd(ctx->map->max_rows - 4);
							trigger->data.t_trap.dest_col = 2 + xrnd(ctx->map->max_cols - 4);
							if(get_cell_tile(ctx, trigger->data.t_trap.dest_row, trigger->data.t_trap.dest_col) == eTile_StoneFloor)
							{
								if(GetMapTrigger(ctx, trigger->data.t_trap.dest_row, trigger->data.t_trap.dest_col) == NULL)
								{
									break;
								}
							}
						}
						assert(get_cell_tile(ctx, trigger->data.t_trap.dest_row, trigger->data.t_trap.dest_col) == eTile_StoneFloor);
						dlist_ins(ctx->map->lstTriggers, trigger);
						break;
					case eT_Spiderweb:
						dlist_ins(ctx->map->lstTriggers, trigger);
						break;
					case eT_TrapDoor:
						// trap door... this one should be rare. drops to next level. dont gen on level 10!
						if(chance(20) && generational_matrix[level].stairs_down == true)
							dlist_ins(ctx->map->lstTriggers, trigger);
						else
						{
							free(trigger);
						}
						break;
					case eT_Rockfall:
						// must be next to rocks on some side...
						if(IsNextTo(ctx, gr, gc, eTile_SmoothStoneWall) == true || IsNextTo(ctx, gr, gc, eTile_RoughRockWall) == true)
							dlist_ins(ctx->map->lstTriggers, trigger);
						else
						{
							free(trigger);
						}
						break;
				}
			}
		}
	}
}

static uMonster *GenMonster(uContext *ctx, int level)
{
	uMonster *arrMonster[32];
	int idx;
	int selected;
	DLElement *e;

	e = dlist_head(ctx->lstMonsters);
	idx = 0;
	while(e != NULL && idx < 32)
	{
		uMonster *m = dlist_data(e);
		e = dlist_next(e);

		// 32000 is our faux life for undefined/unfinished monsters.
		if(level >= m->min_level && level <= m->max_level && m->life < 32000)
		{
			arrMonster[idx++] = m;
		}
	}

	// fallback if something happens, just pick one of first 32!
	if(idx == 0)
	{
		e = dlist_head(ctx->lstMonsters);
		idx = 0;
		while(e != NULL && idx < 32)
		{
			uMonster *m = dlist_data(e);
			e = dlist_next(e);
			arrMonster[idx++] = m;
		}
	}

	selected = xrnd(idx);
	return arrMonster[selected];
}

// adds a monster after one dies
void add_monster(uContext *ctx)
{
	int16_t quad;

	// map quadrants
	int16_t qr, qc;

	// player infos
	int16_t pr, pc, pz;

	// generation infos
	int16_t gr, gc, gz;

	uMonster *m;

	quad = generational_matrix[ctx->locn.level].monster_quad;

	qr = ctx->map->max_rows / quad;
	qc = ctx->map->max_cols / quad;

	if(qr < 1)
	{
		qr = 1;
	}
	if(qc < 1)
	{
		qc = 1;
	}

	pr = ctx->locn.row / qr;
	pc = ctx->locn.col / qc;

	if(pr < 1)
	{
		pr = 1;
	}
	if(pc < 1)
	{
		pc = 1;
	}

	pz = (pr * quad) + pc;

	m = NULL;
	while(m == NULL)
	{
		gr = xrnd(quad);
		gc = xrnd(quad);
		gz = (gr * quad) + gc;

		// check our quadrants are different
		if(gz != pz)
		{
			gr = (gr * qr) + xrnd(qr);
			gc = (gc * qc) + xrnd(qc);

			if(get_cell_tile(ctx, gr, gc) == eTile_StoneFloor && GetMonsterOnMap(ctx, gr, gc) == NULL)
			{
				// NOTE: if we add monster height in future, this should be changed!
				// test LOS to me, no spawning within eyesight! also does not track DISTANCE
				if(line_los(ctx, gc, gr, ctx->locn.col, ctx->locn.row) == false)
				{
					m = GenMonster(ctx, ctx->locn.level);
					assert(m != NULL);
					BuildMonster(ctx, m, gr, gc);


					// NOTE: boost monster life as timer so people dont keep scumming?
					//m->life += ctx->mgen_count / 25;
					// // or...
					//m->life += (ctx->locn.level * 3);
				}
			}
		}
	}
}

static void quad_gen_monsters(uContext *ctx, uint16_t level)
{
	uint16_t zqr, zqc;
	uint16_t qr, qc;
	uint16_t r, c;

	uint16_t gr, gc;

	// defaults
	zqr = generational_matrix[level].item_quad;
	zqc = generational_matrix[level].item_quad;

	qr = ctx->map->max_rows / zqr;
	qc = ctx->map->max_rows / zqc;

	for(r = 0; r < qr; r++)
	{
		for(c = 0; c < qc; c++)
		{
			if(scan_block_floor_count(ctx, qr, qc, qr * r, qc * c) > (qr * qc) / 3)
			{
				uMonster *m;
				int counter;

				counter = 0;
				while(counter < 256)
				{
					counter += 1;

					gr = (qr * r) + xrnd(qr);
					gc = (qc * c) + xrnd(qc);
					if(get_cell_tile(ctx, gr, gc) == eTile_StoneFloor)
					{
						// verify no monster on same cell!
						if(GetMonsterOnMap(ctx, gr, gc) == NULL)
						{
							break;
						}
					}
				}

				// make sure test past. give up if we hit 256 tries
				if(get_cell_tile(ctx, gr, gc) == eTile_StoneFloor)
				{
					m = GenMonster(ctx, level);
					assert(m != NULL);
					BuildMonster(ctx, m, gr, gc);
				}
			}
			else
			{
				// cant find it!
			}
		}
	}
}

void FreeMapItem(void *x)
{
	uMapItem *i = (uMapItem *)x;
	if(i == NULL)
	{
		return;
	}

	memset(i, 0x0, sizeof(uMapItem));
	free(i);
}

void FreeMapMonster(void *x)
{
	uMapMonster *m = (uMapMonster *)x;

	if(m == NULL)
	{
		return;
	}

	memset(m, 0x0, sizeof(uMapMonster));
	free(m);
}

void FreeMapTrigger(void *x)
{
	uMapTrigger *t = (uMapTrigger *)x;

	if(t == NULL)
	{
		return;
	}

	switch(t->type)
	{
		case eTrigger_SignOnWall:
			if(t->data.t_sign_on_wall.msg != NULL)
			{
				free(t->data.t_sign_on_wall.msg);
			}
			break;
	}

	memset(t, 0x0, sizeof(uMapTrigger));
	free(t);
}

static void CreateMessageOnWall(uContext *ctx, uint16_t r, uint16_t c, char *msg)
{
	uMapTrigger *t = calloc(1, sizeof(uMapTrigger));
	assert(t != NULL);
	t->row = r;
	t->col = c;

	t->type = eTrigger_SignOnWall;
	t->data.t_sign_on_wall.msg = strdup(msg);

	dlist_ins(ctx->map->lstTriggers, t);
}

static void gen_stairs_down(uContext *ctx)
{
	uint16_t rr;
	uint16_t cc;
	uint16_t h;

	if(generational_matrix[ctx->locn.level].stairs_down == true)
	{
		h = 0;
		while(h == 0)
		{
			rr = xrnd(ctx->map->max_rows);
			cc = xrnd(ctx->map->max_cols);

			if(get_cell_tile(ctx, rr, cc) == eTile_StoneFloor)
			{
				h = 1;
			}
		}

		dig_cell(ctx, rr, cc, eTile_StairsDown, 0);
	}
}

static void gen_stairs_up(uContext *ctx, bool at_stairs)
{
	uint16_t rr;
	uint16_t cc;
	uint16_t h;

	// randomise stairs up
	if(generational_matrix[ctx->locn.level].stairs_up == true)
	{
		h = 0;
		while(h == 0)
		{
			rr = xrnd(ctx->map->max_rows);
			cc = xrnd(ctx->map->max_cols);

			if(get_cell_tile(ctx, rr, cc) == eTile_StoneFloor)
			{
				h = 1;
			}
		}

		dig_cell(ctx, rr, cc, eTile_StairsUp, 0);

		if(at_stairs == true)
		{
			ctx->locn.row = rr;
			ctx->locn.col = cc;
		}
	}
}


static void gen_map1_level1(uContext *ctx, bool at_stairs)
{
	uint16_t h, w;
	uint16_t rr, cc;
	int16_t msg_count;
	char *msgs[] =
		{
			//  ---------x---------x---------x---------x---------x---------x---------x---------x
			"A small brass plaque on the wall reads 'Beware what lurks below!'",
			"A small brass plaque on the wall reads 'Ken A woz ere'",
			"A small brass plaque on the wall reads 'Kilroy was here'",
			"A small brass plaque on the wall reads 'You have no friends here'",
			"A small brass plaque on the wall reads 'The begining is your end'",
			"A small brass plaque on the wall reads 'Beware what lurks below!'",
			"There is a small brass plaque on the wall that is all scratched up",
			"A small brass plaque on the wall reads 'mfg 2022, Boston Ironworks",
			"A small brass plaque on the wall reads 'In loving memory of {scratched out}'",
			"A small brass plaque on the wall reads 'Rent this space for $5",
			"A faded outline on the brickwork, probably a sign at one time.",
			"Chisel and gouge marks show something was removed from here.",
			NULL
		};

	msg_count = 0;
	while(msgs[msg_count] != NULL)
	{
		msg_count += 1;
	}

	ctx->map->max_cols = generational_matrix[ctx->locn.level].max_col;
	ctx->map->max_rows = generational_matrix[ctx->locn.level].max_row;
	if(ctx->map->max_cols > MAX_MAP_WIDTH - 4)
	{
		ctx->map->max_cols = MAX_MAP_WIDTH - 4;
	}
	if(ctx->map->max_rows > MAX_MAP_HEIGHT - 4)
	{
		ctx->map->max_rows = MAX_MAP_HEIGHT - 4;
	}

	cellgrow(ctx);

	// randomise stairs down
	gen_stairs_down(ctx);
	gen_stairs_up(ctx, at_stairs);

	// build out our safe start zone

	h = 0;
	while(h == 0)
	{
		rr = ctx->map->max_rows - 1;
		cc = 8 + xrnd(ctx->map->max_cols - 16);

		for(w = 0; w < 15 && h == 0; w++)
		{
			if(get_cell_tile(ctx, rr - w, cc) == eTile_StoneFloor)
			{
				h = 1;
			}
		}
	}

	// we found floor at rr-w.cc
	while(w > 0)
	{
		dig_cell(ctx, rr - w, cc, eTile_StoneFloor, 0);
		w -= 1;
	}

	// dig hallway back
	//rr += 1;
	dig_cell(ctx, rr, cc - 1, eTile_SmoothStoneWall, TILE_FLAG_OPAQUE);
	dig_cell(ctx, rr, cc + 0, eTile_ClosedDoor, TILE_FLAG_OPAQUE);
	//dig_cell(ctx, rr, cc + 0, eTile_StoneFloor, 0);
	dig_cell(ctx, rr, cc + 1, eTile_SmoothStoneWall, TILE_FLAG_OPAQUE);

	rr += 1;
	dig_cell(ctx, rr, cc - 1, eTile_SmoothStoneWall, TILE_FLAG_OPAQUE);
	dig_cell(ctx, rr, cc + 0, eTile_StoneFloor, 0);
	dig_cell(ctx, rr, cc + 1, eTile_SmoothStoneWall, TILE_FLAG_OPAQUE);

	rr += 1;

	// now do a room
	h = 5 + xrnd(4);
	w = 5 + xrnd(4);

	digbox_at(ctx, rr, cc - w / 2, h, w, eTile_SmoothStoneWall, TILE_FLAG_OPAQUE);
	dig_cell(ctx, rr, cc, eTile_StoneFloor, 0);

	cc -= w / 2;

	rr += 1;
	cc += 1;
	digbox_at(ctx, rr, cc, h - 2, w - 2, eTile_StoneFloor, 0);

	h -= 2;
	w -= 2;

	// test stairs down
//	if(generational_matrix[ctx->locn.level].stairs_down == true)
//	{
//		dig_cell(ctx, rr + xrnd(h), cc + xrnd(w), eTile_StairsDown, 0);
//	}

	rr += 0 + h;
	cc += 1 + xrnd(w - 1);

	// stairs out at...
	h = 3 + xrnd(5);

	CreateMessageOnWall(ctx, rr, cc, msgs[xrnd(msg_count)]);

	for(w = 0; w < h; w++)
	{
		dig_cell(ctx, rr, cc, eTile_StoneFloor, 0);
		dig_cell(ctx, rr, cc - 1, eTile_SmoothStoneWall, TILE_FLAG_OPAQUE);
		dig_cell(ctx, rr, cc + 1, eTile_SmoothStoneWall, TILE_FLAG_OPAQUE);
		rr += 1;
	}

	// spawn one step away from the stairs
	ctx->locn.row = rr - 1;
	ctx->locn.col = cc;

	dig_cell(ctx, rr, cc, eTile_StairsUp, 0);
	dig_cell(ctx, rr, cc - 1, eTile_SmoothStoneWall, TILE_FLAG_OPAQUE);
	dig_cell(ctx, rr, cc + 1, eTile_SmoothStoneWall, TILE_FLAG_OPAQUE);

	rr += 1;
	dig_cell(ctx, rr, cc, eTile_SmoothStoneWall, TILE_FLAG_OPAQUE);
	dig_cell(ctx, rr, cc - 1, eTile_SmoothStoneWall, TILE_FLAG_OPAQUE);
	dig_cell(ctx, rr, cc + 1, eTile_SmoothStoneWall, TILE_FLAG_OPAQUE);

	if(rr > ctx->map->max_rows)
	{
		ctx->map->max_rows = rr;
	}
	if(cc > ctx->map->max_cols)
	{
		ctx->map->max_cols = cc;
	}

	quad_gen_monsters(ctx, 1);
	quad_gen_items(ctx, 1);
	quad_gen_traps(ctx, 1);
}

static void gen_map1_levelN(uContext *ctx, bool at_stairs)
{
	uint16_t h;
	uint16_t rr, cc;

	ctx->map->max_cols = generational_matrix[ctx->locn.level].max_col;
	ctx->map->max_rows = generational_matrix[ctx->locn.level].max_row;
	if(ctx->map->max_cols > MAX_MAP_WIDTH - 4)
	{
		ctx->map->max_cols = MAX_MAP_WIDTH - 4;
	}
	if(ctx->map->max_rows > MAX_MAP_HEIGHT - 4)
	{
		ctx->map->max_rows = MAX_MAP_HEIGHT - 4;
	}

	cellgrow(ctx);

	rr = UINT16_MAX;
	cc = UINT16_MAX;

	// randomise stairs down
	gen_stairs_down(ctx);
	gen_stairs_up(ctx, at_stairs);

	if(at_stairs == false)
	{
		h = 0;
		while(h == 0)
		{
			rr = xrnd(ctx->map->max_rows);
			cc = xrnd(ctx->map->max_cols);

			if(get_cell_tile(ctx, rr, cc) == eTile_StoneFloor)
			{
				h = 1;
			}
		}

		ctx->locn.row = rr;
		ctx->locn.col = cc;
	}


	if(rr != UINT16_MAX && rr > ctx->map->max_rows)
	{
		ctx->map->max_rows = rr;
	}

	if(cc != UINT16_MAX && cc > ctx->map->max_cols)
	{
		ctx->map->max_cols = cc;
	}

	quad_gen_monsters(ctx, ctx->locn.level);
	quad_gen_items(ctx, ctx->locn.level);
	quad_gen_traps(ctx, ctx->locn.level);

	if(ctx->locn.level == 10)
	{
		uMonster *m;
		while(1)
		{
			rr = 3 + xrnd(MAX_MAP_HEIGHT - 3);
			cc = 3 + xrnd(MAX_MAP_WIDTH - 3);
			if(get_cell_tile(ctx, rr, cc) == eTile_StoneFloor)
			{
				// verify no monster on same cell!
				if(GetMonsterOnMap(ctx, rr, cc) == NULL)
				{
					break;
				}
			}
		}

		m = GenMonster(ctx, 100);
		assert(m != NULL);
		BuildMonster(ctx, m, rr, cc);
	}
}


void GenerateMap(uContext *ctx, uint16_t map, uint16_t level, bool at_stairs)
{
	// generate entry point
	ctx->locn.map = map;
	ctx->locn.level = level;
	ctx->locn.row = 1;
	ctx->locn.col = 1;
	ctx->map->recovery = 10 + (level * 2);

	screen_clear(0x7);
	screen_line_clear(0, 0x70);
	msg(0, 0, 0x70, " Underdark - A roguelike by Bloody Cactus ");

	msg_hl(4, 2, HL_CLR, "Generating random level....");

	// mark level as generated
	bit_set_bit(ctx->game_flags, eFlag_GenLevel0 + level, true);

	ClearMap(ctx);

	// its always map1 right now
	switch(level)
	{
		case 1:
			gen_map1_level1(ctx, at_stairs);
			break;

		default:
			gen_map1_levelN(ctx, at_stairs);
			break;
	}

	calc_draw_offseet(ctx);
}


void ClearMap(uContext *ctx)
{
	assert(ctx->map != NULL);
	assert(ctx->map->lstTriggers != NULL);
	assert(ctx->map->lstMonsters != NULL);
	assert(ctx->map->lstItems != NULL);

	dlist_empty(ctx->map->lstTriggers);
	dlist_empty(ctx->map->lstMonsters);
	dlist_empty(ctx->map->lstItems);

	digbox_at(ctx, 0, 0, MAX_MAP_HEIGHT, MAX_MAP_WIDTH, eTile_Void, 0);

	ctx->map->max_rows = 0;
	ctx->map->max_cols = 0;
}


void FreeMap(uContext *ctx)
{
	assert(ctx->map != NULL);
	assert(ctx->map->lstTriggers != NULL);
	assert(ctx->map->lstMonsters != NULL);
	assert(ctx->map->lstItems != NULL);

	FreeDList(ctx->map->lstTriggers);
	FreeDList(ctx->map->lstMonsters);
	FreeDList(ctx->map->lstItems);

	memset(ctx->map, 0x0, sizeof(uMap));

	free(ctx->map);
}

static void DoItemDropByName(uContext *ctx, char *name, uint8_t drop_type, uint16_t r, uint16_t c, uint8_t count)
{
	uMapItem *i;
	uItem *item = NULL;

	i = (uMapItem *)calloc(1, sizeof(uMapItem));
	assert(i != NULL);
	i->row = r;
	i->col = c;
	i->type = drop_type;
	i->data.i_item.has_item_data = false;
	item = FindItemByName(ctx, name);

	if(item != NULL)
	{
		i->data.i_item.count = count;
		i->data.i_item.idx = item->idx;
		dlist_ins(ctx->map->lstItems, i);
	}
	else
	{
		free(i);
	}
}

void DeleteTrigger(uContext *ctx, uMapTrigger *trig)
{
	DLElement *e = dlist_head(ctx->map->lstTriggers);

	while(e != NULL && ctx->player->life > 0)
	{
		uMapTrigger *t = dlist_data(e);

		if(t == trig)
		{
			dlist_remove(ctx->map->lstTriggers, e, (void *)&t);
			FreeMapTrigger(t);
			return;
		}

		e = dlist_next(e);
	}
}

uMapTrigger *GetMapTrigger(uContext *ctx, uint16_t r, uint16_t c)
{
	DLElement *e = dlist_head(ctx->map->lstTriggers);

	while(e != NULL)
	{
		uMapTrigger *t = dlist_data(e);
		if(t->row == r && t->col == c)
		{
			return t;
		}

		e = dlist_next(e);
	}

	return NULL;
}

void ProcessMapTriggers(uContext *ctx, uint16_t r, uint16_t c)
{
	DLElement *e = dlist_head(ctx->map->lstTriggers);
	int16_t damage;
	uMapMonster *mm = NULL;

	while(e != NULL && ctx->player->life > 0)
	{
		uMapTrigger *t = dlist_data(e);
		e = dlist_next(e);

		if(t->row == r && t->col == c)
		{
			switch(t->type)
			{
				case eTrigger_SignOnWall:
					add_msg(ctx, t->data.t_sign_on_wall.msg);
					break;

				case eTrigger_Trap:
					damage = roll_dice(ctx->locn.level, 4, ctx->locn.level);
					switch(t->data.t_trap.type)
					{
						case eT_Spiderweb:
							// no damage, just get stuck like beartrap
							if(ctx->player->life > 0)
							{
								add_msg(ctx, "You are caught in a spider web");
								// get stuck for a bit
								AddCounter(ctx, eIA_i_HeldInPosition, roll_dice(2, 4, 2), 0);
								DeleteTrigger(ctx, t);
							}
							else
							{
								add_msg(ctx, "You stepped on a beartrap, for `%"PRIi16"` damage, bleeding out.", damage);
								DeathScreen(ctx, "bled out from a bear trap");
							}
							break;
						case eT_BearTrap:
							damage = add_player_damage(ctx, damage);
							if(ctx->player->life > 0)
							{
								add_msg(ctx, "You stepped on a beartrap for `%"PRIi16"` damage.", damage);
								// get stuck for a bit
								AddCounter(ctx, eIA_i_HeldInPosition, roll_dice(2, 4, 2), 0);
								DeleteTrigger(ctx, t);
							}
							else
							{
								add_msg(ctx, "You stepped on a beartrap, for `%"PRIi16"` damage, bleeding out.", damage);
								DeathScreen(ctx, "bled out from a bear trap");
							}
							break;
						case eT_DartTrap:
							damage = add_player_damage(ctx, damage);
							if(ctx->player->life > 0)
							{
								add_msg(ctx, "Arrows fly in from somewhere for `%"PRIi16"` damage.", damage);
								DoItemDropByName(ctx, ITEM_NAME_ARROWS, eDT_Weapon, r, c, 1 + xrnd(4));
								DeleteTrigger(ctx, t);
							}
							else
							{
								add_msg(ctx, "Arrows flew in from somewhere for `%"PRIi16"` damage, impaling you.", damage);
								DeathScreen(ctx, "was shot through by arrows");
							}
							break;
						case eT_SpikeTrap:
							damage = add_player_damage(ctx, damage);
							if(ctx->player->life > 0)
							{
								add_msg(ctx, "Spikes shoot up from below for `%"PRIi16"` damage.", damage);
								DeleteTrigger(ctx, t);
							}
							else
							{
								add_msg(ctx, "Spikes shoot up from below for `%"PRIi16"` damage, impaling you.", damage);
								DeathScreen(ctx, "was impaled on spikes");
							}
							break;
						case eT_SpoonTrap:
							damage = add_player_damage(ctx, damage);
							if(ctx->player->life > 0)
							{
								add_msg(ctx, "Rusty spoons shoot out from above for `%"PRIi16"` damage.", damage);
								DeleteTrigger(ctx, t);
							}
							else
							{
								add_msg(ctx, "Rusty Spoons shoot up from above for `%"PRIi16"` damage, slicing you.", damage);
								DeathScreen(ctx, "got tetanus from a rusty spoon");
							}
							break;

						case eT_Rockfall:
							damage = add_player_damage(ctx, damage);
							if(ctx->player->life > 0)
							{
								add_msg(ctx, "Rocks fall on you for `%"PRIi16"` damage.", damage);
								DeleteTrigger(ctx, t);
							}
							else
							{
								add_msg(ctx, "Rocks fall on you for `%"PRIi16"` damage, crushing you.", damage);
								DeathScreen(ctx, "was crushed by rockfall");
							}
							break;

						case eT_Teleport:
							ctx->locn.row = t->data.t_trap.dest_row;
							ctx->locn.col = t->data.t_trap.dest_col;
							clear_player_level_seen(ctx);
							calc_draw_offseet(ctx);
							add_msg(ctx, "You feel disoriented.");
							DeleteTrigger(ctx, t);

							// test if we teleport on top of monster!
							mm = GetMonsterOnMap(ctx, ctx->locn.row, ctx->locn.col);
							if(mm != NULL)
							{
								uMonster *m = GetMonsterByID(ctx, mm->monster_id);
								add_msg(ctx, "You hear a squelching splinching squishy sound...");
								AddExperiece(ctx, m->xp);
								KillMonster(ctx, mm);
							}
							break;

						case eT_TrapDoor:
							set_cell_tile(ctx, t->row, t->col, eTile_Trap);
							add_msg(ctx, "Arrrgh! You fell through a trapdoor into the level below");
							if(DescendLevel(ctx) == true)
							{
								ctx->player->life = 0;
							}
							break;
					}
					break;

				default:
					printf("unimplemented ProcessMapTriggers %u\n", t->type);
					break;
			}
		}
	}
}

uMapItem *GetMapItem(uContext *ctx, uint16_t r, uint16_t c)
{
	DLElement *e;

	e = dlist_head(ctx->map->lstItems);
	while(e != NULL)
	{
		uMapItem *i = dlist_data(e);
		e = dlist_next(e);

		if(i->row == r && i->col == c)
		{
			return i;
		}
	}

	return NULL;
}

void DeleteItem(uContext *ctx, uMapItem *item)
{
	DLElement *e = dlist_head(ctx->map->lstItems);

	while(e != NULL)
	{
		uMapItem *i = dlist_data(e);

		if(i == item)
		{
			dlist_remove(ctx->map->lstItems, e, (void **)&i);
			FreeMapItem(i);
			return;
		}

		e = dlist_next(e);
	}
}

/// returns false if OK!
bool DescendLevel(uContext *ctx)
{
	SaveMap(ctx);
	ctx->locn.level += 1;

	// load or generate previous level
	if(bit_get_bit(ctx->game_flags, eFlag_GenLevel0 + ctx->locn.level) == true)
	{
		if(LoadMap(ctx) == false)
		{
			DeathScreen(ctx, "Loading of map failed");
			return true;
		}
	}
	else
	{
		GenerateMap(ctx, ctx->locn.map, ctx->locn.level, true);
	}

	calc_draw_offseet(ctx);
	return false;
}
