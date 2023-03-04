#ifndef MAIN_H
#define MAIN_H
#ifdef __cplusplus
extern "C"{
#endif

#define CONFIG_FILE "udark.ini"
#define MSG_BUFFER_LEN 400
extern char *__msg_buffer;
extern bool workstationCactus;

extern void msg(uint16_t row, uint16_t col, uint16_t cA, char *strX, ...);
extern void msg_hl(uint16_t row, uint16_t col, uint16_t cA, uint16_t cB, char *strX, ...);
extern void RollPlayer(uContext *ctx, bool first_time);
extern char* GetConfigurationFile(char *fn);
extern char* GetDataFile(char *fn);
extern void TestConfigFile(void);

extern void ClearPlayer(uContext *ctx);

extern void SetStatusRow(uContext *ctx);
extern uContext *NewContext(void);
extern void ReleaseContext(uContext *ctx);
extern bool IsFirstTime(void);

#ifdef __cplusplus
};
#endif
#endif        //  #ifndef MAIN_H
