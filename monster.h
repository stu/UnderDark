#ifndef MONSTER_H
#define MONSTER_H
#ifdef __cplusplus
extern "C"{
#endif

extern uMapMonster *GetMonsterOnMap(uContext *ctx, uint16_t row, uint16_t col);
extern uMonster *GetMonsterByName(uContext *ctx, char *name);
extern uMonster *GetMonsterByID(uContext *ctx, uint16_t id);
extern uMapMonster* BuildMonster(uContext *ctx, uMonster *m, uint16_t r, uint16_t c);

extern void FreeMonster(void *x);
extern int16_t BuildMonsters(uContext *ctx);

extern void KillMonster(uContext *ctx, uMapMonster *mm);

#ifdef __cplusplus
};
#endif
#endif        //  #ifndef MONSTER_H
