#include "headers.h"

#define AMMO_PAUSE_TIME    1

static void draw_messages(uContext *ctx);

static void draw_status(uContext *ctx);

const char *ascii_roles[] = {
	"Adventurer",
	"Hunter",
	"Rogue",
	"Wizard",
	"Barbarian"
};

// maps to tiles enum
const uint8_t *ascii_tiles = (uint8_t *)" ##.+/+><^";
/*
void = 0,
eTile_RoughRockWall,
eTile_SmoothStoneWall,
eTile_StoneFloor,
eTile_ClosedDoor,
eTile_OpenDoor,
eTile_LockedClosedDoor,
eTile_StairsUp,
eTile_StairsDown,
eTile_Trap
*/

// NASTY!! should not be global

const uint8_t *ascii_colors = (uint8_t *)"\x7\x6\x7\x7\x6\x6\x6\xE\xE\xC";

const uint8_t *ascii_race_list[] = {
	(uint8_t *)"Human",
	(uint8_t *)"Elf",
	(uint8_t *)"Dwarf",
	(uint8_t *)"Orc",
	(uint8_t *)"Gnoll"
};


uint16_t min_light_row = MAX_MAP_HEIGHT;
uint16_t max_light_row = 0;
uint16_t min_light_col = MAX_MAP_WIDTH;
uint16_t max_light_col = 0;

void reset_los_square(void)
{
	min_light_row = MAX_MAP_HEIGHT;
	min_light_col = MAX_MAP_WIDTH;
	max_light_row = 0;
	max_light_col = 0;
}

static void clear_los(uContext *c)
{
	uint16_t row, col;

	if(min_light_row != MAX_MAP_HEIGHT && min_light_col != MAX_MAP_WIDTH)
	{
		for(row = min_light_row; row <= max_light_row && row < MAX_MAP_HEIGHT; row++)
		{
			for(col = min_light_col; col <= max_light_col && col < MAX_MAP_WIDTH; col++)
			{
				c->map->cells[(MAX_MAP_WIDTH * (c->draw_top_row + row)) + (c->draw_top_col + col)].tile.flags &= 0x7F;
			}
		}
	}

	c->player->monster_fov_idx = 0;
	reset_los_square();
}

int16_t add_player_damage(uContext *ctx, int16_t d)
{
	int16_t u = d;

	if(ctx->player->life > u)
	{
		ctx->player->life -= u;
	}
	else
	{
		u -= ctx->player->life;
		ctx->player->life = 0;
	}

	return u;
}

static void AddToAOEBuff(uContext *ctx, DList *lst, uint16_t r, uint16_t c)
{
	uMapMonster *m;
	DLElement *e;

	if(lst == NULL)
	{
		return;
	}

	m = GetMonsterOnMap(ctx, r, c);
	if(m == NULL)
	{
		if(r == ctx->locn.row && c == ctx->locn.col)
		{
			e = dlist_head(lst);
			while(e != NULL)
			{
				if(dlist_data(e) == NULL)
				{
					return;
				}

				e = dlist_next(e);
			}

			// add null, this is me!
			dlist_ins(lst, NULL);
		}

		return;
	}

	e = dlist_head(lst);
	while(e != NULL)
	{
		if(dlist_data(e) == m)
		{
			return;
		}
		e = dlist_next(e);
	}

	dlist_ins(lst, m);
}

void draw_line(uContext *ctx, int16_t row1, int16_t col1, int16_t row2, int16_t col2, uint8_t colour, char ascii, bool pause, DList *lst)
{
	int16_t col, row;
	int16_t deltaCol, deltaRow;
	int16_t stepCol, stepRow;
	int16_t errorCol, errorRow;
	int16_t detdelta;
	int16_t count;

	// stop self targetting breaking with massive vertical line....
	if(row1 == row2 && col1 == col2)
	{
		return;
	}

	// convert row/cols into drawing offsets

//	row1 -= ctx->draw_top_row;
//	row2 -= ctx->draw_top_row;
//	col1 -= ctx->draw_top_col;
//	col2 -= ctx->draw_top_col;

	if(row1 < 0)
	{ row1 = 0; }
	if(col1 < 0)
	{ col1 = 0; }
	if(row2 < 0)
	{ row2 = 0; }
	if(col2 < 0)
	{ col2 = 0; }

	/* Vertical line */
	if(col1 == col2)
	{
		// bugfix
		if(row1 == row2)
		{
			return;
		}

		if(row1 > row2)
		{
			row = -1;
		}
		else
		{
			row = 1;
		}

		// get off my own space
		row1 += row;
		for(; row1 != row2; row1 += row)
		{
			if((get_cell_flags(ctx, row1, col1) & TILE_FLAG_OPAQUE) != TILE_FLAG_OPAQUE)
			{
				if(row1 - ctx->draw_top_row >= 0 && col1 - ctx->draw_top_col >= 0 && row1 - ctx->draw_top_row < ctx->status_row && col1 - ctx->draw_top_col < screen_get_width())
				{
					toggle_swap_draw_to_backbuffer();
					screen_outch(row1 - ctx->draw_top_row, col1 - ctx->draw_top_col, colour, ascii);
					toggle_swap_draw_to_backbuffer();
					restore_from_backbuffer();

					if(pause == true)
					{
						wait_ticks(AMMO_PAUSE_TIME);
					}
				}

				AddToAOEBuff(ctx, lst, row1, col1);
			}
			else
			{
				return;
			}
		}

		if((get_cell_flags(ctx, row1, col1) & TILE_FLAG_OPAQUE) != TILE_FLAG_OPAQUE)
		{
			if(row1 - ctx->draw_top_row >= 0 && col1 - ctx->draw_top_col >= 0 && row1 - ctx->draw_top_row < ctx->status_row && col1 - ctx->draw_top_col < screen_get_width())
			{
				toggle_swap_draw_to_backbuffer();
				screen_outch(row1 - ctx->draw_top_row, col1 - ctx->draw_top_col, colour, ascii);
				toggle_swap_draw_to_backbuffer();
				restore_from_backbuffer();

				if(pause == true)
				{
					wait_ticks(AMMO_PAUSE_TIME);
				}

				AddToAOEBuff(ctx, lst, row1, col1);
			}
		}
		return;
	}

	/* Horizontal line */
	if(row1 == row2)
	{
		if(col1 > col2)
		{
			col = -1;
		}
		else
		{
			col = 1;
		}

		// get off my own space
		col1 += col;

		for(; col1 != col2; col1 += col)
		{
			if(col1 < 0 || col1 >= screen_get_width())
			{
				return;
			}

			if((get_cell_flags(ctx, row1, col1) & TILE_FLAG_OPAQUE) != TILE_FLAG_OPAQUE)
			{
				if(row1 - ctx->draw_top_row >= 0 && col1 - ctx->draw_top_col >= 0 && row1 - ctx->draw_top_row < ctx->status_row && col1 - ctx->draw_top_col < screen_get_width())
				{
					toggle_swap_draw_to_backbuffer();
					screen_outch(row1 - ctx->draw_top_row, col1 - ctx->draw_top_col, colour, ascii);
					toggle_swap_draw_to_backbuffer();
					restore_from_backbuffer();
					if(pause == true)
					{
						wait_ticks(AMMO_PAUSE_TIME);
					}
				}

				AddToAOEBuff(ctx, lst, row1, col1);
			}
			else
			{
				return;
			}
		}

		if((get_cell_flags(ctx, row1, col1) & TILE_FLAG_OPAQUE) != TILE_FLAG_OPAQUE)
		{
			if(row1 - ctx->draw_top_row >= 0 && col1 - ctx->draw_top_col >= 0 && row1 - ctx->draw_top_row < ctx->status_row && col1 - ctx->draw_top_col < screen_get_width())
			{
				toggle_swap_draw_to_backbuffer();
				screen_outch(row1 - ctx->draw_top_row, col1 - ctx->draw_top_col, colour, ascii);
				toggle_swap_draw_to_backbuffer();
				restore_from_backbuffer();
				if(pause == true)
				{
					wait_ticks(AMMO_PAUSE_TIME);
				}
			}

			AddToAOEBuff(ctx, lst, row1, col1);
		}

		return;
	}

	row = row1;
	col = col1;

	// dont check my own space
	//if((get_cell_flags(ctx, row1, col1) & TILE_FLAG_OPAQUE) == TILE_FLAG_OPAQUE)
	//	return false;
	//if(GetMonsterOnMap(ctx, row1, col1) != NULL)
	//	return false;

	stepRow = 1;
	deltaRow = row2 - row1;

	if(deltaRow < 0)
	{
		stepRow = -1;
		deltaRow = -deltaRow;
	}

	stepCol = 1;
	deltaCol = col2 - col1;

	if(deltaCol < 0)
	{
		stepCol = -1;
		deltaCol = -deltaCol;
	}

	if(deltaRow > deltaCol)
	{
		count = deltaRow;
		detdelta = deltaRow;
		errorCol = deltaRow >> 1;
		errorRow = 0;
	}
	else
	{
		count = deltaCol;
		detdelta = deltaCol;
		errorCol = 0;
		errorRow = deltaCol >> 1;
	}

	do
	{
		errorRow = (errorRow + deltaRow);

		if(errorRow >= detdelta)
		{
			errorRow -= detdelta;
			row += stepRow;
		}

		errorCol = (errorCol + deltaCol);

		if(errorCol >= detdelta)
		{
			errorCol -= detdelta;
			col += stepCol;
		}

//		if(row < 0 || row >= ctx->status_row || col < 0 || col >= screen_get_width())
//		{
//			return;
//		}

		if((get_cell_flags(ctx, row, col) & TILE_FLAG_OPAQUE) != TILE_FLAG_OPAQUE)
		{
			if(row - ctx->draw_top_row >= 0 && col - ctx->draw_top_col >= 0 && row - ctx->draw_top_row < ctx->status_row && col - ctx->draw_top_col < screen_get_width())
			{
				toggle_swap_draw_to_backbuffer();
				screen_outch(row - ctx->draw_top_row, col - ctx->draw_top_col, colour, ascii);
				toggle_swap_draw_to_backbuffer();
				restore_from_backbuffer();
				if(pause == true)
				{
					wait_ticks(AMMO_PAUSE_TIME);
				}
			}

			AddToAOEBuff(ctx, lst, row, col);
		}
		else
		{
			return;
		}

		count--;
	} while(count > 0);

	if((get_cell_flags(ctx, row, col) & TILE_FLAG_OPAQUE) != TILE_FLAG_OPAQUE)
	{
		if(row - ctx->draw_top_row >= 0 && col - ctx->draw_top_col >= 0 && row - ctx->draw_top_row < ctx->status_row && col - ctx->draw_top_col < screen_get_width())
		{
			toggle_swap_draw_to_backbuffer();
			screen_outch(row - ctx->draw_top_row, col - ctx->draw_top_col, colour, ascii);
			toggle_swap_draw_to_backbuffer();
			restore_from_backbuffer();

			if(pause == true)
			{
				wait_ticks(AMMO_PAUSE_TIME);
			}
		}

		AddToAOEBuff(ctx, lst, row, col);
	}
}

int16_t roll_dice(int16_t num_dice, int16_t num_faces, int16_t plus)
{
	int16_t c = 0;
	uint16_t i;

	for(i = 0; i < num_dice; i++)
	{
		c += 1 + xrnd(num_faces);
	}

	c += plus;

	return c;
}

