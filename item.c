#include "headers.h"

const char *fortunes[] =
	{
		//   dont go over 75 characters
		//   000000000|000000000|000000000|000000000|000000000|000000000|000000000|00000
		"Don't eat the paper.",
		"This cookie contains no calories.",
		"Shopkeepers always inflate prices.",
		"Some fortune cookies contain no fortune.",
		"The fortune you seek is in another cookie.",
		"Some dream of fortunes, others dream of cookies.",
		"Fortune not found? Abort, Retry, Ignore.",
		"The early bird gets the worm, but the second mouse gets the cheese",
		"Your reality check is about to bounce.",
		"A foolish one listens to his heart. A wise one listens to cookies",
		"Don't worry about money. The best things in life are free.",
		"Your lucky number is 12",
		"You pathetic descendant of monkeys!",
		"Help! I am being held prisoner in a fortune cookie factory.",
		NULL
	};

const char *ascii_potion1[] =
	{
		"cloudy",
		"clear",
		"vibrant",
		"murky",
		"dull",
		"thick",
		"foul",
		"sweet",
		"wispy",
		"sparkly",
		NULL
	};

const char *ascii_potion2[] =
	{
		"red",
		"orange",
		"blue",
		"green",
		"pink",
		"purple",
		"yellow",
		"white",
		"black",
		"almond",
		"bronze",
		"lilac",
		"rose",
		"olive",
		"brown",
		"crimson",
		"violet",
		"gold",
		"silver",
		"gray",

		NULL
	};

const char *ascii_food1[] =
	{
		"chunky",
		"slimey",
		"diced",
		"cubed",
		"stewed",
		"roasted",
		"grilled",
		"pickled",
		"preserved",
		NULL
	};

const char *ascii_food2[] =
	{
		"marshmallows",
		"buttons",
		"nose shavings",
		"woodlice",
		"toe fungus",
		"intestines",
		"elbow bits",
		"ear wax",
		"great worm",
		"eyeballs",
		NULL
	};

char __item_name[64];

char *GetItemName(uItem *i)
{
	uint16_t q;
	if(GetItemAttributeValue_Bool(i, eIA_b_Unknown, false) == true)
	{
		// we dont know its name.
		switch(i->base_type)
		{
			case eIT_Consumable:
				if(GetItemAttributeValue_Int(i, eIA_i_PotionBase, INT16_MAX) != INT16_MAX)
				{
					// it has a potion base
					for(q = 0; q < MAX_ITEM_ATTRIBUTES; q++)
					{
						// test for a potion name
						if(i->attr[q].attr_type == eAttr_Key_i16Value)
						{
							if(i->attr[q].type == eIA_kv_PotionRandomName)
							{
								sprintf(__item_name, "%s %s potion", ascii_potion1[i->attr[q].attributes.kv.key], ascii_potion2[i->attr[q].attributes.kv.value]);
								return (char *)__item_name;
							}
						}
					}

					// unknown food or potion base
					sprintf(__item_name, "unknown potion");
					return (char *)__item_name;
				}
				else
				{
					if(GetItemAttributeValue_Int(i, eIA_i_FoodBase, INT16_MAX) != INT16_MAX)
					{
						// its not a drink
						for(q = 0; q < MAX_ITEM_ATTRIBUTES; q++)
						{
							// test for a potion name
							if(i->attr[q].attr_type == eAttr_Key_i16Value)
							{
								if(i->attr[q].type == eIA_kv_FoodRandomName)
								{
									sprintf(__item_name, "can of %s %s", ascii_food1[i->attr[q].attributes.kv.key], ascii_food2[i->attr[q].attributes.kv.value]);
									return (char *)__item_name;
								}
							}
						}

						// unknown food or potion base
						sprintf(__item_name, "unknown food item");
						return (char *)__item_name;
					}
					else
					{
						// unknown food or potion base
						sprintf(__item_name, "unknown consumable");
						return (char *)__item_name;
					}
				}
				break;

			default:
				sprintf(__item_name, "unknown item");
				return (char *)__item_name;
				break;
		}
	}

	return (char *)i->name;
}

void FreeItem(void *x)
{
	uItem *i = (uItem *)x;

	if(i == NULL)
	{
		return;
	}

	memset(i, 0x0, sizeof(uItem));
	free(i);
}

static uint16_t ScanForFreeIndex(uItem *i)
{
	uint16_t q;

	for(q = 0; q < MAX_ITEM_ATTRIBUTES; q++)
	{
		if(i->attr[q].type == eIA_None)
		{
			return q;
		}
	}

	return UINT16_MAX;
}


uAttribute *GetItemAttribute(uItem *i, uint16_t type)
{
	uint16_t j;

	if(i == NULL)
	{
		return NULL;
	}

	for(j = 0; j < MAX_ITEM_ATTRIBUTES; j++)
	{
		if(i->attr[j].type == type)
		{
			return &i->attr[j];
		}
	}

	return NULL;
}

uint16_t AddItemAttribute_Range(uItem *i, uint16_t type, int16_t min, int16_t max)
{
	uint16_t idx = ScanForFreeIndex(i);
	if(idx != UINT16_MAX)
	{
		i->attr[idx].type = type;
		i->attr[idx].modified = false;
		i->attr[idx].attr_type = eAttr_Range;
		i->attr[idx].attributes.r.min = min;
		i->attr[idx].attributes.r.max = max;
	}
	return idx;
}

uint16_t AddItemAttribute_Die(uItem *i, uint16_t type, int8_t num, int8_t faces, int8_t plus)
{
	uint16_t idx = ScanForFreeIndex(i);
	if(idx != UINT16_MAX)
	{
		i->attr[idx].type = type;
		i->attr[idx].modified = false;
		i->attr[idx].attr_type = eAttr_Dice;
		i->attr[idx].attributes.die.num = num;
		i->attr[idx].attributes.die.faces = faces;
		i->attr[idx].attributes.die.plus = plus;
	}

	return idx;
}

int16_t GetItemAttributeValue_Die(uItem *i, uint16_t type, int16_t def)
{
	uAttribute *a;

	if(i == NULL)
	{
		return def;
	}

	a = GetItemAttribute(i, type);

	if(a != NULL)
	{
		return roll_dice(a->attributes.die.num, a->attributes.die.faces, a->attributes.die.plus);
	}

	return def;
}

uint16_t AddItemAttribute_KV(uItem *i, uint16_t type, uint16_t key, int16_t value)
{
	uint16_t idx = ScanForFreeIndex(i);
	if(idx != UINT16_MAX)
	{
		i->attr[idx].type = type;
		i->attr[idx].modified = false;
		i->attr[idx].attr_type = eAttr_Key_i16Value;
		i->attr[idx].attributes.kv.key = key;
		i->attr[idx].attributes.kv.value = value;
	}

	return idx;
}

uAttribute *GetItemAttributeValue_KV_attribute(uItem *i, uint16_t type)
{
	uint16_t j;

	if(i == NULL)
	{
		return NULL;
	}

	for(j = 0; j < MAX_ITEM_ATTRIBUTES; j++)
	{
		if(i->attr[j].type == type)
		{
			return &i->attr[j];
		}
	}

	return NULL;
}

int16_t GetItemAttributeValue_KV(uItem *i, uint16_t type, uint16_t key, int16_t def)
{
	uint16_t j;

	if(i == NULL)
	{
		return def;
	}

	for(j = 0; j < MAX_ITEM_ATTRIBUTES; j++)
	{
		if(i->attr[j].type == type)
		{
			if(i->attr[j].attributes.kv.key == key)
			{
				return i->attr[j].attributes.kv.value;
			}
		}
	}

	return def;
}

uint16_t AddItemAttribute_Bool(uItem *i, uint16_t type, bool value)
{
	uint16_t idx = ScanForFreeIndex(i);
	if(idx != UINT16_MAX)
	{
		i->attr[idx].type = type;
		i->attr[idx].modified = false;
		i->attr[idx].attr_type = eAttr_Bool;
		i->attr[idx].attributes.b.value = value;
	}
	return idx;
}

void SetItemAttributeValue_Bool(uItem *i, uint16_t type, bool value)
{
	uAttribute *a = GetItemAttribute(i, type);
	if(a != NULL)
	{
		//changing an existing attribute
		a->attributes.b.value = value;
		a->modified = true;
	}
	else
	{
		AddItemAttribute_Bool(i, type, value);
	}
}

bool GetItemAttributeValue_Bool(uItem *i, uint16_t type, bool def)
{
	uAttribute *a;

	if(i == NULL)
	{
		return def;
	}

	a = GetItemAttribute(i, type);

	if(a != NULL)
	{
		return a->attributes.b.value;
	}

	return def;
}

uint16_t AddItemAttribute_Int(uItem *i, uint16_t type, int16_t value)
{
	uint16_t idx = ScanForFreeIndex(i);
	if(idx != UINT16_MAX)
	{
		i->attr[idx].type = type;
		i->attr[idx].modified = false;
		i->attr[idx].attr_type = eAttr_i16Value;
		i->attr[idx].attributes.i.value = value;
	}
	return idx;
}

void SetItemAttributeValue_Int(uItem *i, uint16_t type, int16_t value)
{
	uAttribute *a = GetItemAttribute(i, type);
	if(a != NULL)
	{
		a->attributes.i.value = value;
		a->modified = true;
	}
	else
	{
		AddItemAttribute_Int(i, type, value);
	}
}

int16_t GetItemAttributeValue_Int(uItem *i, uint16_t type, int16_t def)
{
	uAttribute *a;

	if(i == NULL)
	{
		return def;
	}

	a = GetItemAttribute(i, type);
	if(a != NULL)
	{
		return a->attributes.i.value;
	}

	return def;
}

void SetItemName(uItem *item, char *name)
{
	strncpy((char *)item->name, name, MAX_ITEM_NAME);
}

uItem *AddItem(uContext *ctx, uint8_t base_type, char *name)
{
	uItem *i;
	uint16_t q;

	i = (uItem *)calloc(1, sizeof(uItem));
	assert(i != NULL);

	for(q = 0; q < MAX_ITEM_ATTRIBUTES; q++)
	{
		i->attr[q].type = eIA_None;
		i->attr[q].modified = false;
	}

	i->idx = 1 + dlist_size(ctx->lstItems);
	i->base_type = base_type;

	dlist_ins(ctx->lstItems, i);

	if(name != NULL)
	{
		SetItemName(i, name);
	}

	return i;
}

uItem *FindItemByName(uContext *ctx, char *name)
{
	DLElement *e;

	e = dlist_head(ctx->lstItems);
	while(e != NULL)
	{
		uItem *i = dlist_data(e);
		e = dlist_next(e);

		if(i->name[0] != 0)
		{
			if(stricmp((char *)i->name, name) == 0)
			{
				return i;
			}
		}
	}

	return NULL;
}

uint16_t FindItemSlotInInventoryByID(uContext *ctx, uint16_t id)
{
	int i;

	for(i = 0; i < MAX_INVENTORY_ITEMS; i++)
	{
		if(ctx->player->items[i].item.idx == id)
		{
			return i;
		}
	}

	return UINT16_MAX;
}

uItem *FindItemByID(uContext *ctx, uint16_t id)
{
	DLElement *e;

	if(id == 0)
	{
		return NULL;
	}

	e = dlist_head(ctx->lstItems);
	while(e != NULL)
	{
		uItem *i = dlist_data(e);
		e = dlist_next(e);

		if(i->idx == id)
		{
			return i;
		}
	}

	return NULL;
}

static uint16_t GenerateName(uContext *ctx, uItem *i, int16_t type, const char **name1, const char **name2)
{
	uint16_t p1, p2;
	uint16_t a, b;
	uint16_t j;

	p1 = 0;
	while(name1[p1] != 0)
	{
		p1 += 1;
	}

	p2 = 0;
	while(name2[p2] != 0)
	{
		p2 += 1;
	}

	for(j = 0; j < 100; j++)
	{
		DLElement *e;
		uItem *item2;
		bool found;

		a = xrnd(p1);
		b = xrnd(p2);

		found = false;
		e = dlist_head(ctx->lstItems);
		while(e != NULL && found == false)
		{
			int16_t bb;
			item2 = dlist_data(e);
			e = dlist_next(e);

			bb = GetItemAttributeValue_KV(item2, type, a, INT16_MAX);
			// aa = a AND bb == b
			if(bb == b)
			{
				// matching name. make a new name
				found = true;
			}
		}

		if(found == false)
		{
			uint16_t idx;
			idx = AddItemAttribute_KV(i, type, a, b);
			// mark the name so it gets modified + saved
			i->attr[idx].modified = true;

			return 0;
		}
	}

	printf("Failed to create unique name\n");
	return 1;
}


