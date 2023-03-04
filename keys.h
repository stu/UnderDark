#ifndef KEYS_H
#define KEYS_H
#ifdef __cplusplus
extern "C"{
#endif

/////////////////////////////////////////////////////////////////////

extern void SetDefaultKeys(uContext *ctx);
extern uint16_t SetupKey(uContext *ctx, char *key, char *value);
extern uint16_t ProcessKeybinds(uContext *ctx);

extern void KeyGetInit(uKeyGet *k);
extern void KeyGetAdd8(uKeyGet *k, uint8_t c);
extern void KeyGetAdd16(uKeyGet *k, uint16_t c);
extern uint16_t KeyGetKey(uKeyGet *k);
extern char *KeyGetToString(uKeyGet *k);
extern char* DirectionalAscii(uint16_t k);
extern uint16_t DOSKeyToInternal(uContext *ctx, uint16_t k);

extern void ConfigureKeys(uContext *ctx);

#ifdef __cplusplus
};
#endif
#endif        //  #ifndef KEYS_H
