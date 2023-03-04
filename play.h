#ifndef PLAY_H
#define PLAY_H
#ifdef __cplusplus
extern "C"{
#endif

extern const uint8_t *ascii_race_list[];

extern void main_screen(uContext *ctx);
extern void calc_draw_offseet(uContext *ctx);
extern int16_t roll_dice(int16_t num_dice, int16_t num_faces, int16_t plus);
extern int16_t skill_roll(int16_t level);
extern void AddExperiece(uContext *ctx, uint16_t exp);
extern bool line_los(uContext *ctx, int16_t col1, int16_t row1, int16_t col2, int16_t row2);
extern void reset_los_square(void);
extern int16_t add_player_damage(uContext *ctx, int16_t d);

#ifdef __cplusplus
};
#endif
#endif        //  #ifndef PLAY_H