static uint16_t GenerateFoodName(uContext *ctx, uItem *i)
{
	return GenerateName(ctx, i, eIA_kv_FoodRandomName, ascii_food1, ascii_food2);
}

static uint16_t GeneratePotionName(uContext *ctx, uItem *i)
{
	return GenerateName(ctx, i, eIA_kv_PotionRandomName, ascii_potion1, ascii_potion2);
}

static void AddUnknown(uItem *i, int16_t skill_level)
{
	SetItemAttributeValue_Int(i, eIA_i_IdentifySkillLevel, skill_level);
	SetItemAttributeValue_Bool(i, eIA_b_Unknown, true);
}

static uint16_t GenPotion(uContext *ctx, uItem *i, int16_t skill_level)
{
	AddUnknown(i, skill_level);
	return GeneratePotionName(ctx, i);
}

static uint16_t GenFood(uContext *ctx, uItem *i, int16_t skill_level)
{
	AddUnknown(i, skill_level);
	return GenerateFoodName(ctx, i);
}

uint16_t BuildItems(uContext *ctx)
{
	uItem *i;

	// weapon ID creation should not change. anything added, adds at the bottom

	/////////////////////////////////////////////////////////////////////////////////////////////
	// knives have no associated skill. anyone can use a knife

	// low knives
	i = AddItem(ctx, eIT_1h_Weapon, ITEM_NAME_SKINNING_KNIFE);
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_WEAPON);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 1, 3, 0);

	i = AddItem(ctx, eIT_1h_Weapon, "pairing knife");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_WEAPON);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 1, 3, 0);

	i = AddItem(ctx, eIT_1h_Weapon, ITEM_NAME_TANTO_KNIFE);
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_WEAPON);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 1, 4, 0);

	// mid knives
	i = AddItem(ctx, eIT_1h_Weapon, "long knife");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_M_WEAPON | GEARDROP_L_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_MeleeWeapons, 3);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 2, 6, 0);

	i = AddItem(ctx, eIT_1h_Weapon, "wicked knife");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_M_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_MeleeWeapons, 3);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 1, 12, 1);

	i = AddItem(ctx, eIT_1h_Weapon, "prison shiv");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_M_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_MeleeWeapons, 3);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 3, 4, 2);

	i = AddItem(ctx, eIT_1h_Weapon, "razor knife");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_M_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_MeleeWeapons, 3);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 2, 6, 0);


	// high knife
	i = AddItem(ctx, eIT_1h_Weapon, "pig sticker +6/pigs");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_H_WEAPON | GEARDROP_M_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_MeleeWeapons, 5);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 3, 7, 0);

	i = AddItem(ctx, eIT_1h_Weapon, "sharpened tibia");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_H_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_MeleeWeapons, 5);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 2, 12, 2);

	i = AddItem(ctx, eIT_1h_Weapon, "stiletto razor");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_H_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_MeleeWeapons, 5);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 1, 20, 5);

	i = AddItem(ctx, eIT_1h_Weapon, "straight razor");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_H_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_MeleeWeapons, 5);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 2, 10, 4);

	i = AddItem(ctx, eIT_1h_Weapon, "pointy wooden stake");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_H_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_MeleeWeapons, 5);
	AddItemAttribute_KV(i, eIA_kv_VsMonsterType, eMT_Vampire, 10);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 1, 6, 1);

	////////////////////////////////////////////////////////////////////////////////////////////
	// swords require meelee skill

	// low weapons
	i = AddItem(ctx, eIT_1h_Weapon, ITEM_NAME_SHORT_SWORD);
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_MeleeWeapons, 1);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 1, 4, 0);

	i = AddItem(ctx, eIT_1h_Weapon, ITEM_NAME_LONG_SWORD);
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_MeleeWeapons, 1);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 1, 6, 0);

	// named item - only 1 ever exists? unique?
	// named item (Sting) +4 vs spiders
	i = AddItem(ctx, eIT_1h_Weapon, "short sword 'sting' +4/spiders");
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_MeleeWeapons, 1);
	SetItemAttributeValue_Bool(i, eIA_b_Named, true);
	SetItemAttributeValue_Bool(i, eIA_b_Unique, true);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 1, 5, 0);
	AddItemAttribute_KV(i, eIA_kv_VsMonsterType, eMT_Spider, 4);

	i = AddItem(ctx, eIT_1h_Weapon, ITEM_NAME_AXE);
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_MeleeWeapons, 1);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 1, 5, 0);

	i = AddItem(ctx, eIT_1h_Weapon, "axe 'vermin' +1/rats");
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_MeleeWeapons, 1);
	SetItemAttributeValue_Bool(i, eIA_b_Named, true);
	SetItemAttributeValue_Bool(i, eIA_b_Unique, true);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 1, 5, 0);
	AddItemAttribute_KV(i, eIA_kv_VsMonsterType, eMT_Rat, 1);

	i = AddItem(ctx, eIT_1h_Weapon, "axe 'tide' +1/rats");
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_MeleeWeapons, 1);
	SetItemAttributeValue_Bool(i, eIA_b_Named, true);
	SetItemAttributeValue_Bool(i, eIA_b_Unique, true);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 1, 5, 0);
	AddItemAttribute_KV(i, eIA_kv_VsMonsterType, eMT_Rat, 1);

	i = AddItem(ctx, eIT_1h_Weapon, "long sword +2");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_MeleeWeapons, 2);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 1, 6, 2);

	// mid weapons
	i = AddItem(ctx, eIT_1h_Weapon, "well honed long sword");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_WEAPON | GEARDROP_M_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_MeleeWeapons, 4);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 1, 8, 4);

	i = AddItem(ctx, eIT_1h_Weapon, "falchon");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_M_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_MeleeWeapons, 4);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 2, 6, 3);

	i = AddItem(ctx, eIT_1h_Weapon, "falchon +2");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_M_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_MeleeWeapons, 4);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 2, 6, 5);

	i = AddItem(ctx, eIT_1h_Weapon, "etched sword 'the bleeder'");
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_MeleeWeapons, 4);
	SetItemAttributeValue_Bool(i, eIA_b_Named, true);
	SetItemAttributeValue_Bool(i, eIA_b_Unique, true);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 2, 6, 5);
	AddItemAttribute_KV(i, eIA_kv_VsMonsterType, eMT_Human, 6);

	i = AddItem(ctx, eIT_1h_Weapon, "battle tested axe");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_M_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_MeleeWeapons, 3);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 1, 6, 3);

	i = AddItem(ctx, eIT_1h_Weapon, "longsword 'fang' +6/snakes");
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_MeleeWeapons, 4);
	SetItemAttributeValue_Bool(i, eIA_b_Named, true);
	SetItemAttributeValue_Bool(i, eIA_b_Unique, true);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 2, 6, 2);
	AddItemAttribute_KV(i, eIA_kv_VsMonsterType, eMT_Snake, 6);


	// high weapons
	i = AddItem(ctx, eIT_1h_Weapon, "long sword (very sharp)");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_H_WEAPON | GEARDROP_M_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_MeleeWeapons, 6);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 2, 6, 6);

	i = AddItem(ctx, eIT_1h_Weapon, "pirates cutlass");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_H_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_MeleeWeapons, 6);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 2, 10, 5);

	i = AddItem(ctx, eIT_1h_Weapon, "dueling rapier");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_H_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_MeleeWeapons, 6);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 2, 10, 5);

	i = AddItem(ctx, eIT_1h_Weapon, "blessed sword +2/zombies");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_H_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_MeleeWeapons, 6);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 3, 7, 2);
	AddItemAttribute_KV(i, eIA_kv_VsMonsterType, eMT_Zombie, 2);

	i = AddItem(ctx, eIT_1h_Weapon, "sharpened narwhal tusk");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_H_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_MeleeWeapons, 6);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 3, 8, 5);

	i = AddItem(ctx, eIT_1h_Weapon, "hellfire blade +4/demons");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_H_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_MeleeWeapons, 6);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 3, 8, 5);
	AddItemAttribute_KV(i, eIA_kv_VsMonsterType, eMT_Demon, 4);

	i = AddItem(ctx, eIT_1h_Weapon, "femure battle hammer");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_H_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_MeleeWeapons, 7);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 3, 8, 6);

	i = AddItem(ctx, eIT_1h_Weapon, "northern tribal axe");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_H_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_MeleeWeapons, 7);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 3, 8, 6);

	i = AddItem(ctx, eIT_1h_Weapon, "pious vestigial set mace");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_H_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_MeleeWeapons, 7);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 4, 6, 4);



	///////////////////////////////////////////////////////////////////////////////////////////
	// bows require ranged skill
	i = AddItem(ctx, eIT_2h_Weapon, ITEM_NAME_SHORT_BOW);
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_RangedWeapons, 1);
	AddItemAttribute_Die(i, eIA_d_DoRangedDamage, 1, 6, 0);
	AddItemAttribute_Bool(i, eIA_b_FireTargetBow, true);

	i = AddItem(ctx, eIT_2h_Weapon, "short hunter bow");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_RangedWeapons, 2);
	AddItemAttribute_Die(i, eIA_d_DoRangedDamage, 1, 8, 0);
	AddItemAttribute_Bool(i, eIA_b_FireTargetBow, true);

	i = AddItem(ctx, eIT_2h_Weapon, "long bow");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_RangedWeapons, 4);
	AddItemAttribute_Die(i, eIA_d_DoRangedDamage, 1, 10, 1);
	AddItemAttribute_Bool(i, eIA_b_FireTargetBow, true);

	i = AddItem(ctx, eIT_2h_Weapon, "composite bow");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_RangedWeapons, 3);
	AddItemAttribute_Die(i, eIA_d_DoRangedDamage, 1, 12, 2);
	AddItemAttribute_Bool(i, eIA_b_FireTargetBow, true);

	i = AddItem(ctx, eIT_2h_Weapon, "hunters composite bow");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_RangedWeapons, 5);
	AddItemAttribute_Die(i, eIA_d_DoRangedDamage, 1, 12, 4);
	AddItemAttribute_Bool(i, eIA_b_FireTargetBow, true);

	i = AddItem(ctx, eIT_2h_Weapon, "trueshot bow");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_M_WEAPON | GEARDROP_L_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_RangedWeapons, 5);
	AddItemAttribute_Die(i, eIA_d_DoRangedDamage, 2, 8, 4);
	AddItemAttribute_Bool(i, eIA_b_FireTargetBow, true);

	i = AddItem(ctx, eIT_2h_Weapon, "warriors bow");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_M_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_RangedWeapons, 6);
	AddItemAttribute_Die(i, eIA_d_DoRangedDamage, 2, 10, 5);
	AddItemAttribute_Bool(i, eIA_b_FireTargetBow, true);

	i = AddItem(ctx, eIT_2h_Weapon, "ancient bow");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_M_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_RangedWeapons, 6);
	AddItemAttribute_Die(i, eIA_d_DoRangedDamage, 3, 7, 5);
	AddItemAttribute_Bool(i, eIA_b_FireTargetBow, true);

	i = AddItem(ctx, eIT_2h_Weapon, "northern tribal bow");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_H_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_RangedWeapons, 7);
	AddItemAttribute_Die(i, eIA_d_DoRangedDamage, 3, 8, 5);
	AddItemAttribute_Bool(i, eIA_b_FireTargetBow, true);

	i = AddItem(ctx, eIT_2h_Weapon, "arcane bow");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_H_WEAPON);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_RangedWeapons, 7);
	AddItemAttribute_Die(i, eIA_d_DoRangedDamage, 4, 6, 5);
	AddItemAttribute_Bool(i, eIA_b_FireTargetBow, true);

	////////////////////////////////////////////////////////////////////////////////////////////
	// ammunition
	i = AddItem(ctx, eIT_AmmunitionForBow, ITEM_NAME_ARROWS);
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_WEAPON | GEARDROP_M_WEAPON);
	AddItemAttribute_Die(i, eIA_d_DoRangedDamage, 0, 0, 0);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 1, 1, 0);
	SetItemAttributeValue_Bool(i, eIA_b_Stackable, true);

	i = AddItem(ctx, eIT_AmmunitionForBow, "strong arrows");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_WEAPON | GEARDROP_M_WEAPON);
	AddItemAttribute_Die(i, eIA_d_DoRangedDamage, 0, 0, 1);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 1, 1, 1);
	SetItemAttributeValue_Bool(i, eIA_b_Stackable, true);

	i = AddItem(ctx, eIT_AmmunitionForBow, "piercing arrows");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_WEAPON | GEARDROP_M_WEAPON);
	AddItemAttribute_Die(i, eIA_d_DoRangedDamage, 0, 0, 2);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 1, 2, 1);
	SetItemAttributeValue_Bool(i, eIA_b_Stackable, true);

	i = AddItem(ctx, eIT_AmmunitionForBow, "precision arrows");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_WEAPON | GEARDROP_M_WEAPON);
	AddItemAttribute_Die(i, eIA_d_DoRangedDamage, 0, 0, 3);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 1, 2, 1); // melee damage!
	SetItemAttributeValue_Bool(i, eIA_b_Stackable, true);

	i = AddItem(ctx, eIT_AmmunitionForBow, "broadhead arrows");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_WEAPON | GEARDROP_M_WEAPON);
	AddItemAttribute_Die(i, eIA_d_DoRangedDamage, 0, 0, 5);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 1, 2, 2);
	SetItemAttributeValue_Bool(i, eIA_b_Stackable, true);

	i = AddItem(ctx, eIT_AmmunitionForBow, "deadly arrows");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_M_WEAPON | GEARDROP_H_WEAPON);
	AddItemAttribute_Die(i, eIA_d_DoRangedDamage, 0, 0, 6);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 1, 4, 4);
	SetItemAttributeValue_Bool(i, eIA_b_Stackable, true);

	i = AddItem(ctx, eIT_AmmunitionForBow, "heartseeker arrows");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_M_WEAPON | GEARDROP_H_WEAPON);
	AddItemAttribute_Die(i, eIA_d_DoRangedDamage, 0, 0, 8);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 1, 4, 4);
	SetItemAttributeValue_Bool(i, eIA_b_Stackable, true);

	i = AddItem(ctx, eIT_AmmunitionForBow, "culling arrows");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_M_WEAPON | GEARDROP_H_WEAPON);
	AddItemAttribute_Die(i, eIA_d_DoRangedDamage, 0, 0, 10);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 1, 4, 4);
	SetItemAttributeValue_Bool(i, eIA_b_Stackable, true);

	/////////////////////////////////////////////////////////////////////////////////////////////
	// armour

	// low 1-4
	i = AddItem(ctx, eIT_BodyArmour, ITEM_NAME_PADDED_ARMOUR);
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_ARMOUR);
	SetItemAttributeValue_Int(i, eIA_i_ArmourRating, 1);
	i = AddItem(ctx, eIT_BodyArmour, ITEM_NAME_LEATHER_ARMOUR);
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_ARMOUR);
	SetItemAttributeValue_Int(i, eIA_i_ArmourRating, 2);
	i = AddItem(ctx, eIT_BodyArmour, ITEM_NAME_STUDDED_ARMOUR);
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_ARMOUR);
	SetItemAttributeValue_Int(i, eIA_i_ArmourRating, 3);
	i = AddItem(ctx, eIT_BodyArmour, "hide armour");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_ARMOUR);
	SetItemAttributeValue_Int(i, eIA_i_ArmourRating, 4);
	SetItemAttributeValue_Int(i, eIA_i_StatAdjust_Speed, -1);

	i = AddItem(ctx, eIT_BodyArmour, ITEM_NAME_PURPLE_SPANDEX);
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_ARMOUR | GEARDROP_MAGIC);
	SetItemAttributeValue_Int(i, eIA_i_ManaAdjust, 2);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_Magic, 2);
	SetItemAttributeValue_Int(i, eIA_i_StatAdjust_Speed, 1);

	// mid 5-6
	i = AddItem(ctx, eIT_BodyArmour, "scale armour");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_M_ARMOUR);
	SetItemAttributeValue_Int(i, eIA_i_ArmourRating, 5);
	SetItemAttributeValue_Int(i, eIA_i_StatAdjust_Speed, -1);

	i = AddItem(ctx, eIT_BodyArmour, "scaled leather");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_M_ARMOUR);
	SetItemAttributeValue_Int(i, eIA_i_ArmourRating, 5);
	SetItemAttributeValue_Int(i, eIA_i_StatAdjust_Speed, -2);

	i = AddItem(ctx, eIT_BodyArmour, "ringmail armour");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_M_ARMOUR);
	SetItemAttributeValue_Int(i, eIA_i_ArmourRating, 6);
	SetItemAttributeValue_Int(i, eIA_i_StatAdjust_Speed, -2);

	i = AddItem(ctx, eIT_BodyArmour, "wizards muscle shirt");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_M_ARMOUR | GEARDROP_MAGIC);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_Magic, 3);
	SetItemAttributeValue_Int(i, eIA_i_ArmourRating, 5);
	SetItemAttributeValue_Int(i, eIA_i_ManaAdjust, 10);
	SetItemAttributeValue_Int(i, eIA_i_StatAdjust_Speed, 1);

	i = AddItem(ctx, eIT_BodyArmour, "unicorn t-shirt");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_M_ARMOUR | GEARDROP_MAGIC);
	SetItemAttributeValue_Int(i, eIA_i_ArmourRating, 5);
	SetItemAttributeValue_Int(i, eIA_i_ManaAdjust, 15);
	SetItemAttributeValue_Int(i, eIA_i_StatAdjust_Speed, 2);


	// high 7-10
	i = AddItem(ctx, eIT_BodyArmour, "chainmail armour");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_H_ARMOUR);
	SetItemAttributeValue_Int(i, eIA_i_ArmourRating, 7);
	SetItemAttributeValue_Int(i, eIA_i_StatAdjust_Speed, -3);

	i = AddItem(ctx, eIT_BodyArmour, "banded chain armour");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_H_ARMOUR);
	SetItemAttributeValue_Int(i, eIA_i_ArmourRating, 8);
	SetItemAttributeValue_Int(i, eIA_i_StatAdjust_Speed, -3);

	i = AddItem(ctx, eIT_BodyArmour, "half plate armour");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_H_ARMOUR);
	SetItemAttributeValue_Int(i, eIA_i_ArmourRating, 8);
	SetItemAttributeValue_Int(i, eIA_i_StatAdjust_Speed, -3);

	i = AddItem(ctx, eIT_BodyArmour, "plate armour");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_H_ARMOUR);
	SetItemAttributeValue_Int(i, eIA_i_ArmourRating, 10);
	SetItemAttributeValue_Int(i, eIA_i_StatAdjust_Speed, -4);

	i = AddItem(ctx, eIT_BodyArmour, "polished plate armour");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_H_ARMOUR);
	SetItemAttributeValue_Int(i, eIA_i_ArmourRating, 10);
	SetItemAttributeValue_Int(i, eIA_i_StatAdjust_Speed, -4);
	SetItemAttributeValue_Bool(i, eIA_b_Reflective, true);


	i = AddItem(ctx, eIT_BodyArmour, "purple velour robe");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_H_ARMOUR | GEARDROP_MAGIC);
	SetItemAttributeValue_Int(i, eIA_i_ArmourRating, 7);
	SetItemAttributeValue_Int(i, eIA_i_ManaAdjust, 10);


	i = AddItem(ctx, eIT_BodyArmour, "sorcerer cloak");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_H_ARMOUR | GEARDROP_MAGIC);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_Magic, 4);
	SetItemAttributeValue_Int(i, eIA_i_ArmourRating, 8);
	SetItemAttributeValue_Int(i, eIA_i_ManaAdjust, 5);
	SetItemAttributeValue_Int(i, eIA_i_StatAdjust_Speed, 1);

	i = AddItem(ctx, eIT_BodyArmour, "dosember t-shirt");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_H_ARMOUR | GEARDROP_MAGIC);
	SetItemAttributeValue_Int(i, eIA_i_ArmourRating, 8);
	SetItemAttributeValue_Int(i, eIA_i_ManaAdjust, 25);
	SetItemAttributeValue_Int(i, eIA_i_StatAdjust_Speed, 1);



	////////////////////////////////////////////////////////////////////////////////////////////
	// rings

	// helm of the cats eye... unique, provides dark vision...
	// we want it to only provide light when worn...
	i = AddItem(ctx, eIT_Ring, "ring of vision");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_ARMOUR | GEARDROP_M_ARMOUR | GEARDROP_H_ARMOUR | GEARDROP_MAGIC);
	SetItemAttributeValue_Bool(i, eIA_b_IsLitWhenWorn, true);
	SetItemAttributeValue_Bool(i, eIA_b_IsLit, false);
	SetItemAttributeValue_Int(i, eIA_i_LightRadius, 8);
	SetItemAttributeValue_Int(i, eIA_i_LightTurns, ITEM_TURNS_MAX);

	i = AddItem(ctx, eIT_Ring, ITEM_NAME_RING_PROTECTION);
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_ARMOUR | GEARDROP_M_ARMOUR | GEARDROP_H_ARMOUR | GEARDROP_MAGIC);
	SetItemAttributeValue_Int(i, eIA_i_ArmourRating, 2);

	i = AddItem(ctx, eIT_Ring, "ring of hardened steel");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_ARMOUR | GEARDROP_M_ARMOUR | GEARDROP_H_ARMOUR | GEARDROP_MAGIC);
	SetItemAttributeValue_Int(i, eIA_i_ArmourRating, 4);
	AddUnknown(i, 4);

	i = AddItem(ctx, eIT_Ring, "ring of mana boost");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_ARMOUR | GEARDROP_M_ARMOUR | GEARDROP_H_ARMOUR | GEARDROP_MAGIC);
	SetItemAttributeValue_Int(i, eIA_i_ManaAdjust, 10);
	AddUnknown(i, 4);

	i = AddItem(ctx, eIT_Ring, "ring of energy");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_ARMOUR | GEARDROP_M_ARMOUR | GEARDROP_H_ARMOUR | GEARDROP_MAGIC);
	SetItemAttributeValue_Int(i, eIA_i_StatAdjust_Speed, 3);
	AddUnknown(i, 4);

	i = AddItem(ctx, eIT_Ring, "ring of life");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_ARMOUR | GEARDROP_M_ARMOUR | GEARDROP_H_ARMOUR | GEARDROP_MAGIC);
	SetItemAttributeValue_Int(i, eIA_i_LifeBoost, 25);
	AddUnknown(i, 4);


	////////////////////////////////////////////////////////////////////////////////////////////
	// wands

	// wand of missles cast 1d6+0 at target
	i = AddItem(ctx, eIT_1h_Weapon, ITEM_NAME_WAND_MISSILES);
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_WEAPON | GEARDROP_M_WEAPON | GEARDROP_H_WEAPON | GEARDROP_MAGIC);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_Magic, 1);
	AddItemAttribute_Bool(i, eIA_b_FireTargetWand, true);
	AddItemAttribute_Int(i, eIA_i_CastsSpell, eST_AOE);
	AddItemAttribute_Int(i, eIA_i_CastManaCost, 1);
	AddItemAttribute_KV(i, eIA_kv_MagicColour, 0x0C, '*');
	AddItemAttribute_Die(i, eIA_d_DoRangedDamage, 1, 6, 2);

	i = AddItem(ctx, eIT_1h_Weapon, "wand of pulverising orbs");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_WEAPON | GEARDROP_M_WEAPON | GEARDROP_H_WEAPON | GEARDROP_MAGIC);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_Magic, 1);
	AddItemAttribute_Bool(i, eIA_b_FireTargetWand, true);
	AddItemAttribute_Int(i, eIA_i_CastsSpell, eST_MagicMissile);
	AddItemAttribute_Int(i, eIA_i_CastManaCost, 1);
	AddItemAttribute_KV(i, eIA_kv_MagicColour, 0x0E, '*');
	AddItemAttribute_Die(i, eIA_d_DoRangedDamage, 1, 8, 2);
	AddUnknown(i, 4);

	i = AddItem(ctx, eIT_1h_Weapon, "wand of piercing barbs");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_WEAPON | GEARDROP_M_WEAPON | GEARDROP_H_WEAPON | GEARDROP_MAGIC);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_Magic, 1);
	AddItemAttribute_Bool(i, eIA_b_FireTargetWand, true);
	AddItemAttribute_Int(i, eIA_i_CastsSpell, eST_MagicMissile);
	AddItemAttribute_Int(i, eIA_i_CastManaCost, 2);
	AddItemAttribute_KV(i, eIA_kv_MagicColour, 0x09, '*');
	AddItemAttribute_Die(i, eIA_d_DoRangedDamage, 2, 6, 1);
	AddUnknown(i, 4);

	i = AddItem(ctx, eIT_1h_Weapon, "wand of spiked vines");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_WEAPON | GEARDROP_M_WEAPON | GEARDROP_H_WEAPON | GEARDROP_MAGIC);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_Magic, 2);
	AddItemAttribute_Bool(i, eIA_b_FireTargetWand, true);
	AddItemAttribute_Int(i, eIA_i_CastsSpell, eST_MagicMissile);
	AddItemAttribute_Int(i, eIA_i_CastManaCost, 2);
	AddItemAttribute_KV(i, eIA_kv_MagicColour, 0x02, '%');
	AddItemAttribute_Die(i, eIA_d_DoRangedDamage, 1, 12, 4);
	AddUnknown(i, 4);

	i = AddItem(ctx, eIT_1h_Weapon, "wand of spikey thorns");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_WEAPON | GEARDROP_M_WEAPON | GEARDROP_H_WEAPON | GEARDROP_MAGIC);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_Magic, 2);
	AddItemAttribute_Bool(i, eIA_b_FireTargetWand, true);
	AddItemAttribute_Int(i, eIA_i_CastsSpell, eST_MagicMissile);
	AddItemAttribute_Int(i, eIA_i_CastManaCost, 2);
	AddItemAttribute_KV(i, eIA_kv_MagicColour, 0x02, '%');
	AddItemAttribute_Die(i, eIA_d_DoRangedDamage, 2, 7, 4);
	AddUnknown(i, 4);

	i = AddItem(ctx, eIT_1h_Weapon, "wand of internal bleeding");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_WEAPON | GEARDROP_M_WEAPON | GEARDROP_H_WEAPON | GEARDROP_MAGIC);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_Magic, 2);
	AddItemAttribute_Bool(i, eIA_b_FireTargetWand, true);
	AddItemAttribute_Int(i, eIA_i_CastsSpell, eST_MagicMissile);
	AddItemAttribute_Int(i, eIA_i_CastManaCost, 3);
	AddItemAttribute_KV(i, eIA_kv_MagicColour, 0x02, '%');
	AddItemAttribute_Die(i, eIA_d_DoRangedDamage, 2, 8, 4);
	AddUnknown(i, 4);


	i = AddItem(ctx, eIT_1h_Weapon, "wand of exploding points");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_WEAPON | GEARDROP_M_WEAPON | GEARDROP_H_WEAPON | GEARDROP_MAGIC);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_Magic, 3);
	AddItemAttribute_Bool(i, eIA_b_FireTargetWand, true);
	AddItemAttribute_Int(i, eIA_i_CastsSpell, eST_MagicMissile);
	AddItemAttribute_Int(i, eIA_i_CastManaCost, 3);
	AddItemAttribute_KV(i, eIA_kv_MagicColour, 0x04, ':');
	AddItemAttribute_Die(i, eIA_d_DoRangedDamage, 3, 6, 2);
	AddUnknown(i, 4);

	i = AddItem(ctx, eIT_1h_Weapon, "wand of paaiiiiinnnnn");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_WEAPON | GEARDROP_M_WEAPON | GEARDROP_H_WEAPON | GEARDROP_MAGIC);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_Magic, 3);
	AddItemAttribute_Bool(i, eIA_b_FireTargetWand, true);
	AddItemAttribute_Int(i, eIA_i_CastsSpell, eST_MagicMissile);
	AddItemAttribute_Int(i, eIA_i_CastManaCost, 3);
	AddItemAttribute_KV(i, eIA_kv_MagicColour, 0x0D, '%');
	AddItemAttribute_Die(i, eIA_d_DoRangedDamage, 2, 10, 2);
	AddUnknown(i, 4);

	i = AddItem(ctx, eIT_1h_Weapon, "wand of pulpy explosions");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_WEAPON | GEARDROP_M_WEAPON | GEARDROP_H_WEAPON | GEARDROP_MAGIC);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_Magic, 3);
	AddItemAttribute_Bool(i, eIA_b_FireTargetWand, true);
	AddItemAttribute_Int(i, eIA_i_CastsSpell, eST_MagicMissile);
	AddItemAttribute_Int(i, eIA_i_CastManaCost, 4);
	AddItemAttribute_KV(i, eIA_kv_MagicColour, 0x0E, '*');
	AddItemAttribute_Die(i, eIA_d_DoRangedDamage, 3, 7, 4);
	AddUnknown(i, 4);

	i = AddItem(ctx, eIT_1h_Weapon, "wand of gibs");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_WEAPON | GEARDROP_M_WEAPON | GEARDROP_H_WEAPON | GEARDROP_MAGIC);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_Magic, 4);
	AddItemAttribute_Bool(i, eIA_b_FireTargetWand, true);
	AddItemAttribute_Int(i, eIA_i_CastsSpell, eST_MagicMissile);
	AddItemAttribute_Int(i, eIA_i_CastManaCost, 4);
	AddItemAttribute_KV(i, eIA_kv_MagicColour, 0x04, '*');
	AddItemAttribute_Die(i, eIA_d_DoRangedDamage, 3, 8, 3);
	AddUnknown(i, 5);

	i = AddItem(ctx, eIT_1h_Weapon, "wand of insides out");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_WEAPON | GEARDROP_M_WEAPON | GEARDROP_H_WEAPON | GEARDROP_MAGIC);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_Magic, 4);
	AddItemAttribute_Bool(i, eIA_b_FireTargetWand, true);
	AddItemAttribute_Int(i, eIA_i_CastsSpell, eST_MagicMissile);
	AddItemAttribute_Int(i, eIA_i_CastManaCost, 4);
	AddItemAttribute_KV(i, eIA_kv_MagicColour, 0x04, '*');
	AddItemAttribute_Die(i, eIA_d_DoRangedDamage, 4, 10, 2);
	AddUnknown(i, 5);

	i = AddItem(ctx, eIT_1h_Weapon, "wand of dagger storm");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_WEAPON | GEARDROP_M_WEAPON | GEARDROP_H_WEAPON | GEARDROP_MAGIC);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_Magic, 5);
	AddItemAttribute_Bool(i, eIA_b_FireTargetWand, true);
	AddItemAttribute_Int(i, eIA_i_CastsSpell, eST_MagicMissile);
	AddItemAttribute_Int(i, eIA_i_CastManaCost, 4);
	AddItemAttribute_KV(i, eIA_kv_MagicColour, 0x0B, '|');
	AddItemAttribute_Die(i, eIA_d_DoRangedDamage, 5, 8, 3);
	AddUnknown(i, 5);

	i = AddItem(ctx, eIT_1h_Weapon, "wand of piercing daggers");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_WEAPON | GEARDROP_M_WEAPON | GEARDROP_H_WEAPON | GEARDROP_MAGIC);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_Magic, 5);
	AddItemAttribute_Bool(i, eIA_b_FireTargetWand, true);
	AddItemAttribute_Int(i, eIA_i_CastsSpell, eST_MagicMissile);
	AddItemAttribute_Int(i, eIA_i_CastManaCost, 5);
	AddItemAttribute_KV(i, eIA_kv_MagicColour, 0x0B, '^');
	AddItemAttribute_Die(i, eIA_d_DoRangedDamage, 5, 8, 5);
	AddUnknown(i, 5);

	i = AddItem(ctx, eIT_1h_Weapon, "wand of killer barbs");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_WEAPON | GEARDROP_M_WEAPON | GEARDROP_H_WEAPON | GEARDROP_MAGIC);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_Magic, 6);
	AddItemAttribute_Bool(i, eIA_b_FireTargetWand, true);
	AddItemAttribute_Int(i, eIA_i_CastsSpell, eST_MagicMissile);
	AddItemAttribute_Int(i, eIA_i_CastManaCost, 6);
	AddItemAttribute_KV(i, eIA_kv_MagicColour, 0x0E, 'B');
	AddItemAttribute_Die(i, eIA_d_DoRangedDamage, 4, 11, 3);
	AddUnknown(i, 6);


	///////////////////////////////////////////////////////////////////////////////////////////////
	// torches

	i = AddItem(ctx, eIT_Torch, ITEM_NAME_TORCH);
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_TREASURE);
	SetItemAttributeValue_Bool(i, eIA_b_IsLit, false);
	SetItemAttributeValue_Bool(i, eIA_b_DestroyedWhenDone, true);
	SetItemAttributeValue_Int(i, eIA_i_LightRadius, 7);
	SetItemAttributeValue_Int(i, eIA_i_LightTurns, 75 + xrnd(50));        // each game torch length is random

	i = AddItem(ctx, eIT_Torch, "cave torch");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_TREASURE);
	SetItemAttributeValue_Bool(i, eIA_b_IsLit, false);
	SetItemAttributeValue_Bool(i, eIA_b_DestroyedWhenDone, true);
	SetItemAttributeValue_Int(i, eIA_i_LightRadius, 8);
	SetItemAttributeValue_Int(i, eIA_i_LightTurns, 100 + xrnd(50));        // each game torch length is random

	i = AddItem(ctx, eIT_Torch, ITEM_NAME_BRASS_LANTERN);
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_TREASURE);
	SetItemAttributeValue_Bool(i, eIA_b_IsLit, false);
	SetItemAttributeValue_Bool(i, eIA_b_DestroyedWhenDone, false);
	SetItemAttributeValue_Int(i, eIA_i_LightRadius, 10);
	SetItemAttributeValue_Int(i, eIA_i_LightTurns, 150 + xrnd(100));        // each game torch length is random

	i = AddItem(ctx, eIT_Torch, "camp lantern");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_TREASURE);
	SetItemAttributeValue_Bool(i, eIA_b_IsLit, false);
	SetItemAttributeValue_Bool(i, eIA_b_DestroyedWhenDone, false);
	SetItemAttributeValue_Int(i, eIA_i_LightRadius, 15);
	SetItemAttributeValue_Int(i, eIA_i_LightTurns, 200 + xrnd(100));        // each game torch length is random


	//////////////////////////////////////////////////////////////////////////////////////////////
	// helms

	i = AddItem(ctx, eIT_Helmet, "plastic bucket");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_ARMOUR);
	SetItemAttributeValue_Int(i, eIA_i_ArmourRating, 1);

	i = AddItem(ctx, eIT_Helmet, "noggin protector");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_ARMOUR);
	SetItemAttributeValue_Int(i, eIA_i_ArmourRating, 1);

	i = AddItem(ctx, eIT_Helmet, "iron helm");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_ARMOUR);
	SetItemAttributeValue_Int(i, eIA_i_ArmourRating, 2);
	SetItemAttributeValue_Int(i, eIA_i_StatAdjust_Speed, -1);

	i = AddItem(ctx, eIT_Helmet, "leather cap");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_ARMOUR);
	SetItemAttributeValue_Int(i, eIA_i_ArmourRating, 2);

	i = AddItem(ctx, eIT_Helmet, "hard hat");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_ARMOUR);
	SetItemAttributeValue_Int(i, eIA_i_ArmourRating, 3);

	i = AddItem(ctx, eIT_Helmet, "chain mail helm");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_ARMOUR);
	SetItemAttributeValue_Int(i, eIA_i_ArmourRating, 3);

	i = AddItem(ctx, eIT_Helmet, "plate helm");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_ARMOUR);
	SetItemAttributeValue_Int(i, eIA_i_ArmourRating, 5);

	i = AddItem(ctx, eIT_Helmet, "winged cap");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_ARMOUR);
	SetItemAttributeValue_Int(i, eIA_i_ArmourRating, 1);
	SetItemAttributeValue_Int(i, eIA_i_StatAdjust_Speed, 2);


	// helm of the cats eye... unique, provides dark vision...
	// we want it to only provide light when worn...
	i = AddItem(ctx, eIT_Helmet, "helm 'cats eye'");
	SetItemAttributeValue_Bool(i, eIA_b_Unique, true);
	SetItemAttributeValue_Bool(i, eIA_b_IsLitWhenWorn, true);
	SetItemAttributeValue_Bool(i, eIA_b_DestroyedWhenDone, false);
	SetItemAttributeValue_Bool(i, eIA_b_IsLit, false);
	SetItemAttributeValue_Int(i, eIA_i_LightRadius, 10);
	SetItemAttributeValue_Int(i, eIA_i_LightTurns, ITEM_TURNS_MAX);

	i = AddItem(ctx, eIT_Helmet, "sorcerer helm");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_L_ARMOUR);
	SetItemAttributeValue_Int(i, eIA_i_ArmourRating, 8);
	AddItemAttribute_KV(i, eIA_kv_MinSkillRequired, eSkill_Magic, 3);
	SetItemAttributeValue_Int(i, eIA_i_ManaAdjust, 5);
	SetItemAttributeValue_Int(i, eIA_i_StatAdjust_Speed, 1);

	i = AddItem(ctx, eIT_Helmet, "strong helm");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_M_ARMOUR);
	SetItemAttributeValue_Int(i, eIA_i_ArmourRating, 8);
	SetItemAttributeValue_Int(i, eIA_i_ManaAdjust, 5);
	SetItemAttributeValue_Int(i, eIA_i_StatAdjust_Speed, 1);

	///////////////////////////////////////////////////////////////////////////////////////////////
	// potions and drinks

	i = AddItem(ctx, eIT_Consumable, EPB_NAME_HEAL1);
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_TREASURE | GEARDROP_MAGIC);
	AddItemAttribute_Int(i, eIA_i_PotionBase, ePB_Potion_Heal1);
	AddItemAttribute_Die(i, eIA_d_DoHeal, 1, 8, 10);
	SetItemAttributeValue_Bool(i, eIA_b_Unknown, false);

	i = AddItem(ctx, eIT_Consumable, EPB_NAME_HEAL2);
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_TREASURE | GEARDROP_MAGIC);
	AddItemAttribute_Int(i, eIA_i_PotionBase, ePB_Potion_Heal2);
	AddItemAttribute_Die(i, eIA_d_DoHeal, 2, 8, 10);
	if(GenPotion(ctx, i, 3) != 0)
	{
		return 1;
	}

	i = AddItem(ctx, eIT_Consumable, EPB_NAME_HEAL3);
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_TREASURE | GEARDROP_MAGIC);
	AddItemAttribute_Int(i, eIA_i_PotionBase, ePB_Potion_Heal3);
	AddItemAttribute_Die(i, eIA_d_DoHeal, 3, 8, 10);
	if(GenPotion(ctx, i, 2) != 0)
	{
		return 1;
	}

	i = AddItem(ctx, eIT_Consumable, EPB_NAME_MANA_REFILL1);
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_TREASURE | GEARDROP_MAGIC);
	AddItemAttribute_Int(i, eIA_i_PotionBase, ePB_ManaRefill1);
	AddItemAttribute_Die(i, eIA_d_AddMana, 2, 6, 4);
	SetItemAttributeValue_Bool(i, eIA_b_Unknown, false);

	i = AddItem(ctx, eIT_Consumable, EPB_NAME_MANA_REFILL2);
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_TREASURE | GEARDROP_MAGIC);
	AddItemAttribute_Int(i, eIA_i_PotionBase, ePB_ManaRefill2);
	AddItemAttribute_Die(i, eIA_d_AddMana, 3, 6, 4);
	if(GenPotion(ctx, i, 2) != 0)
	{
		return 1;
	}

	i = AddItem(ctx, eIT_Consumable, EPB_NAME_MANA_REFILL3);
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_TREASURE | GEARDROP_MAGIC);
	AddItemAttribute_Int(i, eIA_i_PotionBase, ePB_ManaRefill3);
	AddItemAttribute_Die(i, eIA_d_AddMana, 3, 8, 4);
	if(GenPotion(ctx, i, 2) != 0)
	{
		return 1;
	}

	i = AddItem(ctx, eIT_Consumable, EPB_NAME_MANA_REFILL4);
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_TREASURE | GEARDROP_MAGIC);
	AddItemAttribute_Int(i, eIA_i_PotionBase, ePB_ManaRefill4);
	AddItemAttribute_Die(i, eIA_d_AddMana, 120, 120, 120);
	if(GenPotion(ctx, i, 1) != 0)
	{
		return 1;
	}

	i = AddItem(ctx, eIT_Consumable, EPB_NAME_WATER);
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_TREASURE | GEARDROP_MAGIC);
	AddItemAttribute_Int(i, eIA_i_PotionBase, ePB_Water);
	AddItemAttribute_Die(i, eIA_d_DoHeal, 1, 3, 1);
	if(GenPotion(ctx, i, 1) != 0)
	{
		return 1;
	}

	i = AddItem(ctx, eIT_Consumable, EPB_NAME_POI1);
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_TREASURE | GEARDROP_MAGIC);
	AddItemAttribute_Int(i, eIA_i_PotionBase, ePB_Poisoned);
	AddItemAttribute_Int(i, eIA_i_Poison1for3__food_only, 100 + xrnd(50));
	if(GenPotion(ctx, i, 1) != 0)
	{
		return 1;
	}

	i = AddItem(ctx, eIT_Consumable, EPB_NAME_CURE_POI1);
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_TREASURE | GEARDROP_MAGIC);
	AddItemAttribute_Int(i, eIA_i_PotionBase, ePB_Water);
	AddItemAttribute_Bool(i, eIA_b_CurePoison, true);
	if(GenPotion(ctx, i, 1) != 0)
	{
		return 1;
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// rations (all drinks have a pobtion base!) food != drink, no potion base.
	i = AddItem(ctx, eIT_Consumable, "fortune cookie");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_TREASURE | GEARDROP_MAGIC);
	AddItemAttribute_Bool(i, eIA_b_GiveFortune, true);
	AddItemAttribute_Int(i, eIA_i_FoodBase, eFB_HealthyFood1);
	AddItemAttribute_Die(i, eIA_d_DoHeal, 1, 3, 0);

	i = AddItem(ctx, eIT_Consumable, "rotten christmas dinner");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_TREASURE | GEARDROP_MAGIC);
	AddItemAttribute_Int(i, eIA_i_FoodBase, eFB_HealthyFood1);
	AddItemAttribute_Die(i, eIA_d_DoDamage, 1, 6, 1);
	if(GenFood(ctx, i, 2) != 0)
	{
		return 1;
	}

	i = AddItem(ctx, eIT_Consumable, "pickled dark elf eyes");
	SetItemAttributeValue_Int(i, eIA_i_DropClass, GEARDROP_TREASURE | GEARDROP_MAGIC);
	AddItemAttribute_Int(i, eIA_i_LightRadius, 9);
	AddItemAttribute_Int(i, eIA_i_FoodBase, eFB_HealthyFood1);
	SetItemAttributeValue_Int(i, eIA_i_LightTurns, 50 + xrnd(50));
	if(GenFood(ctx, i, 1) != 0)
	{
		return 1;
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// winning item!
	i = AddItem(ctx, eIT_Helmet, ITEM_NAME_CROWN_DARKNESS);
	SetItemAttributeValue_Bool(i, eIA_b_Unique, true);
	SetItemAttributeValue_Bool(i, eIA_b_IsLitWhenWorn, true);
	SetItemAttributeValue_Bool(i, eIA_b_DestroyedWhenDone, false);
	SetItemAttributeValue_Bool(i, eIA_b_IsLit, false);
	SetItemAttributeValue_Int(i, eIA_i_LightRadius, 10);
	SetItemAttributeValue_Int(i, eIA_i_LightTurns, ITEM_TURNS_MAX);
	SetItemAttributeValue_Int(i, eIA_i_ArmourRating, 5);

	return 0;
}

