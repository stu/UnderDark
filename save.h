#ifndef SAVE_H
#define SAVE_H
#ifdef __cplusplus
extern "C"{
#endif

#define FILE_PLAYER	"z_player.sav"
#define FILE_CORE "z_core.sav"

extern void SaveMap(uContext *ctx);
extern void SaveGame(uContext *ctx);

#ifdef __cplusplus
};
#endif
#endif        //  #ifndef SAVE_H