/*
https://anydice.com/

output 5d10
output 5d10 + 2
output 4d10 + 1d8 + 5
output 4d10 + 1d8 + 7
output 4d10 + 1d6 + 10
output 4d10 + 1d6 + 12
output 4d10 + 1d4 + 15
output 4d10 + 1d4 + 17
output 4d10 + 1d2 + 20
output 4d10 + 1d2 + 22
*/

int16_t skill_roll(int16_t level)
{
	// max level 10!

	if(level < 0)
	{
		return 0 - skill_roll(abs(level));
	}

	switch(level < 10 ? level : 10)
	{
		default :
			return 0;

		case 1:
			return roll_dice(5, 10, 0);

		case 2:
			return roll_dice(5, 10, 2);

		case 3:
			return roll_dice(4, 10, 0) + roll_dice(1, 8, 0) + 5;

		case 4:
			return roll_dice(4, 10, 0) + roll_dice(1, 8, 0) + 7;

		case 5:
			return roll_dice(4, 10, 0) + roll_dice(1, 6, 0) + 10;

		case 6:
			return roll_dice(4, 10, 0) + roll_dice(1, 6, 0) + 12;

		case 7:
			return roll_dice(4, 10, 0) + roll_dice(1, 4, 0) + 15;

		case 8:
			return roll_dice(4, 10, 0) + roll_dice(1, 4, 0) + 17;

		case 9:
			return roll_dice(4, 10, 0) + roll_dice(1, 2, 0) + 20;

		case 10:
			return roll_dice(4, 10, 0) + roll_dice(1, 2, 0) + 22;
	}
}

uint16_t GetDirectionKey(uContext *ctx, char *msg)
{
	uint16_t key;

	add_msg(ctx, "%s", msg);
	draw_messages(ctx);

	while(1)
	{
		key = DOSKeyToInternal(ctx, gkey(NULL));

		if(key == eKey_Up)
		{
			return key;
		}

		if(key == eKey_Down)
		{
			return key;
		}

		if(key == eKey_Left)
		{
			return key;
		}

		if(key == eKey_Right)
		{
			return key;
		}

		if(key == eKey_Cancel)
		{
			return key;
		}

		if(key == eKey_UpLeft)
		{
			return key;
		}

		if(key == eKey_UpRight)
		{
			return key;
		}

		if(key == eKey_DownLeft)
		{
			return key;
		}

		if(key == eKey_DownRight)
		{
			return key;
		}
	}
}

bool DoDoorOpenClose(uContext *ctx, uint16_t r, uint16_t c, bool open)
{
	uint16_t key;

	if(open == true)
	{
		key = GetDirectionKey(ctx, "Open in which direction? ");
	}
	else
	{
		key = GetDirectionKey(ctx, "Close in which direction? ");
	}

	update_last_message(ctx, DirectionalAscii(key));

	draw_messages(ctx);

	switch(key)
	{

		case eKey_Up:
			r -= 1;
			break;

		case eKey_Down:
			r += 1;
			break;

		case eKey_Left:
			c -= 1;
			break;

		case eKey_Right:
			c += 1;
			break;

		case eKey_UpLeft:
			r -= 1;
			c -= 1;
			break;

		case eKey_UpRight:
			r -= 1;
			c += 1;
			break;

		case eKey_DownLeft:
			r += 1;
			c -= 1;
			break;

		case eKey_DownRight:
			r += 1;
			c += 1;
			break;
	}

	if(key != eKey_Cancel)
	{
		// test for door

		if(open == true)
		{
			if(get_cell_tile(ctx, r, c) == eTile_ClosedDoor)
			{
				dig_cell(ctx, r, c, eTile_OpenDoor, 0);
				add_msg(ctx, "You open the door");
				ctx->redraw_messages = true;
				return true;
			}
			else if(get_cell_tile(ctx, r, c) == eTile_LockedClosedDoor)
			{
				add_msg(ctx, "You rattle the door, but its locked tight");
				ctx->redraw_messages = true;
				return false;
			}
		}
		else
		{
			if(get_cell_tile(ctx, r, c) == eTile_OpenDoor)
			{
				dig_cell(ctx, r, c, eTile_ClosedDoor, TILE_FLAG_OPAQUE);
				add_msg(ctx, "You close the door");
				ctx->redraw_messages = true;
				return true;
			}
		}
	}

	return false;
}

bool IsStandingOnTile(uContext *ctx, uint16_t r, uint16_t c, uint16_t tile)
{
	if(r < MAX_MAP_HEIGHT && c < MAX_MAP_WIDTH)
	{
		if(ctx->map->cells[(MAX_MAP_WIDTH * r) + c].tile.tile == tile)
		{
			return true;
		}
	}

	return false;
}

bool MonsterBumpFightsPlayer(uContext *ctx, uMapMonster *mm)
{
	char *ascii_attack[] = {
		"bites",    //eMAT_Bite,
		"hits",    //eMAT_Weapon
		"hits",    //eMAT_RangedWeapon
		"bites",    //eMAT_PoisonBite1
		"bites",    //eMAT_PoisonBite2
		"drains",    //eMAT_LifeDrain1
		"drains",    //eMAT_LifeDrain2
		"hits",    //eMAT_MagicUser
		"hits",    //eMAT_Acid1
		"hits",        //eMAT_Acid2
		"hits",    //eMAT_StealMoney1
		"hits",    //eMAT_StealMoney2
		"claws",    //eMAT_Claw
		"spits",    //eMAT_Spit
		"gazes",    //eMAT_Gaze
		"stings"    //eMAT_Sting
	};

	uMonster *m;
	bool player_is_dead = false;
	int16_t dex;
	int16_t dmg;

	m = GetMonsterByID(ctx, mm->monster_id);

	if(m == NULL)
	{
		return false;
	}

	dmg = roll_dice(m->attack_dice[0], m->attack_dice[1], m->attack_dice[2]);

	// melee dodge, use dexterity - 2
	dex = ctx->player->stats[eStat_Dexterity];

	//dex -= 2;
	if(dex < 1)
	{
		dex = 1;
	}

	if(skill_roll(m->attack_level) > skill_roll(dex))
	{
		// gaze ignores armour!
		if(m->attack_type != eMAT_Gaze)
		{
			dmg -= ctx->player->armour;
		}

		if(dmg <= 0)
		{
			add_msg(ctx, "The %s %s at you and misses", m->name, ascii_attack[m->attack_type], dmg);
		}
		else if(dmg >= ctx->player->life)
		{
			char death_buffer[80];

			ctx->player->life = 0;

			add_msg(ctx, "The %s %s for `%"PRIi16"`. You die!", m->name, ascii_attack[m->attack_type], dmg);
			sprintf(death_buffer, "Was killed by a %s", m->name);
			DeathScreen(ctx, death_buffer);
			player_is_dead = true;
		}
		else
		{
			// we took a hit.
			if((m->attack_type == eMAT_PoisonBite1 && chance(10)) || (m->attack_type == eMAT_PoisonBite2 && chance(20)))
			{
				// poison severity
				if(ctx->locn.level < 4)
				{
					AddCounter(ctx, eIA_i_Poison1for5__food_only, 50 + xrnd(50), 0);
				}
				else if(ctx->locn.level < 7)
				{
					AddCounter(ctx, eIA_i_Poison1for3__food_only, 50 + xrnd(75), 0);
				}
				else
				{
					AddCounter(ctx, eIA_i_Poison1for1__food_only, 50 + xrnd(100), 0);
				}
			}
			else if((m->attack_type == eMAT_LifeDrain1 && chance(10)) || (m->attack_type == eMAT_LifeDrain2 && chance(20)))
			{
				add_msg(ctx, "You feel weaker");

				ctx->player->max_life -= ctx->player->max_life / 10;
				if(ctx->player->life > ctx->player->max_life)
				{
					ctx->player->life = ctx->player->max_life;
				}
			}
			else if((m->attack_type == eMAT_Acid1 && chance(10)) || (m->attack_type == eMAT_Acid2 && chance(20)))
			{
				//disolve something
				uItem *item = NULL;

				switch(xrnd(3))
				{
					case 0:
						if(ctx->player->item_head != EMPTY_ITEM_SLOT)
						{
							item = &ctx->player->items[ctx->player->item_head].item;
							UnEquipItem(ctx, item, true, true);
							DeleteInventoryItem(ctx, item);
							add_msg(ctx, "`Your armour disolves!`");
						}
						break;
					case 1:
						if(ctx->player->item_body != EMPTY_ITEM_SLOT)
						{
							item = &ctx->player->items[ctx->player->item_body].item;
							UnEquipItem(ctx, item, true, true);
							DeleteInventoryItem(ctx, item);
							add_msg(ctx, "`Your armour disolves!`");
						}
						break;
					case 2:
						if(ctx->player->item_left_weap != EMPTY_ITEM_SLOT)
						{
							item = &ctx->player->items[ctx->player->item_left_weap].item;
							UnEquipItem(ctx, item, true, true);
							DeleteInventoryItem(ctx, item);
							add_msg(ctx, "`Your weapon disolves!`");
						}
						else if(ctx->player->item_right_weap != EMPTY_ITEM_SLOT)
						{
							item = &ctx->player->items[ctx->player->item_right_weap].item;
							UnEquipItem(ctx, item, true, true);
							DeleteInventoryItem(ctx, item);
							add_msg(ctx, "`Your weapon disolves!`");
						}
						break;
				}
			}
			else if((m->attack_type == eMAT_StealMoney1 && chance(10)) || (m->attack_type == eMAT_StealMoney2 && chance(20)))
			{
				if(ctx->player->money > 0)
				{
					add_msg(ctx, "You feel lighter");
					if(ctx->player->money > 10)
					{
						ctx->player->money -= ctx->player->money / 10;
					}
					else
					{
						ctx->player->money = 0;
					}
				}
			}
			else if(m->attack_type == eMAT_Bite || m->attack_type == eMAT_Claw || m->attack_type == eMAT_Sting || m->attack_type == eMAT_Spit)
			{
				// no different implementations here
			}
			else if(m->attack_type == eMAT_Gaze)
			{
				// nothing to do for gaze, we did it above
			}

			ctx->player->life -= dmg;
			add_msg(ctx, "The %s %s for `%u` damage", m->name, ascii_attack[m->attack_type], dmg);
		}
	}
	else
	{
		add_msg(ctx, "The %s %s and misses", m->name, ascii_attack[m->attack_type]);
	}

	if(player_is_dead == false)
	{
		draw_status(ctx);
		draw_messages(ctx);
	}

	return player_is_dead;
}