void GenerateCopy(uItem *src, uItem *dest)
{
	if(src == NULL || dest == NULL)
	{
		return;
	}

	memmove(dest, src, sizeof(uItem));
}

uint16_t GetFreeItemSlot(uContext *ctx)
{
	uint16_t q;

	for(q = 0; q < MAX_INVENTORY_ITEMS; q++)
	{
		if(ctx->player->items[q].item.idx == 0)
		{
			return q;
		}
	}

	return UINT16_MAX;
}

uItem *AddItemToInventory(uContext *ctx, uItem *i, int16_t count)
{
	uint16_t q;

	if(i == NULL)
	{
		return NULL;
	}

	for(q = 0; q < MAX_INVENTORY_ITEMS; q++)
	{
		if(ctx->player->items[q].item.idx == 0)
		{
			// copy item into inventory
			ctx->player->items[q].count = count;
			GenerateCopy(i, &ctx->player->items[q].item);
			return &ctx->player->items[q].item;
		}
	}

	return NULL;
}

bool IsItemBodySlot(uContext *ctx, uItem *item, uint16_t slot)
{
	if(slot == EMPTY_ITEM_SLOT)
	{
		return false;
	}

	if(&ctx->player->items[slot].item == item)
	{
		return true;
	}

	return false;
}


