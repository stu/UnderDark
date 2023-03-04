#ifndef SAVE_EXT_H
#define SAVE_EXT_H
#ifdef __cplusplus
extern "C"{
#endif

extern bool verify_file(char *fn);
extern void ClearSaveFiles(void);
extern void SetLoadSaveStatus(bool loading, char *s);

extern void encode64(const uint8_t *str, uint16_t slen, char *buff, int16_t bufflen);
extern void decode64(const char *str, uint8_t *buff, int16_t bufflen);

extern bool load_rle(INIFILE *ini, char *group, char *key, uint8_t *out_buff, int16_t out_len);
extern bool load_ue64(INIFILE *ini, char *group, char *key, uint8_t *out_buff, int16_t out_len);
extern uint8_t get_u8(INIFILE *ini, char *group, char *key, bool *rc);
extern int16_t get_i16(INIFILE *ini, char *group, char *key, bool *rc);
extern uint16_t get_u16(INIFILE *ini, char *group, char *key, bool *rc);
extern uint32_t get_u32(INIFILE *ini, char *group, char *key, bool *rc);

extern void save_str(INIFILE *ini, char *group, char *key, uint8_t *s);
extern void save_u8(INIFILE *ini, char *group, char *key, uint8_t value);
extern void save_i16(INIFILE *ini, char *group, char *key, int16_t value);
extern void save_u16(INIFILE *ini, char *group, char *key, uint16_t value);
extern void save_u32(INIFILE *ini, char *group, char *key, uint32_t value);

extern void unpack_rle(const uint8_t *src, uint16_t dlen, uint8_t *dest);
extern uint16_t rle_pack_line(uint8_t *dest, uint16_t dest_len, uint8_t *src, uint16_t src_len);

#ifdef __cplusplus
};
#endif
#endif        //  #ifndef SAVE_EXT_H