bool DoBumpCombat(uContext *ctx, uint16_t r, uint16_t c)
{
	uMapMonster *mm;
	uMonster *m;

	int16_t vs_monster;
	int16_t our_attack;

	uItem *ourWeap1;
	uItem *ourWeap2;

	bool monster_is_dead = true;

	mm = GetMonsterOnMap(ctx, r, c);

	if(mm == NULL)
	{
		return monster_is_dead;
	}

	monster_is_dead = false;

	m = GetMonsterByID(ctx, mm->monster_id);

	ourWeap1 = GetPlayerItemWeaponLeft(ctx);
	ourWeap2 = GetPlayerItemWeaponRight(ctx);

	// we attack, they defend.

	// get VS Monster bonus
	vs_monster = GetItemAttributeValue_KV(ourWeap1, eIA_kv_VsMonsterType, m->type, 0);
	vs_monster += GetItemAttributeValue_KV(ourWeap2, eIA_kv_VsMonsterType, m->type, 0);

	// get Attack dice value
	our_attack = GetItemAttributeValue_Die(ourWeap1, eIA_d_DoDamage, 0);
	our_attack += GetItemAttributeValue_Die(ourWeap2, eIA_d_DoDamage, 0);

	// adjust for VS monster bonuses
	our_attack += vs_monster;

	if(skill_roll(m->defend_level) < skill_roll(ctx->player->skills[eSkill_MeleeWeapons]))
	{

		if(GetItemAttributeValue_Bool(ourWeap1, eIA_b_Unknown, false) == true)
		{
			// its unknown!
			if(skill_roll(ctx->player->skills[eSkill_ItemLore]) >= skill_roll(GetItemAttributeValue_Int(ourWeap1, eIA_i_IdentifySkillLevel, 1)))
			{
				AddExperiece(ctx, 100);
				SetItemAttributeValue_Bool(ourWeap1, eIA_b_Unknown, false);
			}
		}

		if(GetItemAttributeValue_Bool(ourWeap2, eIA_b_Unknown, false) == true)
		{
			// its unknown!
			if(skill_roll(ctx->player->skills[eSkill_ItemLore]) >= skill_roll(GetItemAttributeValue_Int(ourWeap2, eIA_i_IdentifySkillLevel, 1)))
			{
				AddExperiece(ctx, 100);
				SetItemAttributeValue_Bool(ourWeap2, eIA_b_Unknown, false);
			}
		}


		our_attack -= mm->armour;

		if(our_attack <= 0)
		{
			add_msg(ctx, "You hit the %s and do no damage", m->name);
		}
		else if(our_attack >= mm->health)
		{
			mm->health = 0;
			// we hit and kill
			add_msg(ctx, "You hit the %s for %u points, it dies.", m->name, our_attack);

			ctx->player->score += m->xp / 2;
			AddExperiece(ctx, m->xp);

			KillMonster(ctx, mm);
			monster_is_dead = true;
			add_monster(ctx);
		}
		else
		{
			mm->health -= our_attack;
			add_msg(ctx, "You hit the %s for %u points.", m->name, our_attack);
		}
	}
	else
	{
		// monster defends!
		add_msg(ctx, "You miss the %s.", m->name);
	}

	// true on killing monster and can move into square
	return monster_is_dead;
}

// col=x, row=y within draw window
bool line_los(uContext *ctx, int16_t col1, int16_t row1, int16_t col2, int16_t row2)
{
	uint16_t col, row;
	int16_t deltaCol, deltaRow;
	int16_t stepCol, stepRow;
	int16_t errorCol, errorRow;
	int16_t detdelta;
	int16_t count;

	/* Vertical line */
	if(row1 < 0)
	{ row1 = 0; }
	if(col1 < 0)
	{ col1 = 0; }
	if(row2 < 0)
	{ row2 = 0; }
	if(col2 < 0)
	{ col2 = 0; }

	/* Vertical line */

	if(col1 == col2)
	{
		// stop bug when x.y == x.y!
		if(row1 == row2)
		{
			return true;
		}

		if(row1 > row2)
		{
			row = -1;
		}
		else
		{
			row = 1;
		}

		// get off my own space
		row1 += row;

		for(; row1 != row2; row1 += row)
		{
			if((get_cell_flags(ctx, row1, col1) & TILE_FLAG_OPAQUE) == TILE_FLAG_OPAQUE)
			{
				return false;
			}

			if(GetMonsterOnMap(ctx, row1, col1) != NULL)
			{
				return false;
			}
		}

		return true;
	}

	/* Horizontal line */
	if(row1 == row2)
	{
		if(col1 > col2)
		{
			col = -1;
		}
		else
		{
			col = 1;
		}

		// get off my own space
		col1 += col;

		for(; col1 != col2; col1 += col)
		{
			if((get_cell_flags(ctx, row1, col1) & TILE_FLAG_OPAQUE) == TILE_FLAG_OPAQUE)
			{
				return false;
			}

			if(GetMonsterOnMap(ctx, row1, col1) != NULL)
			{
				return false;
			}
		}

		return true;
	}

	row = row1;
	col = col1;

	// dont check my own space
	//if((get_cell_flags(ctx, row1, col1) & TILE_FLAG_OPAQUE) == TILE_FLAG_OPAQUE)
	//	return false;
	//if(GetMonsterOnMap(ctx, row1, col1) != NULL)
	//	return false;

	stepRow = 1;
	deltaRow = row2 - row1;

	if(deltaRow < 0)
	{
		stepRow = -1;
		deltaRow = -deltaRow;
	}

	stepCol = 1;
	deltaCol = col2 - col1;

	if(deltaCol < 0)
	{
		stepCol = -1;
		deltaCol = -deltaCol;
	}

	if(deltaRow > deltaCol)
	{
		count = deltaRow;
		detdelta = deltaRow;
		errorCol = deltaRow >> 1;
		errorRow = 0;
	}
	else
	{
		count = deltaCol;
		detdelta = deltaCol;
		errorCol = 0;
		errorRow = deltaCol >> 1;
	}

	do
	{
		errorRow = (errorRow + deltaRow);

		if(errorRow >= detdelta)
		{
			errorRow -= detdelta;
			row += stepRow;
		}

		errorCol = (errorCol + deltaCol);

		if(errorCol >= detdelta)
		{
			errorCol -= detdelta;
			col += stepCol;
		}

		if((get_cell_flags(ctx, row, col) & TILE_FLAG_OPAQUE) == TILE_FLAG_OPAQUE)
		{
			return false;
		}

		if(GetMonsterOnMap(ctx, row, col) != NULL)
		{
			return false;
		}

		count--;
	} while(count > 0);

	return true;
}

static bool IfMonsterCanThenDo(uContext *ctx, uMapMonster *mm, uint16_t r, uint16_t c)
{
	if((get_cell_flags(ctx, r, c) & TILE_FLAG_OPAQUE) != TILE_FLAG_OPAQUE)
	{
		if(GetMonsterOnMap(ctx, r, c) != NULL)
		{
			return false;
		}

		mm->row = r;
		mm->col = c;

		return true;
	}

	return false;
}