uint16_t GetItemSlot(uContext *ctx, uItem *item)
{
	uint16_t i;

	for(i = 0; i < MAX_INVENTORY_ITEMS; i++)
	{
		if(&ctx->player->items[i].item == item)
		{
			return i;
		}
	}

	return EMPTY_ITEM_SLOT;
}

bool IsAlreadyEquipped(uContext *ctx, uItem *item)
{
	if(IsItemBodySlot(ctx, item, ctx->player->item_body) == true)
	{
		return true;
	}
	if(IsItemBodySlot(ctx, item, ctx->player->item_head) == true)
	{
		return true;
	}
	if(IsItemBodySlot(ctx, item, ctx->player->item_left_weap) == true)
	{
		return true;
	}
	if(IsItemBodySlot(ctx, item, ctx->player->item_right_weap) == true)
	{
		return true;
	}
	if(IsItemBodySlot(ctx, item, ctx->player->item_left_hand) == true)
	{
		return true;
	}
	if(IsItemBodySlot(ctx, item, ctx->player->item_right_hand) == true)
	{
		return true;
	}

	return false;
}

bool IsAlreadyUnEquipped(uContext *ctx, uItem *item)
{
	if(IsItemBodySlot(ctx, item, ctx->player->item_body) == true)
	{
		return false;
	}
	if(IsItemBodySlot(ctx, item, ctx->player->item_head) == true)
	{
		return false;
	}
	if(IsItemBodySlot(ctx, item, ctx->player->item_left_weap) == true)
	{
		return false;
	}
	if(IsItemBodySlot(ctx, item, ctx->player->item_right_weap) == true)
	{
		return false;
	}
	if(IsItemBodySlot(ctx, item, ctx->player->item_left_hand) == true)
	{
		return false;
	}
	if(IsItemBodySlot(ctx, item, ctx->player->item_right_hand) == true)
	{
		return false;
	}

	return true;
}

