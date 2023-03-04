#ifndef MESSAGE_H
#define MESSAGE_H
#ifdef __cplusplus
extern "C"{
#endif

extern void FreeMessage(void *message);
extern void add_msg(uContext *ctx, char *strX, ...);
extern void update_last_message(uContext *ctx, char *strX, ...);

#ifdef __cplusplus
};
#endif
#endif        //  #ifndef MESSAGE_H