void MonstersTurn(uContext *ctx)
{
	uMapMonster *mm;
	DLElement *e;

	int16_t rr;
	int16_t cc;
	bool death;
	int16_t damage;

	uint8_t gr, gc;

	e = dlist_head(ctx->map->lstMonsters);
	death = false;

	while(e != NULL && death == false)
	{
		mm = dlist_data(e);
		e = dlist_next(e);

		if(mm->disposition == eMD_DocileChasing || mm->disposition == eMD_HostileChasing || (mm->disposition == eMD_HostileShambling && (ctx->moves % 3 == 0)))
		{
			bool ranged_attack = false;

			rr = mm->row;
			cc = mm->col;
			gr = UINT8_MAX;
			gc = UINT8_MAX;

			// move toward player, if player is in LOS
			if(line_los(ctx, mm->col, mm->row, ctx->locn.col, ctx->locn.row) == true)
			{
				uMonster *mon;

				mm->last_player_row = ctx->locn.row;
				mm->last_player_col = ctx->locn.col;
				gr = ctx->locn.row;
				gc = ctx->locn.col;

				// monster has LOS on player!
				mon = GetMonsterByID(ctx, mm->monster_id);
				damage = roll_dice(mon->attack_dice[0], mon->attack_dice[1], mon->attack_dice[2]);

				// NOTE: Trolls in combat can heal.. if they are in combat they are chasing.
				if(mon != NULL && mon->type == eMT_Troll)
				{
					// health boost per 10 rounds
					if(mm->health < mon->life && (ctx->moves % 10 == 0))
					{
						mm->health += 1 + xrnd(5);
					}

					if(mm->health > mon->life)
					{
						mm->health = mon->life;
					}
				}

				// ranged attack from monster?
				if(mon != NULL && (mon->attack_type == eMAT_MagicUser || mon->attack_type == eMAT_RangedWeapon))
				{
					ranged_attack = true;

					switch(mm->disposition)
					{
						// not if docile
						case eMD_DocileChasing:
						case eMD_DocileWatching:
						case eMD_DocileCantMove:
							break;

						default:
						{
							DList *lst;
							// ranged spell.
							bool aoe = false;

							lst = NewDList(NULL);
							if(mon->attack_type == eMAT_MagicUser)
							{
								if(MIN(abs(mm->row - ctx->locn.row), abs(mm->col - ctx->locn.col)) > 3 && chance(20))
								{
									aoe = true;
								}

								if(aoe == true)
								{
									int i;
									int j;
									int r = 2;

									draw_line(ctx, mm->row, mm->col, ctx->locn.row, ctx->locn.col, 0x0C, '*', true, lst);
									for(i = r; i >= 0; i--)
									{
										for(j = r; j >= 0; j--)
										{
											if((MAX(j, i)) + ((MIN(j, i)) / 2) <= r)
											{
												draw_line(ctx, mm->row, mm->col, ctx->locn.row + abs(j), ctx->locn.col + abs(i), 0x0C, '*', false, lst);
												draw_line(ctx, mm->row, mm->col, ctx->locn.row + abs(j), ctx->locn.col - abs(i), 0x0C, '*', false, lst);
												draw_line(ctx, mm->row, mm->col, ctx->locn.row - abs(j), ctx->locn.col + abs(i), 0x0C, '*', false, lst);
												draw_line(ctx, mm->row, mm->col, ctx->locn.row - abs(j), ctx->locn.col - abs(i), 0x0C, '*', false, lst);
											}
										}
									}
								}
								else
								{
									// not aoe
									draw_line(ctx, mm->row, mm->col, ctx->locn.row, ctx->locn.col, 0xC, '*', true, lst);
								}
							}
							else
							{
								// ranged weapon
								draw_line(ctx, mm->row, mm->col, ctx->locn.row, ctx->locn.col, 0x6, '*', true, lst);
							}
							wait_ticks(AMMO_PAUSE_TIME);

							e = dlist_head(lst);
							while(e != NULL && ctx->player->life > 0)
							{
								uMapMonster *mx = dlist_data(e);
								e = dlist_next(e);

								if(mx == NULL)
								{
									char buffer[128];

									if(aoe == false)
									{
										if(skill_roll(ctx->player->stats[eStat_Speed]) < skill_roll(mon->attack_level))
										{
											// me
											add_player_damage(ctx, damage);
											if(ctx->player->life > 0)
											{
												//
												add_msg(ctx, "The %s hits you for `%u`.", mon->name, damage);
											}
											else
											{
												add_msg(ctx, "The %s hits for `%u`. You die!", mon->name, damage);
												sprintf(buffer, "Was killed by a %s", mon->name);
												DeathScreen(ctx, buffer);
											}
										}
										else
										{
											// miss!
										}
									}
									else
									{
										// its aoe, no dodge!

										// me
										add_player_damage(ctx, damage);
										if(ctx->player->life > 0)
										{
											//
											add_msg(ctx, "The %s hits you for `%u`.", mon->name, damage);
										}
										else
										{
											add_msg(ctx, "The %s hits for `%u`. You die!", mon->name, damage);
											sprintf(buffer, "Was killed by a %s", mon->name);
											DeathScreen(ctx, buffer);
										}
									}
								}
								else
								{
									// other monster!
									if(mx->health <= damage)
									{
										mx->health = 0;
									}
									else
									{
										mx->health -= damage;
									}

									KillMonster(ctx, mx);
									add_monster(ctx);
								}
							}

							FreeDList(lst);
						}
							break;
					}
				}
			}
			else
			{
				// otherwise move toward where we last saw player
				gr = mm->last_player_row;
				gc = mm->last_player_col;
			}

			// make sure its a valid location to move toward.
			if(ctx->player->life > 0 && ranged_attack == false && (gr != UINT8_MAX && gc != UINT8_MAX))
			{
				if(rr > gr)
				{
					rr -= 1;
				}
				else if(rr < gr)
				{
					rr += 1;
				}

				if(cc > gc)
				{
					cc -= 1;
				}
				else if(cc < gc)
				{
					cc += 1;
				}

				if(mm->disposition == eMD_DocileChasing)
				{
					// keep this line for non-aggression
				}
				else if(cc == ctx->locn.col && rr == ctx->locn.row)
				{
					// bump combat!
					death = MonsterBumpFightsPlayer(ctx, mm);
				}
				else
				{
					rr = mm->row;
					cc = mm->col;

					// 3
					if(rr > gr && cc > gc)
					{
						if(IfMonsterCanThenDo(ctx, mm, rr - 1, cc - 1) == false)
						{
							if(IfMonsterCanThenDo(ctx, mm, rr, cc - 1) == false)
							{
								if(IfMonsterCanThenDo(ctx, mm, rr - 1, cc) == false)
								{
									// ?? do nothing...
								}
							}
						}
					}

						// 2
					else if(rr > gr && cc == gc)
					{
						if(IfMonsterCanThenDo(ctx, mm, rr - 1, cc) == false)
						{
							// ?? do nothing...
						}
					}

						// 1
					else if(rr > gr && cc < gc)
					{
						if(IfMonsterCanThenDo(ctx, mm, rr - 1, cc + 1) == false)
						{
							if(IfMonsterCanThenDo(ctx, mm, rr, cc + 1) == false)
							{
								if(IfMonsterCanThenDo(ctx, mm, rr - 1, cc) == false)
								{
									// ?? do nothing...
								}
							}
						}
					}

						// 7
					else if(rr < gr && cc < gc)
					{
						if(IfMonsterCanThenDo(ctx, mm, rr + 1, cc + 1) == false)
						{
							if(IfMonsterCanThenDo(ctx, mm, rr, cc + 1) == false)
							{
								if(IfMonsterCanThenDo(ctx, mm, rr + 1, cc) == false)
								{
									// ?? do nothing...
								}
							}
						}
					}

						// 8
					else if(rr < gr && cc == gc)
					{
						if(IfMonsterCanThenDo(ctx, mm, rr + 1, cc) == false)
						{
							// do nothing
						}
					}

						// 9
					else if(rr < gr && cc > gc)
					{
						if(IfMonsterCanThenDo(ctx, mm, rr + 1, cc - 1) == false)
						{
							if(IfMonsterCanThenDo(ctx, mm, rr, cc - 1) == false)
							{
								if(IfMonsterCanThenDo(ctx, mm, rr + 1, cc) == false)
								{
									// ?? do nothing...
								}
							}
						}
					}

						// 4
					else if(rr == gr && cc < gc)
					{
						if(IfMonsterCanThenDo(ctx, mm, rr, cc + 1) == false)
						{
							// ?? do nothing...
						}
					}

						// 6
					else if(rr == gr && cc > gc)
					{
						if(IfMonsterCanThenDo(ctx, mm, rr, cc - 1) == false)
						{
							// ?? do nothing...
						}
					}
					else
					{
						//add_msg(ctx, "unknown move quadrant. %u.%u to %u.%u", rr, cc, gr, gc); draw_messages(ctx);
					}

					// dumb monsters wont go through doors....
				}
			}
		}
			// they dont move.
		else if(mm->disposition == eMD_HostileWatching
				|| mm->disposition == eMD_DocileWatching
				|| mm->disposition == eMD_DocileCantMove
				|| mm->disposition == eMD_HostileCantMove)
		{
			bool fight = false;

			// switch into aggressive mode.
			switch(mm->disposition)
			{
				case eMD_HostileCantMove:
				case eMD_DocileCantMove:
					mm->disposition = eMD_HostileCantMove;
					break;

				default:
					mm->disposition = eMD_HostileChasing;
					break;
			}

			if(mm->col - 1 == ctx->locn.col && mm->row - 1 == ctx->locn.row)
			{
				fight = true;
			}
			else if(mm->col == ctx->locn.col && mm->row - 1 == ctx->locn.row)
			{
				fight = true;
			}
			else if(mm->col + 1 == ctx->locn.col && mm->row - 1 == ctx->locn.row)
			{
				fight = true;
			}
			else if(mm->col - 1 == ctx->locn.col && mm->row == ctx->locn.row)
			{
				fight = true;
			}
			else if(mm->col + 1 == ctx->locn.col && mm->row == ctx->locn.row)
			{
				fight = true;
			}
			else if(mm->col - 1 == ctx->locn.col && mm->row + 1 == ctx->locn.row)
			{
				fight = true;
			}
			else if(mm->col == ctx->locn.col && mm->row + 1 == ctx->locn.row)
			{
				fight = true;
			}
			else if(mm->col + 1 == ctx->locn.col && mm->row + 1 == ctx->locn.row)
			{
				fight = true;
			}

			if(fight == true)
			{
				// bump combat!
				death = MonsterBumpFightsPlayer(ctx, mm);
			}
		}
	}
}

static uMapMonster *GetTargettedMonster(uContext *ctx)
{
	DLElement *e;
	uMapMonster *mm;

	if(ctx->player->target_idx == UINT16_MAX || ctx->player->target_idx == TARGET_MYSELF)
	{
		return NULL;
	}

	e = dlist_head(ctx->map->lstMonsters);
	while(e != NULL)
	{
		mm = dlist_data(e);
		e = dlist_next(e);

		if(mm->gen_id == ctx->player->target_idx)
		{
			return mm;
		}
	}

	return NULL;
}

#pragma disable_message(303);
static void applyPlayerMana(uContext *ctx, uint16_t spell, int16_t mana)
{
	if(mana != INT16_MAX)
	{
		// succeed
		if(ctx->player->mana == ctx->player->max_mana)
		{
			add_msg(ctx, "You dont feel any different.");
		}
		else
		{
			ctx->player->mana += mana;
			if(ctx->player->mana > ctx->player->max_mana)
			{
				mana -= (ctx->player->mana - ctx->player->max_mana);
				ctx->player->mana = ctx->player->max_mana;
			}

			add_msg(ctx, "You mana regenerates for %"
						 PRIi16
						 " points", mana);
			ctx->redraw_status = true;
		}
	}
}
#pragma enable_message(303);

#pragma disable_message(303);
static void applyPlayerHeal(uContext *ctx, uint16_t spell, int16_t heal)
{
	if(heal != INT16_MAX)
	{
		// succeed
		if(ctx->player->life == ctx->player->max_life)
		{
			add_msg(ctx, "You dont feel any different.");
		}
		else
		{
			ctx->player->life += heal;
			if(ctx->player->life > ctx->player->max_life)
			{
				heal -= (ctx->player->life - ctx->player->max_life);
				ctx->player->life = ctx->player->max_life;
			}

			add_msg(ctx, "You heal yourself for %"PRIi16" points", heal);
			ctx->redraw_status = true;
		}
	}
}
#pragma enable_message(303);

#pragma disable_message(303);
static void applyPlayerDamage(uContext *ctx, uint16_t spell, int16_t damage)
{
	if(damage != INT16_MAX)
	{
		// magic missile ignores armour
		if(spell != eST_MagicMissile)
		{
			if(damage > ctx->player->armour)
			{
				damage -= ctx->player->armour;
			}
			else
			{
				damage = 0;
			}
		}

		// succeed
		if(damage > ctx->player->life)
		{
			switch(xrnd(4))
			{
				case 0:
					add_msg(ctx, "You feel the magic consume you, killing yourself in the process!");
					break;
				case 1:
					add_msg(ctx, "BZZZrttt! The magic pops and your heart stops beating!");
					break;
				case 2:
					add_msg(ctx, "You crumple to the floor as the magic smashes into you. You die..");
					break;
				case 3:
					add_msg(ctx, "Everything goes white... you wonder what happened? DEATH!");
					break;
			}

			ctx->player->life = 0;
			DeathScreen(ctx, "committed suicide by magic");
		}
		else
		{
			ctx->player->life -= damage;
			add_msg(ctx, "The spell goes off on you for %"PRIi16" damage", damage);
			ctx->redraw_status = true;
		}
	}
}
#pragma enable_message(303);