uItem *GetPlayerItemHead(uContext *ctx)
{
	if(ctx->player->item_head == EMPTY_ITEM_SLOT)
	{
		return NULL;
	}

	return &ctx->player->items[ctx->player->item_head].item;
}

uItem *GetPlayerItemBody(uContext *ctx)
{
	if(ctx->player->item_body == EMPTY_ITEM_SLOT)
	{
		return NULL;
	}

	return &ctx->player->items[ctx->player->item_body].item;
}

uItem *GetPlayerItemLeftHand(uContext *ctx)
{
	if(ctx->player->item_left_hand == EMPTY_ITEM_SLOT)
	{
		return NULL;
	}

	return &ctx->player->items[ctx->player->item_left_hand].item;
}

uItem *GetPlayerItemRightHand(uContext *ctx)
{
	if(ctx->player->item_right_hand == EMPTY_ITEM_SLOT)
	{
		return NULL;
	}

	return &ctx->player->items[ctx->player->item_right_hand].item;
}

uItem *GetPlayerItemWeaponLeft(uContext *ctx)
{
	if(ctx->player->item_left_weap == EMPTY_ITEM_SLOT)
	{
		return NULL;
	}

	return &ctx->player->items[ctx->player->item_left_weap].item;
}

uItem *GetPlayerItemWeaponRight(uContext *ctx)
{
	if(ctx->player->item_right_weap == EMPTY_ITEM_SLOT)
	{
		return NULL;
	}

	return &ctx->player->items[ctx->player->item_right_weap].item;
}


void ReCalcPlayerVision(uContext *ctx)
{
	uint16_t i;
	uint16_t j;

	ctx->player->stats[eStat_DeepVision] = ctx->player->base_stats[eStat_DeepVision];

	// for every item in our inventry
	for(i = 0; i < MAX_INVENTORY_ITEMS; i++)
	{
		// is item idx 0 or UINT16_MAX ?
		if(ctx->player->items[i].item.idx != 0)
		{
			if(GetItemAttributeValue_Bool(&ctx->player->items[i].item, eIA_b_IsLit, false) == true)
			{
				j = GetItemAttributeValue_Int(&ctx->player->items[i].item, eIA_i_LightRadius, 0);

				// it is lit, so give us radius!
				if(ctx->player->stats[eStat_DeepVision] < j)
				{
					ctx->player->stats[eStat_DeepVision] = j;
				}
			}
		}
	}

	// check temporary counters
	for(i = 0; i < MAX_COUNTER_ITEMS; i++)
	{
		// this is an absolute
		if(ctx->player->counters[i].effect == eIA_i_LightRadius)
		{
			if(ctx->player->counters[i].rounds > 0)
			{
				j = ctx->player->counters[i].adjust;

				if(ctx->player->stats[eStat_DeepVision] < j)
				{
					ctx->player->stats[eStat_DeepVision] = j;
				}
			}
		}
	}

}

