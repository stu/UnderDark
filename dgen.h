#ifndef DGEN_H
#define DGEN_H
#ifdef __cplusplus
extern "C"{
#endif

extern uMapTrigger* GetMapTrigger(uContext *ctx, uint16_t r, uint16_t c);
extern bool DescendLevel(uContext *ctx);
extern void GenerateMap(uContext *ctx, uint16_t map, uint16_t level, bool at_stairs);
extern void FreeMap(uContext *ctx);
extern void FreeMapMonster(void *x);
extern void FreeMapTrigger(void *x);
extern void FreeMapItem(void *x);
extern void ClearMap(uContext *ctx);


extern uint8_t get_cell_tile(uContext *ctx, uint16_t r, uint16_t c);
extern uint8_t get_cell_flags(uContext *ctx, uint16_t r, uint16_t c);
extern void or_cell_flags(uContext *ctx, uint16_t r, uint16_t c, uint8_t flags);
extern void set_cell_flags(uContext *ctx, uint16_t r, uint16_t c, uint8_t flags);
extern void set_cell_tile(uContext *ctx, uint16_t r, uint16_t c, uint8_t tile);


extern void dig_cell(uContext *ctx, uint16_t r, uint16_t c, uint8_t tile_type, uint8_t tile_flags);

extern void ProcessMapTriggers(uContext *ctx, uint16_t r, uint16_t c);

extern uMapItem *GetMapItem(uContext *ctx, uint16_t r, uint16_t c);
extern void DeleteItem(uContext *ctx, uMapItem *item);

extern void add_monster(uContext *ctx);

extern void GenWin(uContext *ctx, uint16_t row, uint16_t col);
extern void GenItemByLevel(uContext *ctx, uint16_t row, uint16_t col, uint16_t flags, uint16_t flags2);
extern void GenSmallGold(uContext *ctx, uint16_t row, uint16_t col);
extern void GenLargeGold(uContext *ctx, uint16_t row, uint16_t col);

#ifdef __cplusplus
};
#endif
#endif        //  #ifndef DGEN_H