bool DoRangedWand(uContext *ctx, uItem *currItem, uMapMonster *mm)
{
	bool monster_is_dead = false;
	int16_t spCost;
	int16_t castSpell;
	int16_t damage;
	int16_t heal;
	int16_t mana;
	uMonster *monster;
	int16_t mdefense_level;

	DList *lstMapMonster;

	if(currItem == NULL)
	{
		return monster_is_dead;
	}

	spCost = 0;
	castSpell = GetItemAttributeValue_Int(currItem, eIA_i_CastsSpell, INT16_MAX);
	if(castSpell != INT16_MAX)
	{
		spCost = GetItemAttributeValue_Int(currItem, eIA_i_CastManaCost, 0);
	}

	// item does not cast spell
	if(castSpell == INT16_MAX)
	{
		return false;
	}

	// do we have ammunition (aka) mana cost
	if(ctx->player->mana >= spCost)
	{
		if(GetItemAttributeValue_Bool(currItem, eIA_b_Unknown, false) == true)
		{
			// its unknown!
			if(skill_roll(ctx->player->skills[eSkill_ItemLore]) >= skill_roll(GetItemAttributeValue_Int(currItem, eIA_i_IdentifySkillLevel, 1)))
			{
				AddExperiece(ctx, 100);
				SetItemAttributeValue_Bool(currItem, eIA_b_Unknown, false);
			}
		}


		lstMapMonster = NewDList(NULL);

		// pay cost of spell
		ctx->player->mana -= spCost;

		// do wand firing!
		damage = GetItemAttributeValue_Die(currItem, eIA_d_DoRangedDamage, INT16_MAX);
		heal = GetItemAttributeValue_Die(currItem, eIA_d_DoHeal, INT16_MAX);
		mana = GetItemAttributeValue_Die(currItem, eIA_d_AddMana, INT16_MAX);

		// no defense against missile weapons but speed/2 (who can see arrows coming but high level peeps?
		if(ctx->player->target_idx == TARGET_MYSELF)
		{
			/////////
			// DAMAGE
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////
			applyPlayerDamage(ctx, castSpell, damage);

			/////////
			// HEAL
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////
			applyPlayerHeal(ctx, castSpell, heal);

			/////////
			// MANA
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////
			applyPlayerMana(ctx, castSpell, mana);
		}
		else
		{
			uAttribute *attr = GetItemAttributeValue_KV_attribute(currItem, eIA_kv_MagicColour);
			uint8_t a_clr;
			uint8_t a_asc;
			DLElement *e;
			uMapMonster *mx;
			uMonster *monx;

			if(attr == NULL)
			{
				a_clr = 0x0C;
				a_asc = '*';
			}
			else
			{
				a_clr = attr->attributes.kv.key;
				a_asc = attr->attributes.kv.value;
			}

			if(castSpell == eST_MagicMissile)
			{
				draw_line(ctx, ctx->locn.row, ctx->locn.col, mm->row, mm->col, a_clr, a_asc, true, lstMapMonster);
			}
			else if(castSpell == eST_AOE)
			{
				int i;
				int j;
				int r = 2;

				draw_line(ctx, ctx->locn.row, ctx->locn.col, mm->row, mm->col, a_clr, a_asc, true, lstMapMonster);
				for(i = r; i >= 0; i--)
				{
					for(j = r; j >= 0; j--)
					{
						if((MAX(j, i)) + ((MIN(j, i)) / 2) <= r)
						{
							draw_line(ctx, mm->row, mm->col, mm->row + abs(j), mm->col + abs(i), a_clr, a_asc, false, lstMapMonster);
							draw_line(ctx, mm->row, mm->col, mm->row + abs(j), mm->col - abs(i), a_clr, a_asc, false, lstMapMonster);
							draw_line(ctx, mm->row, mm->col, mm->row - abs(j), mm->col + abs(i), a_clr, a_asc, false, lstMapMonster);
							draw_line(ctx, mm->row, mm->col, mm->row - abs(j), mm->col - abs(i), a_clr, a_asc, false, lstMapMonster);
						}
					}
				}
				wait_ticks(AMMO_PAUSE_TIME);
			}

			ctx->redraw_dungeon = true;

			e = dlist_head(lstMapMonster);

			monster = GetMonsterByID(ctx, mm->monster_id);
			while(e != NULL)
			{
				mx = dlist_data(e);
				e = dlist_next(e);
				if(mx == NULL)
				{
					// special case, this is me!
					/////////
					// DAMAGE
					///////////////////////////////////////////////////////////////////////////////////////////////////////////////
					applyPlayerDamage(ctx, castSpell, damage);

					/////////
					// HEAL
					///////////////////////////////////////////////////////////////////////////////////////////////////////////////
					applyPlayerHeal(ctx, castSpell, heal);

					/////////
					// MANA
					///////////////////////////////////////////////////////////////////////////////////////////////////////////////
					applyPlayerMana(ctx, castSpell, mana);
				}
				else
				{
					monx = GetMonsterByID(ctx, mx->monster_id);
					mdefense_level = monx->defend_level / 3;
					mdefense_level *= 2;
					if(mdefense_level == 0)
					{
						mdefense_level = 1;
					}

					// monster has defended or not...
					if(skill_roll(mdefense_level) < skill_roll(ctx->player->skills[eSkill_Magic]))
					{
						/////////
						// DAMAGE
						////////////////////////////////////////////////////////////////////////////////////////////////////
						if(damage != INT16_MAX)
						{
							// magic missile ignores armour
							if(castSpell != eST_MagicMissile)
							{
								if(damage > mx->armour)
								{
									damage -= mx->armour;
								}
								else
								{
									damage = 0;
								}
							}

							// hit!
							if(damage >= mx->health)
							{
								if(monx == monster)
								{
									add_msg(ctx, "You zap the %s for %u points, it dies.", monx->name, damage);
								}

								mx->health = 0;
								ctx->player->score += monx->xp / 2;
								AddExperiece(ctx, monx->xp);
								KillMonster(ctx, mx);

								if(monx == monster)
								{
									monster_is_dead = true;
								}

								add_monster(ctx);
							}
							else
							{
								mx->health -= damage;
								if(monx == monster)
								{
									add_msg(ctx, "You zap the %s for %u points damage", monx->name, damage);
								}
							}
						}

						/////////
						// HEAL
						////////////////////////////////////////////////////////////////////////////////////////////////////
						if(heal != INT16_MAX)
						{
							// NOTE: Monsters dont have a max health. If player heals monster... bonus health.
							mx->health += heal;
							if(monx == monster)
							{
								add_msg(ctx, "You zap the %s for %u points healing it", monx->name, heal);
							}
						}

						/////////
						// MANA
						////////////////////////////////////////////////////////////////////////////////////////////////////
						// monsters dont have a mana stat!
						// no adjustment necessary

					}
					else
					{
						if(monx == monster)
						{
							add_msg(ctx, "You miss the %s.", monster->name);
						}
					}
				}
			}
		}

		FreeDList(lstMapMonster);
	}
	else
	{
		// no ammo
		add_msg(ctx, "You dont have the mana to power the %s", GetItemName(currItem));
	}

	return monster_is_dead;
}

bool FireOnTarget(uContext *ctx)
{
	bool monster_is_dead = false;

	// do we have target?
	uItem *itemLH, *itemRH;
	int16_t castSpellLH;
	int16_t castSpellRH;

	int16_t mdefense_level;

	int16_t damage1;
	int16_t damage2;

	uMapMonster *mm;
	uMonster *monster;

	// nothing targetted
	if(ctx->player->target_idx == UINT16_MAX)
	{
		return monster_is_dead;
	}

	mm = GetTargettedMonster(ctx);

	itemLH = NULL;
	itemRH = NULL;

	itemLH = GetPlayerItemWeaponLeft(ctx);
	itemRH = GetPlayerItemWeaponRight(ctx);

	// do we have ranged weapon in hand? or wand? or staff?

	// bow will always be 2h, so in left hand
	if(itemLH != NULL && GetItemAttributeValue_Bool(itemLH, eIA_b_FireTargetBow, false) == true && itemRH != NULL && itemRH->base_type == eIT_AmmunitionForBow)
	{
		// do we have ammunition, always be in right hand
		if(ctx->player->items[ctx->player->item_right_weap].count > 0)
		{
			// do bow firing!
			castSpellLH = GetItemAttributeValue_Int(itemLH, eIA_i_CastsSpell, INT16_MAX);
			damage1 = GetItemAttributeValue_Die(itemLH, eIA_d_DoRangedDamage, 0);

			// ammunition bonus
			castSpellRH = GetItemAttributeValue_Int(itemRH, eIA_i_CastsSpell, INT16_MAX);
			damage2 = GetItemAttributeValue_Die(itemRH, eIA_d_DoRangedDamage, 0);

			damage1 += damage2;

			// TODO: apply spell on hit?


			// no defense against missile weapons but speed/2 (who can see arrows coming but high level peeps?
			if(ctx->player->target_idx == TARGET_MYSELF)
			{
				// succeed
				if(damage1 > ctx->player->life)
				{
					switch(xrnd(4))
					{
						case 0:
							add_msg(ctx, "You fire an arrow into your eye socket, killing yourself in the process!");
							break;
						case 1:
							add_msg(ctx, "You fire an arrow into your thigh, opening an artery and killing yourself!");
							break;
						case 2:
							add_msg(ctx, "You swallow an arrow, choking to death!");
							break;
						case 3:
							add_msg(ctx, "There is an arrow sticking out of your chest... what happened? You died!");
							break;
					}

					ctx->player->life = 0;
					DeathScreen(ctx, "committed suicide");
				}
				else
				{
					ctx->player->life -= damage1;
					add_msg(ctx, "You fire an arrow into your foot for %"
								 PRIi16
								 " damage", damage1);
					ctx->redraw_status = true;
				}
			}
			else
			{
				assert(mm != NULL);
				monster = GetMonsterByID(ctx, mm->monster_id);
				assert(monster != NULL);

				mdefense_level = monster->defend_level / 3;
				mdefense_level *= 2;
				if(mdefense_level == 0)
				{
					mdefense_level = 1;
				}

				if(skill_roll(mdefense_level) < skill_roll(ctx->player->skills[eSkill_RangedWeapons]))
				{
					draw_line(ctx, ctx->locn.row, ctx->locn.col, mm->row, mm->col, 0x6, '*', true, NULL);

					ctx->redraw_dungeon = true;

					// hit!
					if(damage1 > mm->health)
					{
						add_msg(ctx, "You hit the %s for %u points, it dies.", monster->name, damage1);
						mm->health = 0;

						ctx->player->score += monster->xp / 2;
						AddExperiece(ctx, monster->xp);

						KillMonster(ctx, mm);
						monster_is_dead = true;
						add_monster(ctx);
					}
					else
					{
						mm->health -= damage1;
						add_msg(ctx, "You hit the %s for %u points.", monster->name, damage1);
					}
				}
				else
				{
					draw_line(ctx, ctx->locn.row, ctx->locn.col, mm->row, mm->col, 0x6, '*', true, NULL);
					ctx->redraw_dungeon = true;

					add_msg(ctx, "You miss the %s.", monster->name);
				}
			}

			// remove arrow. zero check at end before we destroy item!
			ctx->player->items[ctx->player->item_right_weap].count -= 1;

			// delete item now its done if its at 0
			if(ctx->player->items[ctx->player->item_right_weap].count == 0)
			{
				// if name ends in S do singular/plurar
				if(StringEndsInS(GetItemName(itemRH)) == true)
				{
					add_msg(ctx, "That was the last of your %s", GetItemName(itemRH));
				}
				else
				{
					add_msg(ctx, "That was the last %s", GetItemName(itemRH));
				}

				// delete item!
				ClearRightWeapon(ctx);
			}
		}
		else
		{
			// no ammo
		}
	}
	else if(GetItemAttributeValue_Bool(itemLH, eIA_b_FireTargetWand, true) == true || GetItemAttributeValue_Bool(itemRH, eIA_b_FireTargetWand, false) == true)
	{
		// we have a wand in one or both of our hands!
		monster_is_dead = DoRangedWand(ctx, itemLH, mm);
		if(monster_is_dead == false)
		{
			monster_is_dead = DoRangedWand(ctx, itemRH, mm);
		}
	}

	return monster_is_dead;
}

bool CanMoveTo(uContext *ctx, uint16_t r, uint16_t c)
{
	if(r >= MAX_MAP_HEIGHT || c >= MAX_MAP_WIDTH)
	{
		return false;
	}

	if(DoBumpCombat(ctx, r, c) == false)
	{
		return false;
	}

	//ctx->map->cells[(MAX_MAP_WIDTH * r) + c].tile.tile
	switch(get_cell_tile(ctx, r, c))
	{
		case eTile_ClosedDoor:
			if(ctx->auto_open_doors == true)
			{
				// bump into closed to open it
				return true;
			}

			return false;
			break;
	}

	//if((ctx->map->cells[(MAX_MAP_WIDTH * r) + c].tile.flags & TILE_FLAG_OPAQUE) == TILE_FLAG_OPAQUE)
	if((get_cell_flags(ctx, r, c) & TILE_FLAG_OPAQUE) == TILE_FLAG_OPAQUE)
	{
		return false;
	}

	return true;
}