static void DoIA_AddEffect(uContext *ctx, uint16_t type, int16_t value)
{
	switch(type)
	{
		// trigger adjustments
		case eIA_None:
			break;

			// just status markers applied as a counter effect, not add/sub effects
		case eIA_i_Poison1for1__food_only:
		case eIA_i_Poison1for3__food_only:
		case eIA_i_Poison1for5__food_only:
			break;


		case eIA_i_ArmourRating:
			ctx->player->armour += value;
			if(ctx->player->armour < 0)
			{
				ctx->player->armour = 0;
			}
			ctx->redraw_status = true;
			break;

		case eIA_i_StatAdjust_Speed:
			ctx->player->base_stats[eStat_Speed] += value;
			ctx->player->stats[eStat_Speed] += value;
			if(ctx->player->stats[eStat_Speed] > ctx->player->base_stats[eStat_Speed])
			{
				ctx->player->stats[eStat_Speed] = ctx->player->base_stats[eStat_Speed];
			}
			ctx->redraw_status = true;
			break;

		case eIA_i_ManaAdjust:
			ctx->player->max_mana += value;
			if(ctx->player->max_mana < 1)
			{
				ctx->player->max_mana = 1;
			}
			ctx->player->mana += value;
			if(ctx->player->mana < 1)
			{
				ctx->player->mana = 1;
			}
			if(ctx->player->mana > ctx->player->max_mana)
			{
				ctx->player->mana = ctx->player->max_mana;
			}
			if(ctx->player->mana < 1)
			{
				ctx->player->mana = 1;
			}
			ctx->redraw_status = true;
			break;

		case eIA_i_LifeBoost:
			ctx->player->max_life += value;
			if(ctx->player->max_life < 1)
			{
				ctx->player->max_life = 1;
			}
			if(ctx->player->life > ctx->player->max_life)
			{
				ctx->player->life = ctx->player->max_life;
			}
			break;
	}
}

// this item is the item in the players inventory.
#pragma disable_message(303)

uint16_t EquipItem(uContext *ctx, uItem *item, bool in_inventory_screen)
{
	uint16_t i;
	uint16_t j;
	uItem *right_weap;
	uItem *left_weap;

	uint16_t rc = eReturn_Bad;

	if(item == NULL)
	{
		return eReturn_Bad;
	}

	for(i = 0; i < eStat_MAX; i++)
	{
		if(ctx->player->stats[i] < GetItemAttributeValue_KV(item, eIA_kv_MinStatRequired, i, 0))
		{
			switch(i)
			{
				case eStat_Strength:
					return eReturn_DontMeetStatRequirement_Strength;
				case eStat_Speed:
					return eReturn_DontMeetStatRequirement_Speed;
				case eStat_Willpower:
					return eReturn_DontMeetStatRequirement_Willpower;
				case eStat_Dexterity:
					return eReturn_DontMeetStatRequirement_Dexterity;
				case eStat_DeepVision:
					return eReturn_DontMeetStatRequirement_DeepVision;
			}
		}
	}

	for(i = 0; i < eSkill_MAX; i++)
	{
		if(ctx->player->skills[i] < GetItemAttributeValue_KV(item, eIA_kv_MinSkillRequired, i, 0))
		{
			switch(i)
			{
				case eSkill_MeleeWeapons:
					return eReturn_DontMeetSkillRequirement_Meelee;
				case eSkill_RangedWeapons:
					return eReturn_DontMeetSkillRequirement_Ranged;
				case eSkill_Magic:
					return eReturn_DontMeetSkillRequirement_Magic;
				case eSkill_ItemLore:
					return eReturn_DontMeetSkillRequirement_ItemLore;
			}
		}
	}

	// is it equipped in any other slot?
	if(IsAlreadyEquipped(ctx, item) == true)
	{
		// do we need message?
		return eReturn_AlreadyEquipped;
	}

	j = GetItemSlot(ctx, item);
	if(j == EMPTY_ITEM_SLOT)
	{
		return eReturn_NoEmptyItemSlot;
	}

	left_weap = GetPlayerItemWeaponLeft(ctx);
	right_weap = GetPlayerItemWeaponRight(ctx);

	// is proper slot free?
	if(item->base_type == eIT_BodyArmour && ctx->player->item_body == EMPTY_ITEM_SLOT)
	{
		ctx->player->item_body = j;
		rc = eReturn_OK;
	}
	else if(item->base_type == eIT_Helmet && ctx->player->item_head == EMPTY_ITEM_SLOT)
	{
		ctx->player->item_head = j;
		rc = eReturn_OK;
	}
	else if(item->base_type == eIT_AmmunitionForBow && (ctx->player->item_left_weap == EMPTY_ITEM_SLOT || ctx->player->item_right_weap == EMPTY_ITEM_SLOT))
	{
		// if left hand is bow, put ammo into right
		if(left_weap != NULL && left_weap->base_type == eIT_2h_Weapon && GetItemAttributeValue_Bool(left_weap, eIA_b_FireTargetBow, false) == true)
		{
			// we have bow in left hand so ready up arrows in right!
			ctx->player->item_right_weap = j;
		}
		else
		{
			// just ready up ammon in left... ? no?
			if(left_weap == NULL)
			{
				ctx->player->item_left_weap = j;
			}
			else
			{
				ctx->player->item_right_weap = j;
			}
		}

		rc = eReturn_OK;
	}
	else if(item->base_type == eIT_2h_Weapon &&
			(
				// 2h + empty, ok
				(ctx->player->item_left_weap == EMPTY_ITEM_SLOT)
				// 2h BOW + lh ammunition? ok
				|| (left_weap != NULL && left_weap->base_type == eIT_AmmunitionForBow && GetItemAttributeValue_Bool(item, eIA_b_FireTargetBow, false) == true)
				// 2h bow + rh ammunition? ok
				|| (right_weap != NULL && right_weap->base_type == eIT_AmmunitionForBow && GetItemAttributeValue_Bool(item, eIA_b_FireTargetBow, false) == true)
			))
	{
		// make sure ammo is in left hand if bow + ammo
		if(left_weap != NULL && left_weap->base_type == eIT_AmmunitionForBow && GetItemAttributeValue_Bool(item, eIA_b_FireTargetBow, false) == true)
		{
			// and its a bow
			ctx->player->item_right_weap = ctx->player->item_left_weap;
		}

		ctx->player->item_left_weap = j;

//		if(right_hand != NULL && right_hand->base_type != eIT_AmmunitionForBow)
//		{
//			ctx->player->item_right_weap = EMPTY_ITEM_SLOT;
//		}

		rc = eReturn_OK;
	}
	else if((item->base_type == eIT_1h_Weapon || item->base_type == eIT_Shield)
			&& (ctx->player->item_left_weap == EMPTY_ITEM_SLOT || ctx->player->item_right_weap == EMPTY_ITEM_SLOT))
	{
		if(ctx->player->item_left_weap == EMPTY_ITEM_SLOT)
		{
			ctx->player->item_left_weap = j;
			rc = eReturn_OK;
		}
		else
		{
			// we know item_left_weap is used
			uItem *i2 = &ctx->player->items[ctx->player->item_left_weap].item;
			if(i2->base_type != eIT_2h_Weapon)
			{
				// check we dont have 2h weapon in left hand...
				ctx->player->item_right_weap = j;
				rc = eReturn_OK;
			}
		}
	}
	else if(item->base_type == eIT_Torch && (ctx->player->item_left_weap == EMPTY_ITEM_SLOT || ctx->player->item_right_weap == EMPTY_ITEM_SLOT))
	{
		if(ctx->player->item_left_weap == EMPTY_ITEM_SLOT)
		{
			SetItemAttributeValue_Bool(item, eIA_b_IsLit, true);
			ctx->player->item_left_weap = j;
			rc = eReturn_OK;
			ReCalcPlayerVision(ctx);
		}
		else
		{
			// we know item_left_weap is used
			uItem *i2 = &ctx->player->items[ctx->player->item_left_weap].item;
			if(i2->base_type != eIT_2h_Weapon)
			{
				SetItemAttributeValue_Bool(item, eIA_b_IsLit, true);
				// check we dont have 2h weapon in left hand...
				ctx->player->item_right_weap = j;
				rc = eReturn_OK;
				ReCalcPlayerVision(ctx);
			}
		}
	}
	else if(item->base_type == eIT_Ring && (ctx->player->item_left_hand == EMPTY_ITEM_SLOT || ctx->player->item_right_hand == EMPTY_ITEM_SLOT))
	{
		if(ctx->player->item_left_hand == EMPTY_ITEM_SLOT)
		{
			ctx->player->item_left_hand = j;
		}
		else
		{
			ctx->player->item_right_hand = j;
		}

		rc = eReturn_OK;
	}
	else
	{
		rc = eReturn_ItemTypeAlreadyEquipped;
	}


	// we did not equip it, something happened, fall out
	if(rc != eReturn_OK)
	{
		return rc;
	}

	// process triggers
	for(i = 0; i < MAX_ITEM_ATTRIBUTES; i++)
	{
		if(item->attr[i].type > eIA_i_____CounterEffectStart && item->attr[i].type < eIA_i_____CounterEffectEnd)
		{
			switch(item->attr[i].type)
			{
				// these are status markers not add/sub effects.
				case eIA_i_Poison1for1__food_only:
				case eIA_i_Poison1for3__food_only:
				case eIA_i_Poison1for5__food_only:
					// do not apply...
					break;

				default:
					DoIA_AddEffect(ctx, item->attr[i].type, item->attr[i].attributes.i.value);
					break;
			}
		}
		else
		{
			switch(item->attr[i].type)
			{
				case eIA_None:
					break;

				case eIA_b_IsLitWhenWorn:
					if(item->attr[i].attributes.b.value == true)
					{
						// turn it on!
						SetItemAttributeValue_Bool(&ctx->player->items[i].item, eIA_b_IsLit, true);
						ReCalcPlayerVision(ctx);
					}
					break;
			}
		}
	}

	return rc;
}

#pragma enable_message(303)

uint16_t UnEquipItem(uContext *ctx, uItem *item, bool in_inventory_screen, bool ignore_cursed)
{
	uint16_t i;
	uint16_t rc = eReturn_Bad;

	uint16_t j;

	if(item == NULL)
	{
		return eReturn_Bad;
	}

	j = GetItemSlot(ctx, item);
	if(j == EMPTY_ITEM_SLOT)
	{
		return eReturn_Bad;
	}

	if(ignore_cursed == false && GetItemAttributeValue_Bool(item, eIA_b_Cursed, false) == true)
	{
		SetItemAttributeValue_Bool(item, eIA_b_CursedKnown, true);

		// cant remove a cursed item!
		if(in_inventory_screen == false)
		{
			add_msg(ctx, "I cant remove the `%s`, it must be cursed!", GetItemName(item));
		}

		return eReturn_ItemCursed;
	}

	// is proper slot free?
	if(item->base_type == eIT_BodyArmour && ctx->player->item_body == j)
	{
		ctx->player->item_body = EMPTY_ITEM_SLOT;
		rc = eReturn_OK;
	}
	else if(item->base_type == eIT_Helmet && ctx->player->item_head == j)
	{
		ctx->player->item_head = EMPTY_ITEM_SLOT;
		rc = eReturn_OK;
	}
	else if(item->base_type == eIT_2h_Weapon && ctx->player->item_left_weap == j)
	{
		ctx->player->item_left_weap = EMPTY_ITEM_SLOT;
		rc = eReturn_OK;
	}
	else if(item->base_type == eIT_Torch)
	{
		if(ctx->player->item_left_weap == j)
		{
			ctx->player->item_left_weap = EMPTY_ITEM_SLOT;
			SetItemAttributeValue_Bool(item, eIA_b_IsLit, false);
			ReCalcPlayerVision(ctx);
		}
		else
		{
			ctx->player->item_right_weap = EMPTY_ITEM_SLOT;
			SetItemAttributeValue_Bool(item, eIA_b_IsLit, false);
			ReCalcPlayerVision(ctx);
		}
	}
	else if((item->base_type == eIT_AmmunitionForBow || item->base_type == eIT_1h_Weapon || item->base_type == eIT_Shield) && (ctx->player->item_left_weap == j || ctx->player->item_right_weap == j))
	{
		if(ctx->player->item_left_weap == j)
		{
			ctx->player->item_left_weap = EMPTY_ITEM_SLOT;
		}
		else
		{
			ctx->player->item_right_weap = EMPTY_ITEM_SLOT;
		}

		rc = eReturn_OK;
	}
	else if(item->base_type == eIT_Ring && (ctx->player->item_left_hand == j || ctx->player->item_right_hand == j))
	{
		if(ctx->player->item_left_hand == j)
		{
			ctx->player->item_left_hand = EMPTY_ITEM_SLOT;
		}
		else
		{
			ctx->player->item_right_hand = EMPTY_ITEM_SLOT;
		}

		rc = eReturn_OK;
	}

	// something happened.
	if(rc != eReturn_OK)
	{
		return rc;
	}

	for(i = 0; i < MAX_ITEM_ATTRIBUTES; i++)
	{
		if(item->attr[i].type > eIA_i_____CounterEffectStart && item->attr[i].type < eIA_i_____CounterEffectEnd)
		{
			// add the reverse
			DoIA_AddEffect(ctx, item->attr[i].type, 0 - item->attr[i].attributes.i.value);
		}
		else
		{
			switch(item->attr[i].type)
			{
				case eIA_None:
					break;

				case eIA_b_IsLitWhenWorn:
					if(item->attr[i].attributes.b.value == true)
					{
						// turn it off!
						SetItemAttributeValue_Bool(&ctx->player->items[i].item, eIA_b_IsLit, false);
						ReCalcPlayerVision(ctx);
					}
					break;
			}
		}
	}

	return rc;
}

