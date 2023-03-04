#include "headers.h"

char *ascii_stat_names[] = {"Strength", "Speed", "Dexterity", "Willpower", "DeepVision"};
char *ascii_skill_names[] = {"Meelee", "Ranged", "Magic", "Item Lore"};

void show_inventory(uContext *ctx)
{
	uint16_t i;
	uint16_t row;
	uint16_t col;
	uItem *itm;

	char buff_count[32];
	char buff_worn_wield[32];
	char buff_cursed[32];

	bool quit;
	char *kstring;
	uKeyGet keys;

	quit = false;

	// covers our race length as well.
	col = screen_get_width() - 22;

	while(quit == false)
	{
		toggle_swap_draw_to_backbuffer();

		screen_clear(0x7);
		screen_line_clear(0, 0x70);
		msg(0, 2, 0x70, "Inventory - Loadout %u", 1 + ctx->player->loadout);
		msg(0, screen_get_width() - (1 + MAX_NAME_LEN), 0x70, "%s", ctx->player->name);

		//MAX_INVENTORY_ITEMS is < 'z'!

		row = 2;
		KeyGetInit(&keys);

		msg_hl(row, col, HL_CLR, "     `Race`: %s", ascii_race_list[ctx->player->race]);
		row += 1;
		msg_hl(row, col, HL_CLR, "     `Life`: %"PRIi16"/%"PRIi16"", ctx->player->life, ctx->player->max_life);
		row += 1;
		msg_hl(row, col, HL_CLR, "     `Mana`: %"PRIi16"/%"PRIi16"", ctx->player->mana, ctx->player->max_mana);
		row += 1;
		msg_hl(row, col, HL_CLR, "   `Armour`: %"PRIi16"", ctx->player->armour);

		row += 1;
		row += 1;

		msg_hl(row, col, HL_CLR, "Attributes");
		row += 1;
		msg_hl(row, col, HL_CLR, " `Strength`: %"PRIi16"", ctx->player->stats[eStat_Strength]);
		row += 1;
		msg_hl(row, col, HL_CLR, "    `Speed`: %"PRIi16"", ctx->player->stats[eStat_Speed]);
		row += 1;
		msg_hl(row, col, HL_CLR, "`Willpower`: %"PRIi16"", ctx->player->stats[eStat_Willpower]);
		row += 1;
		msg_hl(row, col, HL_CLR, "`Dexterity`: %"PRIi16"", ctx->player->stats[eStat_Dexterity]);


		row += 1;
		row += 1;
		msg_hl(row, col, HL_CLR, "    Skills");
		row += 1;
		msg_hl(row, col, HL_CLR, "    `Melee`: %"PRIu8"", ctx->player->skills[eSkill_MeleeWeapons]);
		row += 1;
		msg_hl(row, col, HL_CLR, "   `Ranged`: %"PRIu8"", ctx->player->skills[eSkill_RangedWeapons]);
		row += 1;
		msg_hl(row, col, HL_CLR, "    `Magic`: %"PRIu8"", ctx->player->skills[eSkill_Magic]);
		row += 1;
		msg_hl(row, col, HL_CLR, "`Item Lore`: %"PRIu8"", ctx->player->skills[eSkill_ItemLore]);

		row = 2;
		for(i = 0; i < MAX_INVENTORY_ITEMS; i++)
		{
			//uItem *item = FindItemByID(ctx, ctx->player->items[i].item_idx);

			uItem *item = &ctx->player->items[i].item;

			if(item->idx != 0)
			{
				buff_count[0] = 0;
				buff_worn_wield[0] = 0;
				buff_cursed[0] = 0;

				if(ctx->player->items[i].count > 1)
				{
					sprintf(buff_count, "(%u) ", ctx->player->items[i].count);
				}

				if(ctx->player->item_head == i)
				{
					sprintf(buff_worn_wield, "(Worn) ");
				}
				if(ctx->player->item_body == i)
				{
					sprintf(buff_worn_wield, "(Worn) ");
				}
				if(ctx->player->item_left_hand == i)
				{
					sprintf(buff_worn_wield, "(Worn) ");
				}
				if(ctx->player->item_right_hand == i)
				{
					sprintf(buff_worn_wield, "(Worn) ");
				}

				if(ctx->player->item_left_weap == i && item->base_type == eIT_2h_Weapon)
				{
					sprintf(buff_worn_wield, "(Wielding Two Handed) ");
				}
				else if(ctx->player->item_left_weap == i && item->base_type != eIT_Torch)
				{
					sprintf(buff_worn_wield, "(Wielding) ");
				}
				else if(ctx->player->item_left_weap == i && item->base_type == eIT_Torch)
				{
					sprintf(buff_worn_wield, "(Holding) ");
				}

				if(ctx->player->item_right_weap == i && item->base_type != eIT_Torch)
				{
					sprintf(buff_worn_wield, "(Wielding) ");
				}
				else if(ctx->player->item_right_weap == i && item->base_type == eIT_Torch)
				{
					sprintf(buff_worn_wield, "(Holding) ");
				}

				if(GetItemAttributeValue_Bool(item, eIA_b_CursedKnown, false) == true)
				{
					sprintf(buff_cursed, "(Cursed) ");
				}

				msg_hl(row, 4, HL_CLR, "`%c` - %s %s`%s``%s`", 'a' + i, GetItemName(item), buff_count, buff_worn_wield, buff_cursed);
				KeyGetAdd8(&keys, 'a' + i);
				row += 1;
			}
		}

		row += 1;

		KeyGetAdd8(&keys, 'z');
		msg_hl(row, 4, HL_CLR, "`z` - Quit");
		row += 1;

		KeyGetAdd8(&keys, 'X');
		msg_hl(row, 4, HL_CLR, "`X` - Drop something (gone forever!)");
		row += 1;

		KeyGetAdd8(&keys, 'L');
		msg_hl(row, 4, HL_CLR, "`L` - Swap to Loadout %u", 1 + (ctx->player->loadout ^ 1));
		row += 1;

		if(ctx->player->level_up_points > 0)
		{
			KeyGetAdd8(&keys, 'U');
			msg_hl(row, 4, HL_CLR, "`U` - Level up (%u)", ctx->player->level_up_points);
			row += 1;
		}

		row += 1;

		kstring = KeyGetToString(&keys);
		msg_hl(row, 4, HL_CLR, "Select item to use [%s]", kstring);
		free(kstring);
		toggle_swap_draw_to_backbuffer();
		restore_from_backbuffer();

		switch(KeyGetKey(&keys))
		{
			default:
				if(keys.value >= 'a' && keys.value < 'z')
				{
					itm = &ctx->player->items[keys.value - 'a'].item;

					switch(itm->base_type)
					{
						default:
							// unknown item type to action on
							break;

						case eIT_Consumable:
							if(Consume(ctx, itm, true) == true)
							{
								// delete item....
								DeleteInventoryItem(ctx, itm);
							}
							else
							{
								//TODO: say something to player?
							}
							break;

							//case eIT_Torch:
							//	break;

						case eIT_Torch:
						case eIT_1h_Weapon:
						case eIT_2h_Weapon:
						case eIT_AmmunitionForBow:
						case eIT_Ring:
						case eIT_Shield:
						case eIT_BodyArmour:
						case eIT_Helmet:
							if(IsAlreadyEquipped(ctx, itm) == true)
							{
								// action on an item.
								if(UnEquipItem(ctx, itm, true, false) == eReturn_ItemCursed)
								{
									toggle_swap_draw_to_backbuffer();
									msg(screen_get_height() - 1, 2, 0x0E, "Item was CURSED! Cant be removed. Press any key");
									toggle_swap_draw_to_backbuffer();
									restore_from_backbuffer();
									gkey(NULL);
								}
							}
							else if(IsAlreadyUnEquipped(ctx, itm) == true)
							{
								i = EquipItem(ctx, itm, true);
								switch(i)
								{
									case eReturn_DontMeetStatRequirement_Strength:
									case eReturn_DontMeetStatRequirement_Speed:
									case eReturn_DontMeetStatRequirement_Dexterity:
									case eReturn_DontMeetStatRequirement_Willpower:
									case eReturn_DontMeetStatRequirement_DeepVision:
										toggle_swap_draw_to_backbuffer();
										msg(screen_get_height() - 1, 2, 0x0E, "You dont meet the %s requirements. Press any key", ascii_stat_names[i - eReturn_DontMeetStatRequirement_Strength]);
										toggle_swap_draw_to_backbuffer();
										restore_from_backbuffer();
										gkey(NULL);
										break;

									case eReturn_DontMeetSkillRequirement_Meelee:
									case eReturn_DontMeetSkillRequirement_Ranged:
									case eReturn_DontMeetSkillRequirement_Magic:
									case eReturn_DontMeetSkillRequirement_ItemLore:
										toggle_swap_draw_to_backbuffer();
										msg(screen_get_height() - 1, 2, 0x0E, "You dont meet the %s requirements. Press any key", ascii_skill_names[i - eReturn_DontMeetSkillRequirement_Meelee]);
										toggle_swap_draw_to_backbuffer();
										restore_from_backbuffer();
										gkey(NULL);
										break;

									case eReturn_ItemTypeAlreadyEquipped:
										toggle_swap_draw_to_backbuffer();
										msg(screen_get_height() - 1, 2, 0x0E, "You already have something of that type equipped. Press any key");
										toggle_swap_draw_to_backbuffer();
										restore_from_backbuffer();
										gkey(NULL);
										break;

									case eReturn_NoEmptyItemSlot:
										toggle_swap_draw_to_backbuffer();
										msg(screen_get_height() - 1, 2, 0x0E, "No empty slots to equip that. Press any key");
										toggle_swap_draw_to_backbuffer();
										restore_from_backbuffer();
										gkey(NULL);
										break;

									case eReturn_AlreadyEquipped:
									case eReturn_Bad:
										toggle_swap_draw_to_backbuffer();
										msg(screen_get_height() - 1, 2, 0x0E, "Already equipped (You should not get here!!). Press any key");
										toggle_swap_draw_to_backbuffer();
										restore_from_backbuffer();
										gkey(NULL);
										break;

									case eReturn_OK:
										break;
								}
							}
							break;

					}
				}
				break;
				// quit
			case 'z':
				quit = true;
				break;
			case 'U':
				DoLevelUp(ctx);
				break;
			case 'L':
				SwapLoadout(ctx, true);
				break;
			case 'X':
			{
				// clear from back
				for(i = 0; i < keys.k8_idx; i++)
				{
					if(keys.keys8[i] < 'a' || keys.keys8[i] >= 'z')
					{
						keys.keys8[i] = 0;
					}
				}

				KeyGetAdd16(&keys, DOS_KEY_ESCAPE);
				kstring = KeyGetToString(&keys);
				toggle_swap_draw_to_backbuffer();
				msg_hl(screen_get_height() - 1, 2, HL_CLR, "Press key [`%s`] to select item to drop. (ESC to cancel)", kstring);
				free(kstring);
				toggle_swap_draw_to_backbuffer();
				restore_from_backbuffer();
				KeyGetKey(&keys);


				itm = &ctx->player->items[keys.value - 'a'].item;
				// TODO: drop stuff!
				if(IsAlreadyEquipped(ctx, itm) == true)
				{
					// action on an item.
					if(UnEquipItem(ctx, itm, true, false) == eReturn_ItemCursed)
					{
						toggle_swap_draw_to_backbuffer();
						msg(screen_get_height() - 1, 2, 0x0E, "Item was CURSED! Cant be removed. Press any key");
						toggle_swap_draw_to_backbuffer();
						restore_from_backbuffer();
						gkey(NULL);
					}
				}

				// if we get here, its not equipped (anymore).
				DeleteInventoryItem(ctx, itm);
			}
				break;
		}

		// life can change after we consume something or do something with items!!
		if(ctx->player->life == 0)
		{
			quit = true;
		}
	}
}