void DoPickup(uContext *ctx, uint16_t r, uint16_t c, bool force_pickup)
{
	uMapItem *item;

//	eDT_Gold,
//	eDT_Weapon,
//	eDT_Armour,
//	eDT_Potion,
//	eDT_Food,
//	eDT_Item,
//
	char *pickup_msgs[] = {
		"",
		"You pickup a weapon",
		"You pickup some armour",
		"You pickup a potion",
		"You pickup some food",
		"You pickup an item"
	};

	item = GetMapItem(ctx, r, c);

	if(item != NULL)
	{
		switch(item->type)
		{
			case eDT_Weapon:
			case eDT_Armour:
			case eDT_Potion:
			case eDT_Food:
			case eDT_Item:
				// no auto pick up of these items
				if(force_pickup == true)
				{
					uItem *ii;
					uint16_t slot;
					bool skip = false;

					// 1 - do we have inventory space?
					// 2 - put it in inventory, (or add count if stackable attribute)
					// 3 - if item lore test passes, identify it.

					// test if its a stackable item and if we have any already?
					ii = FindItemByID(ctx, item->data.i_item.idx);
					assert(ii != NULL);
					if(GetItemAttributeValue_Bool(ii, eIA_b_Stackable, false) == true)
					{
						// look in inventory for same item ID
						slot = FindItemSlotInInventoryByID(ctx, ii->idx);
						if(slot != UINT16_MAX)
						{
							if(item->data.i_item.count + ctx->player->items[slot].count < UINT8_MAX)
							{
								ctx->player->items[slot].count += item->data.i_item.count;
								add_msg(ctx, "You add %u %s to your stack", item->data.i_item.count, ctx->player->items[slot].item.name);
								skip = true;
								DeleteItem(ctx, item);
							}
						}
					}

					if(skip == false)
					{
						slot = GetFreeItemSlot(ctx);
						if(slot != UINT16_MAX)
						{
							if(item->data.i_item.has_item_data == true)
							{
								memmove(&ctx->player->items[slot], &item->data.i_item.item, sizeof(uItem));
							}
							else
							{
								GenerateCopy(ii, &ctx->player->items[slot].item);
							}

							ii = &ctx->player->items[slot].item;
							if(item->data.i_item.count >= 1)
							{
								ctx->player->items[slot].count = item->data.i_item.count;
							}
							else
							{
								ctx->player->items[slot].count = 1;
							}

							// if it unknown? test it
							if(GetItemAttributeValue_Bool(ii, eIA_b_Unknown, false) == true)
							{
								// its unknown!
								if(skill_roll(ctx->player->skills[eSkill_ItemLore]) >= skill_roll(GetItemAttributeValue_Int(ii, eIA_i_IdentifySkillLevel, 1)))
								{
									add_msg(ctx, "You pickup the %s", ctx->player->items[slot].item.name);

									AddExperiece(ctx, 100);
									SetItemAttributeValue_Bool(ii, eIA_b_Unknown, false);
								}
								else
								{
									add_msg(ctx, "%s", pickup_msgs[item->type]);
								}
							}
							else
							{
								add_msg(ctx, "You pickup the %s", ii->name);
							}

							DeleteItem(ctx, item);
						}
						else
						{
							add_msg(ctx, "No space in your inventory to grab this item");
						}
					}
				}
				break;

			case eDT_Gold:
				if(force_pickup == true || ctx->auto_get_money == true)
				{
					ctx->player->money += item->data.i_gold.gold;
					add_msg(ctx, "You picked up %u gold", item->data.i_gold.gold);
					AddExperiece(ctx, item->data.i_gold.gold / 2);
					DeleteItem(ctx, item);
				}
				break;
		}
	}
}

// triggers
void MovePlayerTo(uContext *ctx, uint16_t r, uint16_t c)
{
	uint16_t q;

	if(r >= MAX_MAP_HEIGHT || c >= MAX_MAP_WIDTH)
	{
		return;
	}

	q = (MAX_MAP_WIDTH * r) + c;

	// will do an auto pickup of what we are on...
	DoPickup(ctx, r, c, false);

	switch(ctx->map->cells[q].tile.tile)
	{
		case eTile_ClosedDoor:
			ctx->map->cells[q].tile.tile = eTile_OpenDoor;
			ctx->map->cells[q].tile.flags &= ~TILE_FLAG_OPAQUE;
			add_msg(ctx, "You open the door");
			break;
	}

	// each un-visited cell gets 1pt!
	if((ctx->map->cells[q].tile.flags & TILE_FLAG_VISITED) != TILE_FLAG_VISITED)
	{
		ctx->map->cells[q].tile.flags |= TILE_FLAG_VISITED;
		ctx->player->score += 1;
		ctx->redraw_status = true;
	}

	// trigs on r/c
	ProcessMapTriggers(ctx, r, c);
}

void calc_draw_offseet(uContext *ctx)
{
	clear_los(ctx);
	reset_los_square();
	// draw_row + draw_col is MY location

	if(ctx->locn.row < ctx->status_row / 2)
	{
		ctx->draw_top_row = 0;
		ctx->draw_row = ctx->locn.row;
	}
	else
	{
		ctx->draw_row = ctx->status_row / 2;
		ctx->draw_top_row = ctx->locn.row - ctx->draw_row;
	}

	if(ctx->locn.col < screen_get_width() / 2)
	{
		ctx->draw_top_col = 0;
		ctx->draw_col = ctx->locn.col;
	}
	else
	{
		ctx->draw_col = (screen_get_width() / 2);
		ctx->draw_top_col = ctx->locn.col - ctx->draw_col;
	}

//	if(ctx->locn.row < ctx->status_row / 2)
//	{
//		ctx->draw_row = ctx->locn.row;
//		ctx->draw_top_row = 0;
//	}
//	else
//	{
//		ctx->draw_row = ctx->status_row / 2;
//		ctx->draw_top_row = ctx->locn.row - ctx->draw_row;
//	}
//
//	if(ctx->locn.col < screen_get_width() / 2)
//	{
//		ctx->draw_col = ctx->locn.col;
//		ctx->draw_top_col = 0;
//	}
//	else
//	{
//		ctx->draw_col = (screen_get_width() / 2);
//		ctx->draw_top_col = ctx->locn.col - ctx->draw_col;
//	}
}

static void NewGame(uContext *ctx)
{
	// we have new player instance, new name.
	// setup our base variables.

	GenerateMap(ctx, 1, 1, true);
}

static void draw_dungeon(uContext *ctx)
{
	uint16_t rr, cc;
	uint8_t clr;
	uint8_t f;
	uint16_t r;

	for(rr = 0; rr < ctx->status_row; rr++)
	{
		if((ctx->draw_top_row + rr) < MAX_MAP_HEIGHT)
		{
			r = (MAX_MAP_WIDTH * (ctx->draw_top_row + rr));

			screen_line_clear(rr, 7);

			for(cc = 0; cc < screen_get_width(); cc++)
			{
				clr = 0x8;

				if((ctx->draw_top_col + cc) < MAX_MAP_WIDTH)
				{
					f = ctx->map->cells[r + (ctx->draw_top_col + cc)].tile.flags;

					if(f >= 0x80)
					{
						clr = ascii_colors[ctx->map->cells[r + (ctx->draw_top_col + cc)].tile.tile];
					}

					if((f & TILE_FLAG_SEEN) == TILE_FLAG_SEEN)
					{
						screen_outch(rr, cc, clr, ascii_tiles[ctx->map->cells[r + (ctx->draw_top_col + cc)].tile.tile]);
					}
				}
			}
		}
	}
}

static uint8_t light_object(uContext *ctx, uint16_t col, uint16_t row)
{
	// gold, weapon, armour, potion
	// maps directly to eDroppedType
//	eDT_Gold,
//	eDT_Weapon,
//	eDT_Armour,
//	eDT_Potion,
//	eDT_Food,
//	eDT_Item,
	char ascii_items[] = {'$', ')', '&', '!', ';', '?'};
	char ascii_items_colours[] = {0x0E, 0x06, 0x06, 0x06, 0x06, 0x06};

	uint16_t q;
	uMapCell *t;

	uMapItem *item;
	uMapMonster *mm;
	uMonster *m;

	if(row >= MAX_MAP_HEIGHT || col >= MAX_MAP_WIDTH)
	{
		return 0;
	}

	q = (MAX_MAP_WIDTH * (ctx->draw_top_row + row)) + (ctx->draw_top_col + col);
	t = &ctx->map->cells[q];
	t->tile.flags |= TILE_FLAG_LIT | TILE_FLAG_SEEN;

	if(row < min_light_row)
	{
		min_light_row = row;
	}

	if(row > max_light_row)
	{
		max_light_row = row;
	}

	if(col < min_light_col)
	{
		min_light_col = col;
	}

	if(col > max_light_col)
	{
		max_light_col = col;
	}

	if(row >= ctx->status_row || col >= 80)
	{
		return t->tile.flags;
	}

	mm = GetMonsterOnMap(ctx, ctx->draw_top_row + row, ctx->draw_top_col + col);

	if(mm != NULL)
	{
		m = GetMonsterByID(ctx, mm->monster_id);

		if(m != NULL)
		{
			if(mm->gen_id == ctx->player->target_idx)
			{
				screen_outch(row, col, 0x70, m->tile);
			}
			else
			{
				screen_outch(row, col, m->colour, m->tile);
			}

			// stack them into the targetting fov

			for(q = 0; q < ctx->player->monster_fov_idx; q++)
			{
				if(ctx->player->monster_fov[q] == mm->gen_id)
				{
					return TILE_FLAG_OPAQUE;
				}
			}

			if(ctx->player->monster_fov_idx < MAX_MONSTER_FOV - 1)
			{
				ctx->player->monster_fov[ctx->player->monster_fov_idx++] = mm->gen_id;
			}

			// can we see through monsters? no.
			return TILE_FLAG_OPAQUE;
		}
		else
		{
			// should not see this...
			screen_outch(row, col, '?', 0xF);
			return TILE_FLAG_OPAQUE;
		}
	}
	else
	{
		item = GetMapItem(ctx, ctx->draw_top_row + row, ctx->draw_top_col + col);

		if(item != NULL)
		{
			screen_outch(row, col, ascii_items_colours[item->type], ascii_items[item->type]);
		}
		else
		{
			// draws object in proper colour
			screen_outch(row, col, ascii_colors[t->tile.tile], ascii_tiles[t->tile.tile]);
		}
	}

	return t->tile.flags;
}

// col=x, row=y within draw window
static void line(uContext *ctx, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t w, uint16_t h)
{
	int16_t x, y;
	int16_t deltaX, deltaY;
	int16_t stepX, stepY;
	int16_t errorX, errorY;
	int16_t detdelta;
	uint16_t count;

	/* Vertical line */
	if(x1 < 0)
	{
		x1 = 0;
	}
	if(x2 < 0)
	{
		x2 = 0;
	}
	if(y1 < 0)
	{
		y1 = 0;
	}
	if(y2 < 0)
	{
		y2 = 0;
	}

	if(x1 == x2)
	{
		// fix for same destination.
		if(y1 == y2)
		{
			light_object(ctx, x1, y1);
			return;
		}

		if(y1 > y2)
		{
			y = -1;
		}
		else
		{
			y = 1;
		}

		for(; y1 != y2; y1 += y)
		{
			if((light_object(ctx, x1, y1) & TILE_FLAG_OPAQUE) == TILE_FLAG_OPAQUE)
			{
				return;
			}
		}

		if(y1 < h && x1 < w)
		{
			light_object(ctx, x1, y1);
		}

		return;
	}

	/* Horizontal line */
	if(y1 == y2)
	{
		if(x1 == x2)
		{
			light_object(ctx, x1, y1);
			return;
		}

		if(x1 > x2)
		{
			x = -1;
		}
		else
		{
			x = 1;
		}

		for(; x1 != x2; x1 += x)
		{
			if((light_object(ctx, x1, y1) & TILE_FLAG_OPAQUE) == TILE_FLAG_OPAQUE)
			{
				return;
			}
		}

		if(y1 < h && x1 < w)
		{
			light_object(ctx, x1, y1);
		}

		return;
	}

	y = y1;
	x = x1;

	if(y1 < h && x1 < w)
	{
		if((light_object(ctx, x1, y1) & TILE_FLAG_OPAQUE) == TILE_FLAG_OPAQUE)
		{
			return;
		}
	}

	stepY = 1;
	deltaY = y2 - y1;

	if(deltaY < 0)
	{
		stepY = -1;
		deltaY = -deltaY;
	}

	stepX = 1;
	deltaX = x2 - x1;

	if(deltaX < 0)
	{
		stepX = -1;
		deltaX = -deltaX;
	}

	if(deltaY > deltaX)
	{
		count = deltaY;
		detdelta = deltaY;
		errorX = deltaY >> 1;
		errorY = 0;
	}
	else
	{
		count = deltaX;
		detdelta = deltaX;
		errorX = 0;
		errorY = deltaX >> 1;
	}

	do
	{
		errorY = (errorY + deltaY);

		if(errorY >= detdelta)
		{
			errorY -= detdelta;
			y += stepY;
		}

		errorX = (errorX + deltaX);

		if(errorX >= detdelta)
		{
			errorX -= detdelta;
			x += stepX;
		}

		if(y < h && x < w)
		{
			if((light_object(ctx, x, y) & TILE_FLAG_OPAQUE) == TILE_FLAG_OPAQUE)
			{
				return;
			}
		}

		count--;
	} while((int16_t)count > 0);

	if(y < h && x < w)
	{
		light_object(ctx, x, y);
	}
}