static void ProcessItemTurn(uContext *ctx, uItem *item)
{
	uint16_t i;

	for(i = 0; i < MAX_ITEM_ATTRIBUTES; i++)
	{
		switch(item->attr[i].type)
		{
			case eIA_None:
				break;

			case eIA_i_LightTurns:
				if(item->attr[i].attributes.i.value != ITEM_TURNS_MAX && item->attr[i].attributes.i.value > 0)
				{
					item->attr[i].attributes.i.value -= 1;
					if(item->attr[i].attributes.i.value == 0)
					{
						if(item->base_type == eIT_Torch)
						{
							if(GetItemAttributeValue_Bool(item, eIA_b_IsLit, false) == true)
							{
								SetItemAttributeValue_Bool(item, eIA_b_IsLit, false);
							}

							add_msg(ctx, "The %s has gone out.", GetItemName(item));
						}

						// destroy item!
						if(GetItemAttributeValue_Bool(item, eIA_b_DestroyedWhenDone, false) == true)
						{
							// mark item as free
							//item->idx = 0;
							DeleteInventoryItem(ctx, item);
						}

						// recalc FOV vision
						ReCalcPlayerVision(ctx);
					}
				}
				break;
		}
	}
}

static void DoRemoveCounter(uContext *ctx, uint16_t idx)
{
	// reverse effect
	switch(ctx->player->counters[idx].effect)
	{
		// status effects, dont get reversed. counters expire!
		case eIA_i_Poison1for1__food_only:
		case eIA_i_Poison1for3__food_only:
		case eIA_i_Poison1for5__food_only:
			break;

		case eIA_i_ArmourRating:
			ctx->player->armour -= ctx->player->counters[idx].adjust;
			break;

		case eIA_i_ManaAdjust:
			ctx->player->max_mana -= ctx->player->counters[idx].adjust;
			if(ctx->player->mana > ctx->player->max_mana)
			{
				ctx->player->mana = ctx->player->max_mana;
			}
			break;

		case eIA_i_StatAdjust_Speed:
			ctx->player->base_stats[eStat_Speed] -= ctx->player->counters[idx].adjust;
			if(ctx->player->stats[eStat_Speed] > ctx->player->base_stats[eStat_Speed])
			{
				ctx->player->stats[eStat_Speed] = ctx->player->base_stats[eStat_Speed];
			}
			break;

		case eIA_i_HeldInPosition:
			add_msg(ctx, "I can move again!");
			break;
	}

	// delete effect
	ctx->player->counters[idx].effect = 0;
}

void ProcessInventoryTurnItemTriggers(uContext *ctx)
{
	uint16_t slot;
	uint16_t i;
	bool poisoned = false;

	// recovery loss, 1 hp every 15 moves
	if(ctx->moves % 15 == 0)
	{
		if(ctx->map->recovery > 0)
		{
			if(ctx->player->life < ctx->player->max_life)
			{
				ctx->player->life += 1;
				ctx->map->recovery -= 1;
			}
		}
	}

	slot = ctx->player->item_body;
	if(slot != EMPTY_ITEM_SLOT)
	{
		ProcessItemTurn(ctx, &ctx->player->items[slot].item);
	}

	slot = ctx->player->item_head;
	if(slot != EMPTY_ITEM_SLOT)
	{
		ProcessItemTurn(ctx, &ctx->player->items[slot].item);
	}

	slot = ctx->player->item_left_weap;
	if(slot != EMPTY_ITEM_SLOT)
	{
		ProcessItemTurn(ctx, &ctx->player->items[slot].item);
	}

	slot = ctx->player->item_right_weap;
	if(slot != EMPTY_ITEM_SLOT)
	{
		ProcessItemTurn(ctx, &ctx->player->items[slot].item);
	}

	slot = ctx->player->item_left_hand;
	if(slot != EMPTY_ITEM_SLOT)
	{
		ProcessItemTurn(ctx, &ctx->player->items[slot].item);
	}

	slot = ctx->player->item_right_hand;
	if(slot != EMPTY_ITEM_SLOT)
	{
		ProcessItemTurn(ctx, &ctx->player->items[slot].item);
	}

	// expire before test
	for(i = 0; i < MAX_COUNTER_ITEMS && ctx->player->life > 0; i++)
	{
		if(ctx->player->counters[i].effect != 0)
		{
			// dont expire count if its unlimited!
			if(ctx->player->counters[i].rounds > 0 && ctx->player->counters[i].rounds != ITEM_TURNS_MAX)
			{
				// all counters decremented!
				ctx->player->counters[i].rounds -= 1;

				if(ctx->player->counters[i].rounds == 0)
				{
					DoRemoveCounter(ctx, i);
				}
			}
		}
	}

	// process affective attributes
	poisoned = false;
	for(i = 0; i < MAX_COUNTER_ITEMS && ctx->player->life > 0; i++)
	{
		switch(ctx->player->counters[i].effect)
		{
			// nothing
			case 0:
				break;

				// no other eIA_i_ effects tick in such a manner.
			case eIA_i_Poison1for5__food_only:
				if(ctx->moves % 5 == 0)
				{
					ctx->player->life -= 1;
					poisoned = true;
					if(ctx->player->life == 0)
					{
						DeathScreen(ctx, "by poisoning");
					}
				}
				break;

			case eIA_i_Poison1for3__food_only:
				if(ctx->moves % 3 == 0)
				{
					ctx->player->life -= 1;
					poisoned = true;
					if(ctx->player->life == 0)
					{
						DeathScreen(ctx, "by poisoning");
					}
				}
				break;

			case eIA_i_Poison1for1__food_only:
				if(ctx->moves % 1 == 0)
				{
					ctx->player->life -= 1;
					poisoned = true;
					if(ctx->player->life == 0)
					{
						DeathScreen(ctx, "by poisoning");
					}
				}
				break;
		}
	}

	if(poisoned == true && ctx->player->life > 0)
	{
		add_msg(ctx, "Uuuugh!");
	}

}

void ClearCounter(uContext *ctx, uint16_t effect)
{
	uint16_t i;

	for(i = 0; i < MAX_COUNTER_ITEMS && ctx->player->life > 0; i++)
	{
		if(ctx->player->counters[i].effect == effect)
		{
			DoRemoveCounter(ctx, i);
		}
	}
}

void AddCounter(uContext *ctx, uint16_t effect, uint16_t rounds, int16_t adjustment)
{
	uint16_t i;

	if(effect > eIA_i_____CounterEffectStart && effect < eIA_i_____CounterEffectEnd)
	{
		for(i = 0; i < MAX_COUNTER_ITEMS; i++)
		{
			if(ctx->player->counters[i].effect == 0)
			{
				switch(effect)
				{
					case eIA_i_Poison1for1__food_only:
					case eIA_i_Poison1for3__food_only:
					case eIA_i_Poison1for5__food_only:
						break;

					case eIA_i_HeldInPosition:
						break;
				}

				ctx->player->counters[i].effect = effect;
				ctx->player->counters[i].rounds = rounds;
				ctx->player->counters[i].adjust = adjustment;

				DoIA_AddEffect(ctx, effect, adjustment);
				return;
			}
		}
	}
}

void ClearRightWeapon(uContext *ctx)
{
	if(ctx->player->item_right_weap == ctx->player->loadout_item_left_weap)
	{
		ctx->player->loadout_item_left_weap = EMPTY_ITEM_SLOT;
	}

	if(ctx->player->item_right_weap == ctx->player->loadout_item_right_weap)
	{
		ctx->player->loadout_item_right_weap = EMPTY_ITEM_SLOT;
	}

	memset(&ctx->player->items[ctx->player->item_right_weap], 0x0, sizeof(uItem));
	ctx->player->item_right_weap = EMPTY_ITEM_SLOT;
}

void ClearLeftWeapon(uContext *ctx)
{
	if(ctx->player->item_left_weap == ctx->player->loadout_item_left_weap)
	{
		ctx->player->loadout_item_left_weap = EMPTY_ITEM_SLOT;
	}

	if(ctx->player->item_left_weap == ctx->player->loadout_item_right_weap)
	{
		ctx->player->loadout_item_right_weap = EMPTY_ITEM_SLOT;
	}

	memset(&ctx->player->items[ctx->player->item_left_weap], 0x0, sizeof(uItem));
	ctx->player->item_left_weap = EMPTY_ITEM_SLOT;
}

void SwapLoadout(uContext *ctx, bool in_inventory_screen)
{
	uint8_t r1;
	uint8_t l1;

	uint8_t lr1;
	uint8_t ll1;

	ctx->player->loadout ^= 1;

	l1 = ctx->player->item_left_weap;
	r1 = ctx->player->item_right_weap;
	ll1 = ctx->player->loadout_item_left_weap;
	lr1 = ctx->player->loadout_item_right_weap;

	if(l1 != EMPTY_ITEM_SLOT)
	{
		UnEquipItem(ctx, &ctx->player->items[l1].item, in_inventory_screen, false);
	}

	if(r1 != EMPTY_ITEM_SLOT)
	{
		UnEquipItem(ctx, &ctx->player->items[r1].item, in_inventory_screen, false);
	}

	if(ctx->player->item_left_weap == EMPTY_ITEM_SLOT)
	{
		ctx->player->loadout_item_left_weap = l1;
		if(ll1 != EMPTY_ITEM_SLOT)
		{
			EquipItem(ctx, &ctx->player->items[ll1].item, in_inventory_screen);
		}
	}

	if(ctx->player->item_right_weap == EMPTY_ITEM_SLOT)
	{
		ctx->player->loadout_item_right_weap = r1;
		if(lr1 != EMPTY_ITEM_SLOT)
		{
			EquipItem(ctx, &ctx->player->items[lr1].item, in_inventory_screen);
		}
	}

//	lok = true;
//	if(l1 != EMPTY_ITEM_SLOT)
//	{
//		if(UnEquipItem(ctx, &ctx->player->items[l1].item, in_inventory_screen) != eReturn_OK)
//		{
//			lok = false;
//		}
//	}
//
//	rok = true;
//	if(r1 != EMPTY_ITEM_SLOT)
//	{
//		if(UnEquipItem(ctx, &ctx->player->items[r1].item, in_inventory_screen) != eReturn_OK)
//		{
//			rok = false;
//		}
//	}
//
//	// now equip left + right from loadout
//	if(lok == true && ll1 != EMPTY_ITEM_SLOT)
//	{
//		EquipItem(ctx, &ctx->player->items[ll1].item, in_inventory_screen);
//	}
//	ctx->player->loadout_item_left_weap = l1;
//
//	if(rok == true && lr1 != EMPTY_ITEM_SLOT)
//	{
//		EquipItem(ctx, &ctx->player->items[lr1].item, in_inventory_screen);
//	}
//	ctx->player->loadout_item_right_weap = r1;

	if(in_inventory_screen == false)
	{
		add_msg(ctx, "You swap your loadout.");
	}
}

