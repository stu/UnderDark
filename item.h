#ifndef ITEM_H
#define ITEM_H
#ifdef __cplusplus
extern "C"{
#endif

extern char* GetItemName(uItem *i);
extern void FreeItem(void *x);
extern uint16_t BuildItems(uContext *ctx);
extern uItem *FindItemByName(uContext *ctx, char *name);
extern uItem *FindItemByID(uContext *ctx, uint16_t id);
extern uint16_t FindItemSlotInInventoryByID(uContext *ctx, uint16_t id);
extern uItem* AddItemToInventory(uContext *ctx, uItem *i, int16_t count);
extern void GenerateCopy(uItem *src, uItem *dest);

extern uint16_t EquipItem(uContext *ctx, uItem *item, bool in_inventory_screen);
extern uint16_t UnEquipItem(uContext *ctx, uItem *item, bool in_inventory_screen, bool ignore_cursed);
extern void ProcessInventoryItemTriggers(uContext *ctx);

extern bool IsAlreadyEquipped(uContext *ctx, uItem *item);
extern bool IsAlreadyUnEquipped(uContext *ctx, uItem *item);

extern uAttribute *GetItemAttribute(uItem *i, uint16_t type);
extern uint16_t AddItemAttribute_KV(uItem *i, uint16_t type, uint16_t key, int16_t value);
extern uint16_t AddItemAttribute_Bool(uItem *i, uint16_t type, bool value);
extern void SetItemAttributeValue_Bool(uItem *i, uint16_t type, bool value);
extern uint16_t AddItemAttribute_Int(uItem *i, uint16_t type, int16_t value);
extern void SetItemAttributeValue_Int(uItem *i, uint16_t type, int16_t value);
extern uint16_t AddItemAttribute_Range(uItem *i, uint16_t type, int16_t min, int16_t max);
extern uint16_t AddItemAttribute_Die(uItem *i, uint16_t type, int8_t num, int8_t faces, int8_t plus);

extern void SetItemName(uItem *item, char *name);

extern int16_t GetItemAttributeValue_Die(uItem *i, uint16_t type, int16_t def);
extern int16_t GetItemAttributeValue_KV(uItem *i, uint16_t type, uint16_t key, int16_t def);
extern uAttribute* GetItemAttributeValue_KV_attribute(uItem *i, uint16_t type);
extern bool GetItemAttributeValue_Bool(uItem *i, uint16_t type, bool def);
extern int16_t GetItemAttributeValue_Int(uItem *i, uint16_t type, int16_t def);


extern void AddCounter(uContext *ctx, uint16_t effect, uint16_t rounds, int16_t adjustment);
extern void ClearCounter(uContext *ctx, uint16_t effect);
extern uint16_t GetFreeItemSlot(uContext *ctx);

extern uItem* GetPlayerItemLeftHand(uContext *ctx);
extern uItem* GetPlayerItemRightHand(uContext *ctx);
extern uItem* GetPlayerItemWeaponLeft(uContext *ctx);
extern uItem* GetPlayerItemWeaponRight(uContext *ctx);
extern uItem* GetPlayerItemBody(uContext *ctx);
extern uItem* GetPlayerItemHead(uContext *ctx);

extern void ClearRightWeapon(uContext *ctx);
extern void ClearLeftWeapon(uContext *ctx);
extern void SwapLoadout(uContext *ctx, bool in_inventory_screen);
extern bool Consume(uContext *ctx, uItem *item, bool inside_inventory);
extern void DeleteInventoryItem(uContext *ctx, uItem *item);
extern void ProcessInventoryTurnItemTriggers(uContext *ctx);

extern bool reconstruct_item_attributes(uItem *item, const char *str);
extern bool deconstruct_item_attribute(uItem *item, char *out);

#ifdef __cplusplus
};
#endif
#endif        //  #ifndef ITEM_H