// we give r/c where we are in the draw window
static void draw_los(uContext *ctx, int16_t row, int16_t col, uint16_t radius)
{
	int16_t i;
	int16_t j;

	uint16_t w, h;

	w = screen_get_width();
	h = ctx->status_row;

	for(i = radius; i >= 0; i--)
	{
		for(j = radius; j >= 0; j--)
		{
			if((MAX(j, i)) + ((MIN(j, i)) / 2) <= radius)    //octagonal LOS/FOV
			{
				line(ctx, col, row, col + abs(j), row + abs(i), w, h);
				line(ctx, col, row, col + abs(j), row - abs(i), w, h);
				line(ctx, col, row, col - abs(j), row + abs(i), w, h);
				line(ctx, col, row, col - abs(j), row - abs(i), w, h);
			}
		}
	}

	for(i = 0; i < ctx->player->monster_fov_idx; i++)
	{
		if(ctx->player->monster_fov[i] == ctx->player->target_idx)
		{
			// shift target selection index
			ctx->player->monster_fov_selected_idx = i;
			return;
		}
	}

	// if I am target.. we ok, dont reset
	if(ctx->player->target_idx == TARGET_MYSELF)
	{
		return;
	}

	// no? reset target list

	// old target not in fov list. reset!
	ctx->player->monster_fov_selected_idx = UINT8_MAX;
	ctx->player->target_idx = UINT16_MAX;
}


static void draw_status(uContext *ctx)
{
	uint16_t r = ctx->status_row;

	screen_line_clear(r, 0x70);

	msg(r, 1, 0x70, "%s", ctx->player->name);
	msg(r, 26, 0x70, "Life: %"
					 PRIi16
					 ", Mana: %"
					 PRIi16
					 ", Armour: %"
					 PRIi16, ctx->player->life, ctx->player->mana, ctx->player->armour);

	if(ctx->player->level_up_points > 0)
	{
		if(ctx->player->level_up_points > 9)
		{
			msg(r, 75, 0x70, "\x18*");
		}
		else
		{
			msg(r, 75, 0x70, "\x18%u", ctx->player->level_up_points);
		}
	}

	screen_outch(r, 78, 0x70, 'L');
	if(ctx->player->loadout == 0)
	{
		screen_outch(r, 79, 0x70, '1');
	}
	else
	{
		screen_outch(r, 79, 0x70, '2');
	}

}

static void draw_messages(uContext *ctx)
{
	uint16_t i;
	uint16_t j;

	DLElement *e;
	uMessage *m;

	i = 1 + ctx->status_row;
	j = screen_get_height() - 1;

	e = dlist_tail(ctx->lstMessages);

	while(j >= i)
	{
		screen_line_clear(j, 7);

		if(e != NULL)
		{
			m = dlist_data(e);
			e = dlist_prev(e);

			if(m->count > 1)
			{
				msg_hl(j, 0, HL_CLR, "%s (%u)", m->msg, m->count);
			}
			else
			{
				msg_hl(j, 0, HL_CLR, "%s", m->msg);
			}
		}

		j -= 1;
	}
}

static void draw_me(uContext *ctx)
{
	if(ctx->player->target_idx == TARGET_MYSELF)
	{
		screen_outch(ctx->draw_row, ctx->draw_col, 0x70, '@');
	}
	else
	{
		screen_outch(ctx->draw_row, ctx->draw_col, 7, '@');
	}
}

static void redraw_all(uContext *ctx)
{
	clear_los(ctx);
	toggle_swap_draw_to_backbuffer();
	draw_dungeon(ctx);
	draw_los(ctx, ctx->draw_row, ctx->draw_col, ctx->player->stats[eStat_DeepVision]);
	draw_me(ctx);
	draw_status(ctx);
	draw_messages(ctx);
	toggle_swap_draw_to_backbuffer();
	restore_from_backbuffer();

	ctx->redraw_dungeon = false;
	ctx->redraw_status = false;
	ctx->redraw_messages = false;
}

