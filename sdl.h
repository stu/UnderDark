#ifndef SDL_H
#define SDL_H
#ifdef __cplusplus
extern "C"{
#endif


typedef struct udtXGLImage
{
	SDL_Texture *texture;
	int32_t pow_w;
	int32_t pow_h;
	int32_t real_w;
	int32_t real_h;
} uXGLImage;

extern void PrintText(uint16_t row, uint16_t col, uint8_t *str, uint8_t colour);
extern void PrintChar(uint16_t row, uint16_t col, uint8_t c, uint8_t colour);
extern int32_t window_get_width(void);
extern int32_t window_get_height(void);
extern int32_t get_font_height(void);
extern int32_t get_font_width(void);

extern SDL_Renderer *g_renderer;
extern SDL_Window *g_screen;
extern const uint32_t dos_colours[16];

#ifdef __cplusplus
};
#endif
#endif        //  #ifndef SDL_H
