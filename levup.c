#include "headers.h"


// =INT(((F1*$D$1)/$C$1)+F1), d1=3, c1=2
// f1=600, d1=3, c1=2
//uint16_t experience_table[10]={ 600,1000,1666,2776,4626,7710,12850,21416,35693,59488 };
// f1=650, d1=3, c1=2
uint16_t experience_table[10]={ 0,1050,1750,2916,4860,8100,13500,22500,37500,62500 };


void DoLevelUp(uContext *ctx)
{
	bool quit = false;
	uint16_t key;

	while(quit == false)
	{
		toggle_swap_draw_to_backbuffer();

		screen_clear(0x7);

		screen_line_clear(0, 0x70);
		msg(0,2, 0x70, "Level Up");

		msg_hl(2,2, HL_CLR, "You have `%u` points you can spend.", ctx->player->level_up_points);
		msg_hl(3,2, HL_CLR, "Allocated points are FINAL, there is no undo.");
		msg_hl(5,2, HL_CLR, "Max level of a stat or skill is 10 points");

		msg_hl(7,2, HL_CLR, "`a` - %u Strength", ctx->player->base_stats[eStat_Strength]);
		msg_hl(8,2, HL_CLR, "`b` - %u Speed", ctx->player->base_stats[eStat_Speed]);
		msg_hl(9,2, HL_CLR, "`c` - %u Dexterity", ctx->player->base_stats[eStat_Dexterity]);
		msg_hl(10,2, HL_CLR, "`d` - %u Willpower", ctx->player->base_stats[eStat_Willpower]);

		msg_hl(12,2, HL_CLR, "`e` - %u Meelee Weapons", ctx->player->skills[eSkill_MeleeWeapons]);
		msg_hl(13,2, HL_CLR, "`f` - %u Ranged Weapons", ctx->player->skills[eSkill_RangedWeapons]);
		msg_hl(14,2, HL_CLR, "`g` - %u Magic", ctx->player->skills[eSkill_Magic]);
		msg_hl(15,2, HL_CLR, "`h` - %u Item Lore", ctx->player->skills[eSkill_ItemLore]);

		msg_hl(17,2, HL_CLR, "`1` - Convert a point into extra hit points");

		msg_hl(19,2, HL_CLR, "`z` to quit");

		toggle_swap_draw_to_backbuffer();
		restore_from_backbuffer();

		if(ctx->player->level_up_points > 0)
			key = gkey("abcdefghz1");
		else
			key = gkey("z");

		switch(key)
		{
			case '1':
				key = 10 + xrnd(40);
				ctx->player->life += key;
				ctx->player->max_life += key;
				ctx->player->level_up_points -= 1;
				toggle_swap_draw_to_backbuffer();
				msg_hl(22, 2, HL_CLR, "You gained `%u` life. Press any key to continue", key);
				toggle_swap_draw_to_backbuffer();
				restore_from_backbuffer();
				gkey(NULL);
				break;

			case 'a':
			case 'b':
			case 'c':
			case 'd':
				if(ctx->player->base_stats[key-'a'] < 10)
				{
					ctx->player->stats[key-'a'] += 1;
					ctx->player->base_stats[key-'a'] += 1;
					ctx->player->level_up_points -= 1;
				}
				break;

			case 'e':
			case 'f':
			case 'g':
			case 'h':
				if(ctx->player->skills[key-'e'] < 10)
				{
					ctx->player->skills[key-'e'] += 1;
					ctx->player->level_up_points -= 1;
				}
				break;

			case 'z':
				quit = 1;
				break;
		}
	}
}


void AddExperiece(uContext *ctx, uint16_t exp)
{
	uint16_t a;

	a = ctx->player->experience;
	a += exp;

	if(a < ctx->player->experience)
	{
		ctx->player->experience = UINT16_MAX;
	}
	else
	{
		ctx->player->experience += exp;
	}

	if(ctx->player->level < 10)
	{
		if( experience_table[ctx->player->level] < ctx->player->experience)
		{
			// give XP as score bonus
			ctx->player->score += ctx->player->experience;

			// reset xp
			ctx->player->experience = 0;

			// add level
			ctx->player->level += 1;

			// add life!
			a = 10 + xrnd(10);
			ctx->player->life += a;
			ctx->player->max_life += a;

			// add mana!
			a = 10 + xrnd(10);
			ctx->player->mana += a;
			ctx->player->max_mana += a;

			// level up points
			ctx->player->level_up_points += 1;

			add_msg(ctx, "You have leveled up! You are now level %"PRIu8". You gained a skill point.", ctx->player->level);

			ctx->redraw_status = true;
		}
	}
}