static void ContinueGame(uContext *ctx)
{
	// draw dungeon
	bool quit = false;
	bool redraw = true;
	bool passturn = true;
	bool frozen = false;
	uint16_t key;

	reset_los_square();

	redraw_all(ctx);
	redraw = true;

	while(quit == false)
	{
		frozen = false;

		if(ctx->player->life > 0)
		{
			if(redraw == true)
			{
				redraw = false;
				redraw_all(ctx);
			}
			else
			{
				if(ctx->redraw_dungeon == true)
				{
					ctx->redraw_dungeon = false;

					clear_los(ctx);
					save_to_backbuffer();
					toggle_swap_draw_to_backbuffer();
					draw_dungeon(ctx);
					draw_los(ctx, ctx->draw_row, ctx->draw_col, ctx->player->stats[eStat_DeepVision]);
					draw_me(ctx);
					toggle_swap_draw_to_backbuffer();
					restore_from_backbuffer();
				}

				if(ctx->redraw_status == true)
				{
					toggle_swap_draw_to_backbuffer();
					ctx->redraw_status = false;
					draw_status(ctx);
					toggle_swap_draw_to_backbuffer();
					restore_from_backbuffer();
				}

				if(ctx->redraw_messages == true)
				{
					toggle_swap_draw_to_backbuffer();
					ctx->redraw_messages = false;
					draw_messages(ctx);
					toggle_swap_draw_to_backbuffer();
					restore_from_backbuffer();
				}
			}

			if(passturn == true)
			{
				passturn = false;

				ctx->moves += 1;

				MonstersTurn(ctx);
				redraw = true;

				if(ctx->player->life > 0)
				{
					ProcessInventoryTurnItemTriggers(ctx);
					for(key = 0; frozen == false && ctx->player->life > 0 && key < MAX_COUNTER_ITEMS; key++)
					{
						if(ctx->player->counters[key].effect == eIA_i_HeldInPosition)
						{
							if(ctx->player->counters[key].rounds > 0)
							{
								frozen = true;
							}
						}
					}
				}
			}
		}

		if(ctx->player->life > 0)
		{
			if(redraw == true)
			{
				redraw = false;

				clear_los(ctx);

				toggle_swap_draw_to_backbuffer();
				draw_dungeon(ctx);
				draw_los(ctx, ctx->draw_row, ctx->draw_col, ctx->player->stats[eStat_DeepVision]);
				draw_me(ctx);
				draw_status(ctx);
				draw_messages(ctx);
				toggle_swap_draw_to_backbuffer();
				restore_from_backbuffer();

				ctx->redraw_dungeon = false;
				ctx->redraw_status = false;
				ctx->redraw_messages = false;
			}
			else
			{
				if(ctx->redraw_dungeon == true)
				{
					ctx->redraw_dungeon = false;

					clear_los(ctx);

					save_to_backbuffer();
					toggle_swap_draw_to_backbuffer();
					draw_dungeon(ctx);
					draw_los(ctx, ctx->draw_row, ctx->draw_col, ctx->player->stats[eStat_DeepVision]);
					draw_me(ctx);
					toggle_swap_draw_to_backbuffer();
					restore_from_backbuffer();
				}

				if(ctx->redraw_status == true)
				{
					toggle_swap_draw_to_backbuffer();
					ctx->redraw_status = false;
					draw_status(ctx);
					toggle_swap_draw_to_backbuffer();
					restore_from_backbuffer();
				}

				if(ctx->redraw_messages == true)
				{
					toggle_swap_draw_to_backbuffer();
					ctx->redraw_messages = false;
					draw_messages(ctx);
					toggle_swap_draw_to_backbuffer();
					restore_from_backbuffer();
				}
			}

			passturn = true;
			if(frozen == true)
			{
				key = DOSKeyToInternal(ctx, gkey(NULL));
				if(key != eKey_MAX)
				{
					if(skill_roll(4) < skill_roll(ctx->player->stats[eStat_Strength]))
					{
						add_msg(ctx, "I break free");
						ClearCounter(ctx, eIA_i_HeldInPosition);
					}
					else
					{
						add_msg(ctx, "I cant move, I'm stuck!");
					}
				}
				key = eKey_PassTurn;
			}
			else
			{
				key = DOSKeyToInternal(ctx, gkey(NULL));
			}

			switch(key)
			{
				case eKey_Quit:
					passturn = false;

					add_msg(ctx, "Do you really want to Quit? Y/N [YyNn]?");

					toggle_swap_draw_to_backbuffer();
					draw_messages(ctx);
					toggle_swap_draw_to_backbuffer();
					restore_from_backbuffer();

					key = gkey("YyNn");

					if(key == 'Y' || key == 'y')
					{
						quit = true;
						DeathScreen(ctx, "Rage Quit");
					}
					else
					{
						update_last_message(ctx, " - No");
					}
					break;

				case eKey_Version:
					passturn = false;
					add_msg(ctx, "`Underdark` version %s %s by BloodyCactus", VersionStringFull(), REL_STR);
					break;

				case eKey_PassTurn:
					// ok. update turn!
					redraw = true;
					break;

				case eKey_Up:
					if(CanMoveTo(ctx, ctx->locn.row - 1, ctx->locn.col) == true)
					{
						ctx->locn.row -= 1;
						if(ctx->draw_row > 4)
						{
							ctx->draw_row -= 1;
						}
						else
						{
							calc_draw_offseet(ctx);
						}

						MovePlayerTo(ctx, ctx->locn.row, ctx->locn.col);
						redraw = true;
					}
					break;

				case eKey_UpRight:
					if(CanMoveTo(ctx, ctx->locn.row - 1, ctx->locn.col + 1) == true)
					{
						bool recalc = false;

						ctx->locn.row -= 1;
						ctx->locn.col += 1;

						if(ctx->draw_row > 4)
						{
							ctx->draw_row -= 1;
						}
						else
						{
							recalc = true;
						}

						if(ctx->draw_col < screen_get_width() - 4)
						{
							ctx->draw_col += 1;
						}
						else
						{
							recalc = true;
						}

						if(recalc == true)
						{
							calc_draw_offseet(ctx);
						}

						MovePlayerTo(ctx, ctx->locn.row, ctx->locn.col);
						redraw = true;
					}
					break;

				case eKey_UpLeft:
					if(CanMoveTo(ctx, ctx->locn.row - 1, ctx->locn.col - 1) == true)
					{
						bool recalc = false;

						ctx->locn.row -= 1;
						ctx->locn.col -= 1;

						if(ctx->draw_row > 4)
						{
							ctx->draw_row -= 1;
						}
						else
						{
							recalc = true;
						}

						if(ctx->draw_col > 4)
						{
							ctx->draw_col -= 1;
						}
						else
						{
							recalc = true;
						}

						if(recalc == true)
						{
							calc_draw_offseet(ctx);
						}

						MovePlayerTo(ctx, ctx->locn.row, ctx->locn.col);
						redraw = true;
					}
					break;

				case eKey_Down:
					if(CanMoveTo(ctx, ctx->locn.row + 1, ctx->locn.col) == true)
					{
						ctx->locn.row += 1;

						if(ctx->draw_row < ctx->status_row - 4)
						{
							ctx->draw_row += 1;
						}
						else
						{
							calc_draw_offseet(ctx);
						}

						MovePlayerTo(ctx, ctx->locn.row, ctx->locn.col);
						redraw = true;
					}
					break;

				case eKey_DownRight:
					if(CanMoveTo(ctx, ctx->locn.row + 1, ctx->locn.col + 1) == true)
					{
						bool recalc = false;

						ctx->locn.row += 1;
						ctx->locn.col += 1;

						if(ctx->draw_col < screen_get_width() - 4)
						{
							ctx->draw_col += 1;
						}
						else
						{
							recalc = true;
						}

						if(ctx->draw_row < ctx->status_row - 4)
						{
							ctx->draw_row += 1;
						}
						else
						{
							recalc = true;
						}

						if(recalc == true)
						{
							calc_draw_offseet(ctx);
						}

						MovePlayerTo(ctx, ctx->locn.row, ctx->locn.col);
						redraw = true;
					}
					break;

				case eKey_DownLeft:
					if(CanMoveTo(ctx, ctx->locn.row + 1, ctx->locn.col - 1) == true)
					{
						bool recalc = false;

						ctx->locn.row += 1;
						ctx->locn.col -= 1;

						if(ctx->draw_col > 4)
						{
							ctx->draw_col -= 1;
						}
						else
						{
							recalc = true;
						}

						if(ctx->draw_row < ctx->status_row - 4)
						{
							ctx->draw_row += 1;
						}
						else
						{
							recalc = true;
						}

						if(recalc == true)
						{
							calc_draw_offseet(ctx);
						}

						MovePlayerTo(ctx, ctx->locn.row, ctx->locn.col);
						redraw = true;
					}
					break;

				case eKey_Left:
					if(CanMoveTo(ctx, ctx->locn.row, ctx->locn.col - 1) == true)
					{
						ctx->locn.col -= 1;

						if(ctx->draw_col > 4)
						{
							ctx->draw_col -= 1;
						}
						else
						{
							calc_draw_offseet(ctx);
						}

						MovePlayerTo(ctx, ctx->locn.row, ctx->locn.col);
						redraw = true;
					}
					break;

				case eKey_Right:
					if(CanMoveTo(ctx, ctx->locn.row, ctx->locn.col + 1) == true)
					{
						ctx->locn.col += 1;

						if(ctx->draw_col < screen_get_width() - 4)
						{
							ctx->draw_col += 1;
						}
						else
						{
							calc_draw_offseet(ctx);
						}

						MovePlayerTo(ctx, ctx->locn.row, ctx->locn.col);
						redraw = true;
					}
					break;

				case eKey_Inventory:
					passturn = false;
					show_inventory(ctx);
					redraw = true;
					break;

				case eKey_Descend:
					if(IsStandingOnTile(ctx, ctx->locn.row, ctx->locn.col, eTile_StairsDown) == true)
					{
						// bottom level? even if we scoped for 25!
						if(ctx->locn.level == 10)
						{
							//NOTE: no descendant stairs on level 10...
						}
						else
						{
							quit = DescendLevel(ctx);
							clear_los(ctx);
							redraw = true;
						}
					}
					break;

				case eKey_Ascend:
					if(IsStandingOnTile(ctx, ctx->locn.row, ctx->locn.col, eTile_StairsUp) == true)
					{
						// ascend, generate or load...
						if(ctx->locn.level == 1)
						{
							// win?
							// faux winning score
							ctx->player->score += 100;

							if(bit_get_bit(ctx->game_flags, eFlag_GameWin) == true)
							{
								ctx->player->score += 10000;
								DeathScreen(ctx, "WON THE GAME!");
							}
							else
							{
								DeathScreen(ctx, "left the dungeon.");
							}

							quit = true;
						}
						else
						{
							SaveMap(ctx);
							ctx->locn.level -= 1;

							// load or generate previous level
							if(bit_get_bit(ctx->game_flags, eFlag_GenLevel0 + ctx->locn.level) == true)
							{
								if(LoadMap(ctx) == false)
								{
									DeathScreen(ctx, "Loading of map failed");
									quit = true;
								}
							}
							else
							{
								GenerateMap(ctx, ctx->locn.map, ctx->locn.level, true);
							}

							calc_draw_offseet(ctx);
							clear_los(ctx);
							redraw = true;
						}
					}
					break;

				case eKey_Open:
					redraw = DoDoorOpenClose(ctx, ctx->locn.row, ctx->locn.col, true);
					break;

				case eKey_Close:
					redraw = DoDoorOpenClose(ctx, ctx->locn.row, ctx->locn.col, false);
					break;

				case eKey_Target:
					if(ctx->player->monster_fov_idx > 0)
					{
						passturn = false;

						// no selection, select first monster
						if(ctx->player->target_idx == UINT16_MAX)
						{
							ctx->player->monster_fov_selected_idx = 0;
							ctx->player->target_idx = ctx->player->monster_fov[ctx->player->monster_fov_selected_idx];
						}
						else if(ctx->player->target_idx == TARGET_MYSELF)
						{
							// i'm last, do reset
							ctx->player->monster_fov_selected_idx = UINT8_MAX;
							ctx->player->target_idx = UINT16_MAX;
						}
						else
						{
							ctx->player->monster_fov_selected_idx += 1;
							if(ctx->player->monster_fov_selected_idx >= ctx->player->monster_fov_idx)
							{
								ctx->player->target_idx = TARGET_MYSELF;
								ctx->player->monster_fov_selected_idx = UINT8_MAX;
							}
							else
							{
								ctx->player->target_idx = ctx->player->monster_fov[ctx->player->monster_fov_selected_idx];
							}
						}

						toggle_swap_draw_to_backbuffer();
						draw_los(ctx, ctx->draw_row, ctx->draw_col, ctx->player->stats[eStat_DeepVision]);
						draw_me(ctx);
						toggle_swap_draw_to_backbuffer();
						restore_from_backbuffer();
					}
					else
					{
						// can target me
						if(ctx->player->target_idx == TARGET_MYSELF)
						{
							ctx->player->target_idx = UINT16_MAX;
						}
						else
						{
							ctx->player->target_idx = TARGET_MYSELF;
						}

						//ctx->player->target_idx = UINT16_MAX;
						ctx->player->monster_fov_selected_idx = UINT8_MAX;

						toggle_swap_draw_to_backbuffer();
						draw_me(ctx);
						toggle_swap_draw_to_backbuffer();
						restore_from_backbuffer();
					}
					break;

				case eKey_Fire:
					// fire on target!
					if(ctx->player->target_idx != UINT16_MAX)
					{
						redraw = FireOnTarget(ctx);
					}
					else
					{
						//add_msg(ctx, "No target selected.");
						redraw = true;
					}
					break;

				case eKey_LoadOutToggle:
					SwapLoadout(ctx, false);
					ctx->redraw_status = true;
					break;

				case eKey_Save:
					save_to_backbuffer();
					screen_line_clear(screen_get_height() - 1, 7);
					SaveGame(ctx);
					restore_from_backbuffer();
					quit = true;
					break;

				case eKey_Pickup:
					DoPickup(ctx, ctx->locn.row, ctx->locn.col, true);
					break;

				default:
					//quit = true;
					break;
			}

			// if we dead.. we dead... dont keep playing
			if(ctx->player->life == 0)
			{
				quit = true;
			}
		}
		else
		{
			// death by monster
			quit = true;
		}
	}
}


void main_screen(uContext *ctx)
{
	uint16_t key;
	bool quit = false;
	char *fn;
	bool has_core = false;

	fn = GetConfigurationFile(FILE_CORE);
	has_core = verify_file(fn);
	free(fn);

	while(quit == false)
	{
		toggle_swap_draw_to_backbuffer();

		screen_clear(0x7);
		screen_line_clear(0, 0x70);
		msg(0, 0, 0x70, " Underdark %s %s - A roguelike by Bloody Cactus ", VersionString(), REL_STR);

		msg_hl(4, 2, HL_CLR, "Welcome to the `Underdark`.");
		msg_hl(5, 2, HL_CLR, "See how deep you can go and what secrets you can uncover.");

		if(has_core == true)
		{
			msg_hl(8, 4, HL_CLR, "`C`ontinue saved game");
		}

		msg_hl(9, 4, HL_CLR, "`S`tart a new game as `%s` the %s %s", ctx->player->name, ascii_race_list[ctx->player->race], ascii_roles[ctx->player->counters[0].rounds]);

		msg_hl(12, 4, HL_CLR, "`H`ighscore table");
		msg_hl(13, 4, HL_CLR, "Read the `N`ews");
		msg_hl(14, 4, HL_CLR, "Read the `W`elcome (Read this first)");

		msg_hl(16, 4, HL_CLR, "`K`eyboard Configuration + Options");

		msg_hl(18, 4, HL_CLR, "`Q`uit");

		if(has_core == true)
		{
			msg_hl(20, 4, HL_CLR, "Select [`cshnwkq`]");
		}
		else
		{
			msg_hl(20, 4, HL_CLR, "Select [`shnwkq`]");
		}


		for(key = 0; key < eKey_MAX; key++)
		{
			if(ctx->keys[key] == 0)
			{
				msg(21, 4, 0x0E, "Warning! Some game keys are not configured. Please edit configuration.");
				key = eKey_MAX;
			}
		}

		toggle_swap_draw_to_backbuffer();
		restore_from_backbuffer();

		key = gkey("cCsShHnNwWkKqQ");

		switch(key)
		{
			case 'k':
			case 'K':
				ConfigureKeys(ctx);
				break;

			case 'q':
			case 'Q':
				quit = true;
				break;

			case 'h':
			case 'H':
				DisplayScores();
				break;

			case 'n':
			case 'N':
				DisplayFixedFile("news.txt", "The Underdark Gazette");
				break;

			case 'w':
			case 'W':
				DisplayFixedFile("welcome.txt", "New Player Info");
				break;

			case 's':
			case 'S':
				ClearSaveFiles();
				NewGame(ctx);
				ContinueGame(ctx);
				quit = true;
				break;

			case 'c':
			case 'C':
				if(has_core == true)
				{
					screen_line_clear(screen_get_height() - 1, 7);
					if(LoadGame(ctx) == true)
					{
						ContinueGame(ctx);
					}
					quit = true;
				}
				break;
		}
	}
}
