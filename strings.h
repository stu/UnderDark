#ifndef STRINGS_H
#define STRINGS_H
#ifdef __cplusplus
extern "C"{
#endif

extern bool bit_get_bit(uint8_t *ptr, size_t pos);
extern void bit_set_bit(uint8_t *ptr, size_t pos, bool state);

extern bool only_numbers(char *s);
extern uint32_t get_number_in_string(char *s, int16_t len);
extern bool StringEndsInS(char *s);
extern char *unspace(char *s);

#ifdef __cplusplus
};
#endif
#endif        //  #ifndef STRINGS_H