void DeleteInventoryItem(uContext *ctx, uItem *item)
{
	uint16_t i;

	for(i = 0; i < MAX_INVENTORY_ITEMS; i++)
	{
		uItem *item2;

		item2 = &ctx->player->items[i].item;
		if(item2 == item)
		{
			ctx->player->items[i].count = 0;
			memset(&ctx->player->items[i].item, 0x0, sizeof(uItem));
			return;
		}
	}
}

const char *GetFortune(void)
{
	uint16_t i;

	for(i = 0; fortunes[i] != NULL; i++)
	{}

	return fortunes[xrnd(i)];
}

bool Consume(uContext *ctx, uItem *item, bool inside_inventory)
{
	bool delete_item = false;
	char msgx[256];
	char *p;

	uint16_t i;
	bool fortune;
	int16_t heal;
	int16_t damage;
	int16_t mana;
	int16_t pbase;
	int16_t fbase;
	int16_t poisoned_rounds1;
	int16_t poisoned_rounds3;
	int16_t poisoned_rounds5;
	bool cure_poison;

	int16_t light_turns;
	int16_t light_radius;

	char *fortune_text;

	// quaff.. eat... consume... slurp... get it down!
	fortune = GetItemAttributeValue_Bool(item, eIA_b_GiveFortune, false);
	if(fortune == true)
	{
		fortune_text = (char *)GetFortune();
	}
	heal = GetItemAttributeValue_Die(item, eIA_d_DoHeal, INT16_MAX);
	mana = GetItemAttributeValue_Die(item, eIA_d_AddMana, INT16_MAX);
	damage = GetItemAttributeValue_Die(item, eIA_d_DoDamage, INT16_MAX);
	pbase = GetItemAttributeValue_Int(item, eIA_i_PotionBase, INT16_MAX);
	fbase = GetItemAttributeValue_Int(item, eIA_i_FoodBase, INT16_MAX);
	cure_poison = GetItemAttributeValue_Bool(item, eIA_b_CurePoison, false);

	poisoned_rounds1 = GetItemAttributeValue_Int(item, eIA_i_Poison1for1__food_only, INT16_MAX);
	poisoned_rounds3 = GetItemAttributeValue_Int(item, eIA_i_Poison1for3__food_only, INT16_MAX);
	poisoned_rounds5 = GetItemAttributeValue_Int(item, eIA_i_Poison1for5__food_only, INT16_MAX);

	light_radius = GetItemAttributeValue_Int(item, eIA_i_LightRadius, INT16_MAX);
	light_turns = GetItemAttributeValue_Int(item, eIA_i_LightTurns, INT16_MAX);

	if(pbase != INT16_MAX)
	{
		strcpy(msgx, "You drank!");
	}
	else
	{
		strcpy(msgx, "You ate!");
	}

	p = strchr(msgx, 0);
	if(heal != INT16_MAX)
	{
		if(ctx->player->life + heal > ctx->player->max_life)
		{
			heal = ctx->player->max_life - ctx->player->life;
		}

		sprintf(p, " heal (%"PRIi16"),", heal);

		ctx->player->life += heal;
		if(ctx->player->life > ctx->player->max_life)
		{
			ctx->player->life = ctx->player->max_life;
		}

		ctx->redraw_status = true;
	}

	p = strchr(msgx, 0);
	if(mana != INT16_MAX)
	{
		if(ctx->player->mana + mana > ctx->player->max_mana)
		{
			mana = ctx->player->max_mana - ctx->player->mana;
		}

		sprintf(p, " mana up (%"PRIi16"),", mana);

		ctx->player->mana += mana;
		if(ctx->player->mana > ctx->player->max_mana)
		{
			ctx->player->mana = ctx->player->max_mana;
		}

		ctx->redraw_status = true;
	}

	p = strchr(msgx, 0);
	if(damage != INT16_MAX)
	{
		sprintf(p, " pain (%"PRIi16"),", damage);

		if(damage >= ctx->player->life)
		{
			ctx->player->life = 0;
			DeathScreen(ctx, "consumed something bad!");
		}
		else
		{
			ctx->player->life -= damage;
		}

		ctx->redraw_status = true;
	}

	if(ctx->player->life == 0)
	{
		return false;
	}

	p = strchr(msgx, 0);
	if(poisoned_rounds1 != INT16_MAX)
	{
		AddCounter(ctx, eIA_i_Poison1for1__food_only, poisoned_rounds1, 0);
		sprintf(p, " nasty,");
	}

	p = strchr(msgx, 0);
	if(poisoned_rounds3 != INT16_MAX)
	{
		AddCounter(ctx, eIA_i_Poison1for3__food_only, poisoned_rounds3, 0);
		sprintf(p, " nasty,");
	}

	p = strchr(msgx, 0);
	if(poisoned_rounds5 != INT16_MAX)
	{
		AddCounter(ctx, eIA_i_Poison1for5__food_only, poisoned_rounds5, 0);
		sprintf(p, " nasty,");
	}


	if(light_radius != INT16_MAX && light_turns != INT16_MAX)
	{
		AddCounter(ctx, eIA_i_LightRadius, light_turns, light_radius);
		ReCalcPlayerVision(ctx);
	}

	if(cure_poison == true)
	{
		for(i = 0; i < MAX_COUNTER_ITEMS; i++)
		{
			// remove all poison effect counters
			switch(ctx->player->counters[i].effect)
			{
				case eIA_i_Poison1for1__food_only:
				case eIA_i_Poison1for3__food_only:
				case eIA_i_Poison1for5__food_only:
					ctx->player->counters[i].effect = 0;
					break;
			}
		}
	}

	if(GetItemAttributeValue_Bool(item, eIA_b_Unknown, false) == true)
	{
		if(skill_roll(ctx->player->skills[eSkill_ItemLore]) >= skill_roll(GetItemAttributeValue_Int(item, eIA_i_IdentifySkillLevel, 1)))
		{
			// set parent
			uItem *master = FindItemByID(ctx, item->idx);
			// set template item to true now we know it.
			if(master != NULL)
			{
				SetItemAttributeValue_Bool(master, eIA_b_Unknown, true);
			}

			// set child!
			SetItemAttributeValue_Bool(item, eIA_b_Unknown, true);

			// scan all inventory items for this potion
			for(i = 0; i < MAX_INVENTORY_ITEMS; i++)
			{
				if(ctx->player->items[i].item.idx != 0)
				{
					uItem *clone;
					clone = &ctx->player->items[i].item;

					if(clone->idx != 0 && clone->idx == item->idx)
					{
						SetItemAttributeValue_Bool(clone, eIA_b_Unknown, true);
					}
				}
			}

			add_msg(ctx, "AHA! You recognise this as %s", item->name);
			AddExperiece(ctx, 100);
		}
		else
		{
			// no indication of anything happened..
			if(pbase != INT16_MAX)
			{
				add_msg(ctx, "You drank!");
			}
			else
			{
				add_msg(ctx, "You ate!");
			}
		}
	}
	else
	{
		p = strchr(msgx, 0);
		if(strlen(msgx) > 1)
		{
			p -= 1;
			if(*p == ',')
			{
				*p = 0;
			}
		}

		add_msg(ctx, "A %s, %s", GetItemName(item), msgx);
	}

	if(fortune == true)
	{
		add_msg(ctx, "There is a scrap of paper with your fotune on it, it reads;");
		add_msg(ctx, "  \"%s\"", fortune_text);

		if(inside_inventory == true)
		{
			toggle_swap_draw_to_backbuffer();
			msg(screen_get_height() - 3, 2, 0x0E, "There is a scrap of paper with your fotune on it, it reads;");
			msg(screen_get_height() - 2, 2, 0x0E, "  \"%s\"", fortune_text);
			msg(screen_get_height() - 1, (screen_get_width() - 17) / 2, 7, "-- press any key --");
			toggle_swap_draw_to_backbuffer();
			restore_from_backbuffer();
			gkey(NULL);
		}
	}


	// consume consumables...
	delete_item = true;

	return delete_item;
}

bool reconstruct_item_attributes(uItem *item, const char *str)
{
	bool rc = true;
	char *p;
	char *q;
	uint16_t j;
	uint16_t attr_idx;

	while(rc == true && str != NULL && *str != 0)
	{
		str = unspace((char *)str);
		// attr id ":" values ... ";"
		p = strchr(str, ':');
		if(p != NULL)
		{
			attr_idx = strtoul(str, &q, 10);
			if(p == q)
			{
				str = p + 1;

				for(j = 0; j < MAX_ITEM_ATTRIBUTES; j++)
				{
					if(item->attr[j].type == attr_idx)
					{
						item->attr[j].modified = true;

						switch(item->attr[j].attr_type)
						{
							case eAttr_i16Value:
								// num;
								item->attr[j].attributes.i.value = (int16_t)strtol(str, NULL, 10);
								break;

							case eAttr_Bool:
								// t|f;
								item->attr[j].attributes.b.value = false;
								if(*str == 't')
								{
									item->attr[j].attributes.b.value = true;
								}
								break;

							case eAttr_Key_i16Value:
								// num,num;
								item->attr[j].attributes.kv.key = (uint16_t)strtoul(str, NULL, 10);
								str = strchr(str, ',');
								if(str != NULL)
								{
									str += 1;
									item->attr[j].attributes.kv.value = (int16_t)strtol(str, NULL, 10);
								}
								else
								{
									rc = false;
								}
								break;
							case eAttr_Dice:
								// num,num,num;
								item->attr[j].attributes.die.faces = (int8_t)strtol(str, NULL, 10);
								str = strchr(str, ',');
								if(str != NULL)
								{
									str += 1;
									item->attr[j].attributes.die.num = (int8_t)strtol(str, NULL, 10);
								}
								else
								{
									rc = false;
								}

								if(str != NULL)
								{
									str = strchr(str, ',');
									if(str != NULL)
									{
										str += 1;
										item->attr[j].attributes.die.plus = (int8_t)strtol(str, NULL, 10);
									}
									else
									{
										rc = false;
									}
								}
								else
								{
									rc = false;
								}
								break;
							case eAttr_Range:
								// num,num;
								item->attr[j].attributes.r.min = (int16_t)strtol(str, NULL, 10);
								str = strchr(str, ',');
								if(str != NULL)
								{
									str += 1;
									item->attr[j].attributes.r.max = (int16_t)strtol(str, NULL, 10);
								}
								else
								{
									rc = false;
								}
								break;
						}

						j = MAX_ITEM_ATTRIBUTES + 1;
					}
				}

				if(str != NULL)
				{
					str = strchr(str, ';');
					if(str != NULL)
					{
						str += 1;
					}
					else
					{
						rc = false;
					}
				}
				else
				{
					rc = false;
				}

				// str now points past attribute!
			}
			else
			{
				rc = false;
			}
		}
		else
		{
			rc = false;
		}
	}

	return rc;
}


bool deconstruct_item_attribute(uItem *item, char *out)
{
	char *p;
	bool rc = false;
	uint16_t j;

	*out = 0;

	if(item->idx == 0)
	{
		return false;
	}

	for(j = 0; j < MAX_ITEM_ATTRIBUTES; j++)
	{
		if(item->attr[j].modified == true)
		{
			rc = true;

			p = strchr(out, 0);
			sprintf(p, "%"PRIu8":", item->attr[j].type);
			p = strchr(out, 0);

			switch(item->attr[j].attr_type)
			{
				case eAttr_i16Value:
					sprintf(p, "%"PRIi16"", item->attr[j].attributes.i.value);
					break;
				case eAttr_Bool:
					sprintf(p, "%c", item->attr[j].attributes.b.value == true ? 't' : 'f');
					break;
				case eAttr_Key_i16Value:
					sprintf(p, "%"PRIu16",%"PRIi16"", item->attr[j].attributes.kv.key, item->attr[j].attributes.kv.value);
					break;
				case eAttr_Dice:
					sprintf(p, "%"PRIi8",%"PRIi8",%"PRIi8"", item->attr[j].attributes.die.faces, item->attr[j].attributes.die.num, item->attr[j].attributes.die.plus);
					break;
				case eAttr_Range:
					sprintf(p, "%"PRIi16",%"PRIi16"", item->attr[j].attributes.r.min, item->attr[j].attributes.r.max);
					break;
			}

			strcat(out, ";");
		}
	}

	return rc;
}
