#ifndef STRUCTS_H
#define STRUCTS_H
#ifdef __cplusplus
extern "C"{
#endif

#define HL_CLR_LO        0x03
#define HL_CLR_HI        0x0B

#define HL_CLR_LO_ALT    0x02
#define HL_CLR_HI_ALT    0x0A

#define HL_CLR        HL_CLR_LO,HL_CLR_HI
#define HL_CLR_ALT    HL_CLR_LO_ALT,HL_CLR_HI_ALT


#define MAX_MAP_WIDTH                180
#define MAX_MAP_HEIGHT               180

#define DEFAULT_MAX_MESSAGES         50
#define MAX_NAME_LEN                 24
#define MAX_DEATH_MESSAGE_LEN        128
#define MAX_LOCATION_LEN             32
#define MAX_BUILD_LEN                32

#define MAX_MONSTER_FOV              128
#define MAX_ITEM_NAME                32
#define MAX_MONSTER_NAME             24
#define ITEM_TURNS_MAX               INT16_MAX-1

#define TILE_FLAG_LIT                0x80
#define TILE_FLAG_OPAQUE             0x40
#define TILE_FLAG_SEEN               0x02
#define TILE_FLAG_VISITED            0x01

enum eKeys
{
	eKey_Up,
	eKey_Down,
	eKey_Left,
	eKey_Right,
	eKey_UpRight,
	eKey_UpLeft,
	eKey_DownRight,
	eKey_DownLeft,

	eKey_Inventory,
	eKey_PassTurn,
	eKey_Ascend,
	eKey_Descend,
	eKey_Open,
	eKey_Close,
	eKey_Pickup,
	eKey_Target,
	eKey_Fire,
	eKey_LoadOutToggle,

	eKey_Version,
	eKey_Cancel,
	eKey_Quit,
	eKey_Save,

	eKey_MAX
};

enum eFlags
{
	eFlag_GenLevel0 = 0,
	eFlag_GenLevel_Bottom = 24, // 25 levels

	// did you win the game? (not make it out but hit end requirement)
	eFlag_GameWin,

	eFLAGS_MAX
};

enum eTileTypes
{
	eTile_Void = 0,
	eTile_RoughRockWall,
	eTile_SmoothStoneWall,
	eTile_StoneFloor,
	eTile_ClosedDoor,
	eTile_OpenDoor,
	eTile_LockedClosedDoor,
	eTile_StairsUp,
	eTile_StairsDown,
	eTile_Trap,

	eTile_MAX
};

enum eTrapType
{
	eT_DartTrap,
	eT_SpikeTrap,
	eT_SpoonTrap,
	eT_BearTrap,
	eT_Teleport,
	eT_Spiderweb,
	eT_TrapDoor,
	eT_Rockfall,
	eT_MAX
};

// remmeber to put code into save.c / load.c!
enum eDroppedType
{
	eDT_Gold,
	eDT_Weapon,
	eDT_Armour,
	eDT_Potion,
	eDT_Food,
	eDT_Item,
};

enum eMonsterDisposition
{
	eMD_HostileChasing,
	eMD_HostileWatching,
	eMD_HostileShambling,
	eMD_HostileCantMove,

	eMD_DocileChasing,
	eMD_DocileWatching,
	eMD_DocileCantMove,
};

enum eMonsterAttackType
{
	// ascii in play.c
	eMAT_Bite,
	eMAT_Weapon,
	eMAT_RangedWeapon,
	eMAT_PoisonBite1,
	eMAT_PoisonBite2,
	eMAT_LifeDrain1,
	eMAT_LifeDrain2,
	eMAT_MagicUser,
	eMAT_Acid1,
	eMAT_Acid2,
	eMAT_StealMoney1,
	eMAT_StealMoney2,
	// unimplemented
	eMAT_Claw,
	eMAT_Spit,
	eMAT_Gaze,
	eMAT_Sting,

};

enum eMonsterType
{
	eMT_Bee,
	eMT_Blob,
	eMT_BloodyBigRat,
	eMT_Centipede,
	eMT_Demon,
	eMT_Goblin,
	eMT_HellHound,
	eMT_Human,
	eMT_Kobold,
	eMT_Leprechaun,
	eMT_LizardMan,
	eMT_Mimic,
	eMT_Minotaur,
	eMT_Mummy,
	eMT_Orc,
	eMT_OwlBear,
	eMT_Rat,
	eMT_Scorpion,
	eMT_Snake,
	eMT_Spider,
	eMT_Troll,
	eMT_Vampire,
	eMT_Yeti,
	eMT_Zombie,

	eMT_MAX
};

enum eSpellType
{
	eST_MagicMissile,
	eST_ScatterShot,
	eST_Cloud,
	eST_AOE,
};

// adding to this list, change in item.c to name unknown potions etc
enum ePotionBase
{
	ePB_Potion_Heal1,        // small
	ePB_Potion_Heal2,        // medium
	ePB_Potion_Heal3,        // large

	// refill mana
	ePB_ManaRefill1,
	ePB_ManaRefill2,
	ePB_ManaRefill3,
	ePB_ManaRefill4,

	ePB_Water,

	ePB_Poisoned,
};

enum eFoodBase
{
	eFB_HealthyFood1,
};

#define EPB_NAME_HEAL1 "potion minor healing"
#define EPB_NAME_HEAL2 "potion healing"
#define EPB_NAME_HEAL3 "potion major healing"
#define EPB_NAME_MANA_REFILL1 "potion mana refill"
#define EPB_NAME_MANA_REFILL2 "potion boost mana"
#define EPB_NAME_MANA_REFILL3 "potion extra mana"
#define EPB_NAME_MANA_REFILL4 "potion full mana"
#define EPB_NAME_WATER "bottle of water"
#define EPB_NAME_POI1 "bottle of insectacide"
#define EPB_NAME_CURE_POI1 "bottle of cure poison"

#define ITEM_NAME_TORCH "torch"
#define ITEM_NAME_BRASS_LANTERN "brass lantern"
#define ITEM_NAME_SHORT_SWORD "short sword"
#define ITEM_NAME_LONG_SWORD "long sword"
#define ITEM_NAME_SHORT_BOW "short bow"
#define ITEM_NAME_AXE "axe"
#define ITEM_NAME_ARROWS "arrows"
#define ITEM_NAME_PADDED_ARMOUR "padded armour"
#define ITEM_NAME_LEATHER_ARMOUR "leather armour"
#define ITEM_NAME_STUDDED_ARMOUR "studded leather armour"

#define ITEM_NAME_SKINNING_KNIFE "skinning knife"
#define ITEM_NAME_TANTO_KNIFE "tanto knife"
#define ITEM_NAME_WAND_MISSILES "wand of missiles"
#define ITEM_NAME_RING_PROTECTION "ring of protection"
#define ITEM_NAME_PURPLE_SPANDEX "purple spandex robe"

#define ITEM_NAME_CROWN_DARKNESS "crown of darkness"

// update monster.c for drops
enum eDropClass
{
	eDC_Nothing,

	eDC_SmallMoney,
	eDC_LargeMoney,

	eDC_LowWeaponArmour,
	eDC_MidWeaponArmour,
	eDC_HighWeaponArmour,

	eDC_LowMagicWeaponArmour,
	eDC_MidMagicWeaponArmour,
	eDC_HighMagicWeaponArmour,

	eDC_Win,

	eDC_MAX
};

#define GEARDROP_L_ARMOUR		(1<<0)
#define GEARDROP_M_ARMOUR		(1<<1)
#define GEARDROP_H_ARMOUR		(1<<2)
#define GEARDROP_L_WEAPON		(1<<3)
#define GEARDROP_M_WEAPON		(1<<4)
#define GEARDROP_H_WEAPON		(1<<5)
#define GEARDROP_MAGIC			(1<<6)
#define GEARDROP_TREASURE		(1<<7)

enum eItemAttribute
{
	eIA_None = 0,

	/////////////////////////////////////////////////////////////////////
	/// dice values

	// dies for doing damage (on hit)
	eIA_d_DoDamage,
	eIA_d_DoRangedDamage,

	// dice for doing healing (drink? hit?)
	eIA_d_DoHeal,

	// add to mana pool
	eIA_d_AddMana,

	/////////////////////////////////////////////////////////////////////
	/// i16 values

	// remember to update ProcessInventoryTurnItemTriggers for any new counter effects
	// and AddCounter
	eIA_i_____CounterEffectStart,

	// adjust armour
	eIA_i_ArmourRating,

	// adjust max mana
	eIA_i_ManaAdjust,

	// adjust max speed
	eIA_i_StatAdjust_Speed,


	// number of turns light is emitted (eg: torch)
	eIA_i_LightTurns,

	// radius item emits light to n for turns (not to be confused with eIA_i_LightTurns)
	// this is an absolute, not a +/-.
	eIA_i_LightRadius,

	eIA_i_LifeBoost,

	// poison status effects. 1hp per 5 moves
	// added only to consumable items not to weapons/armour!!
	// these apply status counter effects
	eIA_i_Poison1for5__food_only,
	eIA_i_Poison1for3__food_only,
	eIA_i_Poison1for1__food_only,

	// for spiderwebs/beartraps/etc. anything to hold in place.
	eIA_i_HeldInPosition,

	eIA_i_____CounterEffectEnd,

	// this goes to its usage
	eIA_i_PotionBase,

	// this goes to its usage
	eIA_i_FoodBase,

	// what spell to cast
	eIA_i_CastsSpell,

	// how much east cast cost
	eIA_i_CastManaCost,

	// how easy to identify what it is
	eIA_i_IdentifySkillLevel,

	// drop class for item
	eIA_i_DropClass,

	/////////////////////////////////////////////////////////////////////
	// kv values

	// a vs on hit to a monster, KEY monster type, VALUE bonus
	eIA_kv_VsMonsterType,

	// minimum stat KEY required VALUE
	eIA_kv_MinStatRequired,

	// minimum stat KEY required VALUE
	eIA_kv_MinSkillRequired,

	// potion names!
	eIA_kv_PotionRandomName,
	// food names!
	eIA_kv_FoodRandomName,

	// magic colour/character
	eIA_kv_MagicColour,


	/////////////////////////////////////////////////////////////////////
	/// bool values

	// potion has not been identified yet. change to generic unknown.
	//eIA_b_UnknownPotionName,

	// item has a unique name
	eIA_b_Named,

	// if its rare, itemlore can identify it
	eIA_b_Rare,


	// unique, can only be one of these items.
	// duping a unique destroys both
	// item lore can identify it
	eIA_b_Unique,

	// emits light
	eIA_b_IsLit,

	// when you equip it, emits light
	eIA_b_IsLitWhenWorn,

	// after eating it? torch burns out? etc. destroy item.
	eIA_b_DestroyedWhenDone,

	// can be stacked (arrows).
	eIA_b_Stackable,

	// when potion gets used, flag shows empty. dont display base name. show empty
	// when making something empty, remove all other attributes on item!
	eIA_b_Empty,

	// sets an item to be cursed
	eIA_b_Cursed,

	// if the curse is known (ie: shows up in inventory as cursed)
	// goes hand in hand with eIA_b_Cursed
	eIA_b_CursedKnown,

	// unknown item
	eIA_b_Unknown,

	// displays a fortune?
	eIA_b_GiveFortune,

	// cures poison status
	eIA_b_CurePoison,

	// for fireing at target (bow, wand, etc)
	eIA_b_FireTargetBow,
	eIA_b_FireTargetWand,
	eIA_b_Reflective,

};

enum eItemType
{
	eIT_1h_Weapon,
	eIT_2h_Weapon,
	eIT_Ring,
	eIT_Helmet,
	eIT_BodyArmour,
	eIT_Shield,
	eIT_AmmunitionForBow,
	eIT_Torch,
	eIT_Consumable,
};

enum eSkills
{
	eSkill_MeleeWeapons,
	eSkill_RangedWeapons,
	eSkill_Magic,
	eSkill_ItemLore,

	eSkill_MAX
};

enum eStats
{
	eStat_Strength,
	eStat_Speed,
	eStat_Dexterity,
	eStat_Willpower,
	eStat_DeepVision,
	eStat_MAX
};

enum eRace
{
	// zeroes across the board
	eRace_Human,
	eRace_Elf,
	eRace_Dwarf,
	eRace_Orc,
	eRace_Gnoll,

	eRace_MAX
};

enum eAttrType
{
	eAttr_Bool,
	eAttr_i16Value,
	eAttr_Range,
	eAttr_Key_i16Value,
	eAttr_Dice
};

enum eReturn
{
	eReturn_Bad = 0,
	eReturn_OK,
	eReturn_ItemCursed,
	eReturn_DontMeetStatRequirement_Strength,
	eReturn_DontMeetStatRequirement_Speed,
	eReturn_DontMeetStatRequirement_Dexterity,
	eReturn_DontMeetStatRequirement_Willpower,
	eReturn_DontMeetStatRequirement_DeepVision,
	eReturn_DontMeetSkillRequirement_Meelee,
	eReturn_DontMeetSkillRequirement_Ranged,
	eReturn_DontMeetSkillRequirement_Magic,
	eReturn_DontMeetSkillRequirement_ItemLore,
	eReturn_NoEmptyItemSlot,
	eReturn_AlreadyEquipped,
	eReturn_ItemTypeAlreadyEquipped,

};

typedef struct udtMagic
{
	int a;
} uMagic;

typedef struct udtMessage
{
	uint8_t count;
	char *msg;
} uMessage;

typedef struct udtAttribute
{
	uint8_t type;
	uint8_t attr_type;
	bool modified;
	union
	{
		struct
		{
			bool value;
		} b;
		struct
		{
			int16_t value;
		} i;
		struct
		{
			int16_t min;
			int16_t max;
		} r;
		struct
		{
			uint16_t key;
			int16_t value;
		} kv;
		struct
		{
			int8_t num;
			int8_t faces;
			int8_t plus;
		} die;
	} attributes;
} uAttribute;

typedef struct udtMonster
{
	uint16_t idx;
	uint8_t name[MAX_MONSTER_NAME + 1];
	uint8_t type;
	uint8_t tile;
	uint8_t colour;
	int16_t life;
	int16_t armour;
	int16_t xp;
	uint8_t attack_level;
	uint8_t defend_level;
	uint8_t disposition;
	uint8_t attack_type;
	uint8_t attack_dice[3];
	uint8_t drop_class;
	uint8_t min_level;
	uint8_t max_level;
} uMonster;

#define MAX_ITEM_ATTRIBUTES        10
typedef struct udtItem
{
	uint16_t idx;
	uint8_t name[MAX_ITEM_NAME + 1];
	uint8_t base_type;
	uAttribute attr[MAX_ITEM_ATTRIBUTES];
} uItem;

#define MAX_INVENTORY_ITEMS    16
typedef struct udtInventory
{
	uint8_t count;
	uItem item;
} uInventory;

#define MAX_COUNTER_ITEMS    8
typedef struct udtCounter
{
	uint16_t rounds;
	// only eIA_i affects
	uint16_t effect;
	// do opposite of what was done.
	// eg: if potion did +10 max mana,
	// adjust would be 10, we always do a -= for adjust
	int16_t adjust;
} uCounter;


// 2 bytes * 160*160 is < 64kb!
typedef struct udtTile
{
	uint8_t flags;
	uint8_t tile;
} uTile;

typedef struct udtLocation
{
	uint8_t map;
	uint8_t level;
	uint8_t row;
	uint8_t col;
} uLocation;

typedef struct uMapCell
{
	uTile tile;
} uMapCell;


// rememeber to mod save.c
enum eTriggerTypes
{
	eTrig_None,
	eTrigger_SignOnWall,
	eTrigger_Trap,
};

typedef struct uMapTrigger
{
	uint8_t row;
	uint8_t col;
	uint8_t type;

	union
	{
		struct
		{
			char *msg;
		} t_sign_on_wall;

		struct
		{
			uint8_t type;
			// for teleport
			uint8_t dest_row;
			uint8_t dest_col;
		} t_trap;
	} data;
} uMapTrigger;


typedef struct uMapMonster
{
	uint8_t row;
	uint8_t col;

	uint16_t monster_id;
	uint16_t gen_id;

	// things that can change....
	uint8_t disposition;
	int16_t health;
	int16_t armour;

	uint8_t last_player_row;
	uint8_t last_player_col;
} uMapMonster;

// see: eDroppedType
typedef struct udtMapItem
{
	uint8_t row;
	uint8_t col;
	uint8_t type;

	union
	{
		struct
		{
			uint16_t idx;
			int16_t count;
			bool has_item_data;
			uItem item;
		} i_item;
		struct
		{
			int16_t gold;
		} i_gold;
	} data;
} uMapItem;

typedef struct udtMap
{
	uint8_t max_rows;
	uint8_t max_cols;
	uint8_t recovery;
	uMapCell cells[MAX_MAP_WIDTH * MAX_MAP_HEIGHT];
	DList *lstItems;
	DList *lstTriggers;
	DList *lstMonsters;
} uMap;

typedef struct udtKeyGet
{
	uint8_t keys8[32];
	uint16_t keys16[16];
	int k8_idx;
	int k16_idx;
	uint16_t value;
} uKeyGet;

#define EMPTY_ITEM_SLOT        UINT8_MAX
#define TARGET_MYSELF			(UINT16_MAX - 1)

typedef struct udtPlayer
{
	uint8_t name[MAX_NAME_LEN + 1];
	uint32_t score;

	uint8_t race;

	uint16_t money;

	int16_t armour;

	uint8_t level;
	uint16_t experience;

	uint8_t level_up_points;

	int16_t life;
	int16_t max_life;

	int16_t mana;
	int16_t max_mana;

	uint8_t stats[eStat_MAX];
	uint8_t base_stats[eStat_MAX];

	uint8_t loadout;

	uint8_t item_head;
	uint8_t item_body;
	uint8_t item_left_weap;
	uint8_t item_right_weap;
	uint8_t item_left_hand;
	uint8_t item_right_hand;

	uint8_t loadout_item_head;
	uint8_t loadout_item_body;
	uint8_t loadout_item_left_weap;
	uint8_t loadout_item_right_weap;
	uint8_t loadout_item_left_hand;
	uint8_t loadout_item_right_hand;

	// selected monster target
	uint16_t target_idx;

	uint8_t monster_fov_selected_idx;
	uint8_t monster_fov_idx;
	uint16_t monster_fov[MAX_MONSTER_FOV];

	uCounter counters[MAX_COUNTER_ITEMS];
	uInventory items[MAX_INVENTORY_ITEMS];
	uint8_t skills[eSkill_MAX];
} uPlayer;

typedef struct udtScore
{
	uint16_t id;
	uint8_t name[MAX_NAME_LEN + 1];
	uint32_t score;
	uint8_t message[MAX_DEATH_MESSAGE_LEN + 1];
	uint8_t date[32];
	uint8_t location[MAX_LOCATION_LEN + 1];
	int16_t level;
	uint8_t build[MAX_BUILD_LEN + 1];
} uScore;

typedef struct udtContext
{
	uint16_t maxMessages;
	DList *lstMessages;

	uint16_t mgen_count;
	uint8_t status_row;

	uint32_t moves;

	bool auto_get_money;
	bool auto_open_doors;

	uMap *map;
	uLocation locn;

	bool redraw_messages;
	bool redraw_status;
	bool redraw_dungeon;

	uint16_t draw_top_row;
	uint16_t draw_top_col;
	uint16_t draw_row;
	uint16_t draw_col;

	DList *lstItems;
	DList *lstMonsters;

	uint8_t game_flags[(eFLAGS_MAX + 7) / 8];
	uPlayer *player;

	uint16_t keys[eKey_MAX];
} uContext;


#ifdef __cplusplus
};
#endif
#endif        //  #ifndef STRUCTS_H
