#ifndef TIME_H
#define TIME_H
#ifdef __cplusplus
extern "C"{
#endif

extern uint8_t *get_current_datetime(void);
extern uint8_t *datetime_tostring(char *dt);

#ifdef __cplusplus
};
#endif
#endif        //  #ifndef TIME_H
