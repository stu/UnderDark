#include <unistd.h>
#include <sys/types.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_endian.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <string.h>

#include "headers.h"
#include "version.h"

#include "sdl.h"

#include "inc_fnt.h"

#define COMPANY_NAME "com.bloodycactus"
#define APP_NAME "underdark"

#define DICT_TEXT " ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()_+-=`~;|:'\"\\,.<>/?[]{}"

const uint32_t dos_colours[16] = {
	0x000000,    // black
	0x0000BF,    // blue
	0x00BF00,    // green
	0x008B8B,    // cyan
	0xBF0000,    // red
	0xBF3FBF,    // magenta
	0x7B3F00,    // brown
	0xA0A0A0,    // white

	0x404040,    // black
	0x0000FF,    // blue
	0x00FF00,    // green
	0x00FFFF,    // cyan
	0xFF0000,    // red
	0xFF00FF,    // magenta
	0xFFFF00,    // yellow
	0xFFFFFF     // white
};


SDL_Window *g_screen;
SDL_Renderer *g_renderer;
int32_t g_screen_width;
int32_t g_screen_height;

int32_t g_font_size;

char *WindowTitle = "Underdark";

uXGLImage *font[16];

uint32_t g_basicDict[256];
uint16_t g_font_basicWidth;
uint16_t g_font_basicHeight;

char *GetDataFile(char *fn)
{
	char *home;
	char *q;
	size_t len;

	q = NULL;
	home = SDL_GetBasePath();
	if(home != NULL)
	{
		char *p;

		len = strlen(home) + strlen(fn) + 8;
		q = calloc(1, len);

		strcpy(q, home);

		p = strchr(q, 0);
		p -= 1;
		if(*p != '/' && *p != '\\')
		{
			strcat(q, "/");
		}

		strcat(q, fn);

		SDL_free(home);
	}
	else
	{
		q = strdup(fn);
	}

	return q;
}

char *GetConfigurationFile(char *fn)
{
	char *home;
	char *q;
	size_t len;

	q = NULL;
	home = SDL_GetPrefPath(COMPANY_NAME, APP_NAME);
	if(home != NULL)
	{
		char *p;

		len = strlen(home) + strlen(fn) + 8;
		q = calloc(1, len);

		strcpy(q, home);

		p = strchr(q, 0);
		p -= 1;
		if(*p != '/' && *p != '\\')
		{
			strcat(q, "/");
		}

		strcat(q, fn);

		SDL_free(home);
	}
	else
	{
		q = strdup(fn);
	}

	return q;
}

static void clear_rect(uint16_t row, uint16_t col, uint8_t colour)
{
	SDL_Rect rect;
	uint8_t c = colour & 0xF;

	rect.x = col * get_font_width();
	rect.y = row * get_font_height();
	rect.w = get_font_width();
	rect.h = get_font_height();

	SDL_SetRenderDrawColor(g_renderer, (dos_colours[c] >> 16) & 0xFF, (dos_colours[c] >> 8) & 0xFF, (dos_colours[c] & 0xFF), SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(g_renderer, &rect);
}

void PrintChar(uint16_t row, uint16_t col, uint8_t c, uint8_t colour)
{
	SDL_Rect srcRect;
	SDL_Rect dstRect;
	uXGLImage *img;

	img = font[colour & 0xF];

	dstRect.x = col * get_font_width();
	dstRect.y = row * get_font_height();
	dstRect.w = g_font_basicWidth;
	dstRect.h = img->real_h;

	srcRect.y = 0;
	srcRect.w = g_font_basicWidth;
	srcRect.h = img->real_h;
	srcRect.x = (int)g_basicDict[c];

	clear_rect(row, col, colour >> 4);
	SDL_RenderCopy(g_renderer, img->texture, &srcRect, &dstRect);
}

void PrintText(uint16_t row, uint16_t col, uint8_t *str, uint8_t colour)
{
	SDL_Rect srcRect;
	SDL_Rect dstRect;
	uint8_t *it;
	uXGLImage *img;

	img = font[colour & 0xF];

	dstRect.x = col * get_font_width();
	dstRect.y = row * get_font_height();
	dstRect.w = g_font_basicWidth;
	dstRect.h = img->real_h;

	srcRect.y = 0;
	srcRect.w = g_font_basicWidth;
	srcRect.h = img->real_h;

	for(it = str; *it != 0; it++)
	{
		clear_rect(row, col, colour >> 4);
		col += 1;

		srcRect.x = (int)g_basicDict[*it];
		SDL_RenderCopy(g_renderer, img->texture, &srcRect, &dstRect);
		dstRect.x += get_font_width();
	}
}

uXGLImage *SDLImageToGLTexture(SDL_Surface *src)
{
	uXGLImage *glimg;

	glimg = malloc(sizeof(uXGLImage));

	glimg->real_w = src->w;
	glimg->real_h = src->h;
	glimg->pow_w = (int32_t)pow(2, ceil(log(glimg->real_w) / log(2)));
	glimg->pow_h = (int32_t)pow(2, ceil(log(glimg->real_h) / log(2)));

	glimg->texture = SDL_CreateTextureFromSurface(g_renderer, src);
	SDL_SetTextureBlendMode(glimg->texture, SDL_BLENDMODE_BLEND);

	return glimg;
}

void GLDeleteTexture(uXGLImage *g)
{
	if(g == NULL)
	{
		return;
	}

	assert(g != NULL);

	if(g->texture != NULL)
	{
		SDL_DestroyTexture(g->texture);
	}

	free(g);
}


static void CalcDict(uXGLImage *glFont)
{
	uint16_t i;
	uint16_t z;
	uint32_t q;
	uint8_t *text = (uint8_t *)DICT_TEXT;

	z = glFont->real_w / strlen((char *)text);

	g_font_basicWidth = z;
	g_font_basicHeight = glFont->real_h;

	q = 1;
	i = 0;
	while(q != 0)
	{
		q = text[i];
		if(g_basicDict[q] == 0)
		{
			g_basicDict[q] = i * z;
			i += 1;
		}
	}
}

static void test_font_size(void)
{
	TTF_Font *ttf;
	SDL_RWops *rwFont;
	SDL_Color clrFg;
	SDL_Surface *sText;
	INIFILE *ini;
	char *cfgFile;

	/// maaaaybe we need a window before we do ttf?
	//const char *title,int x, int y, int w,int h, Uint32 flags
	int32_t max_fs = 128;

	cfgFile = GetConfigurationFile(CONFIG_FILE);
	assert(cfgFile != NULL);

	ini = INI_load(cfgFile);
	free(cfgFile);
	if(ini != NULL)
	{
		max_fs = INI_get_number(ini, "sdl", "max_font_size", 8, 128, 128);
		INI_unload(ini);
	}

	g_font_size = 1;

	clrFg.r = 0x80;
	clrFg.g = 0x80;
	clrFg.b = 0x80;
	clrFg.a = 0;

	rwFont = SDL_RWFromConstMem(inc_fnt, inc_fnt_SIZE);
	for(g_font_size = max_fs; g_font_size >= 8; g_font_size -= 2)
	{
		rwFont->seek(rwFont, 0, SEEK_SET);
		ttf = TTF_OpenFontRW(rwFont, 0, g_font_size);
		sText = TTF_RenderUTF8_Solid(ttf, "X", clrFg);
		TTF_CloseFont(ttf);

		// hit a min of 80 _AND_ 28!!
		if(g_screen_width / sText->w >= 80 && g_screen_height / sText->h >= 28)
		{
			//g_screen_width = (g_screen_width / sText->w) * sText->w;
			//g_screen_height = (g_screen_height / sText->h) * sText->h;
			g_screen_width = 80 * sText->w;
			g_screen_height = 28 * sText->h;
			printf("screen in fontsize is %u.%u with fs=%u\n", g_screen_width / sText->w, g_screen_height / sText->h, g_font_size);

			SDL_FreeSurface(sText);
			sText = NULL;
			break;
		}

		if(sText != NULL)
		{
			SDL_FreeSurface(sText);
			sText = NULL;
		}
	}

	SDL_RWclose(rwFont);
}

static uXGLImage *BuildFont(int fsize, uint32_t fg_clr)
{
	TTF_Font *ttf;
	SDL_RWops *rwFont;

	char *text = (char *)DICT_TEXT;

	SDL_Color clrFg;
	SDL_Surface *sText;
	uXGLImage *glimg;

	rwFont = SDL_RWFromConstMem(inc_fnt, inc_fnt_SIZE);
	//rwFont = SDL_RWFromFile(fname, "r");
	ttf = TTF_OpenFontRW(rwFont, 0, fsize);

	clrFg.r = (fg_clr >> 16) & 0xFF;
	clrFg.g = (fg_clr >> 8) & 0xFF;
	clrFg.b = (fg_clr >> 0) & 0xFF;
	clrFg.a = 0;

	sText = TTF_RenderUTF8_Solid(ttf, text, clrFg);

	glimg = SDLImageToGLTexture(sText);
	SDL_FreeSurface(sText);

	TTF_CloseFont(ttf);
	SDL_RWclose(rwFont);

	return glimg;
}

static void unload_tileset(void)
{
	int i;

	for(i = 0; i < 16; i++)
	{
		if(font[i] != NULL)
		{
			GLDeleteTexture(font[i]);
		}
	}
}

static void load_tileset(void)
{
	int i;

	for(i = 0; i < 16; i++)
	{
		font[i] = BuildFont(g_font_size, dos_colours[i]);
	}
}


static int eventFilter(void *userData, SDL_Event *e)
{
	switch(e->type)
	{
		case SDL_WINDOWEVENT:
		case SDL_DISPLAYEVENT:
			if(g_renderer != NULL)
			{
				SDL_RenderPresent(g_renderer);
			}
			break;
	}

	// events are added to the queue
	return 1;
}

void wait_ticks(uint16_t ticks)
{
	uint64_t start = SDL_GetTicks64();
	uint64_t qq = ticks * 55;
	qq += 100;

	// dos ticks 18.2 times per second, which is about 1000/55

	// base clock is 18 ticks a second?
	while(SDL_GetTicks64() - start <= qq)
	{}

}

uint16_t screen_get_width(void)
{
	return g_screen_width / g_font_basicWidth;
}

uint16_t screen_get_height(void)
{
	return g_screen_height / g_font_basicHeight;
}

int32_t window_get_width(void)
{
	return g_screen_width;
}

int32_t window_get_height(void)
{
	return g_screen_height;
}

int32_t get_font_width(void)
{
	return g_font_basicWidth;
}

int32_t get_font_height(void)
{
	return g_font_basicHeight;
}

void dump_sdl_info(void)
{
	const char *video_name;
	SDL_RendererInfo rendinfo;

	SDL_version compiled;
	const SDL_version *linked;

	SDL_VERSION(&compiled);
	printf("SDL compiled with v%d.%d.%d\n", compiled.major, compiled.minor, compiled.patch);
	//linked = IMG_Linked_Version();
	//printf("SDL Image running v%d.%d.%d\n", linked->major, linked->minor, linked->patch);
	linked = TTF_Linked_Version();
	printf("SDL TTF running v%d.%d.%d\n", linked->major, linked->minor, linked->patch);

	printf("SDL Platform : %s\n", SDL_GetPlatform());
	printf("   SDL says cpu cache line size %ikb\n", SDL_GetCPUCacheLineSize());
	printf("   SDL says logical cpu count %i\n", SDL_GetCPUCount());
	printf("   SDL says system ram %iMB\n", SDL_GetSystemRAM());
	printf("   SDL says has 3D Now %s\n", SDL_Has3DNow() == SDL_TRUE ? "(yes)" : "(no)");
	printf("   SDL says has AltiVec %s\n", SDL_HasAltiVec() == SDL_TRUE ? "(yes)" : "(no)");
	printf("   SDL says has AVX %s\n", SDL_HasAVX() == SDL_TRUE ? "(yes)" : "(no)");
	printf("   SDL says has AVX2 %s\n", SDL_HasAVX2() == SDL_TRUE ? "(yes)" : "(no)");
	printf("   SDL says has MMX %s\n", SDL_HasMMX() == SDL_TRUE ? "(yes)" : "(no)");
	printf("   SDL says has RDTSC %s\n", SDL_HasRDTSC() == SDL_TRUE ? "(yes)" : "(no)");
	printf("   SDL says has SSE %s\n", SDL_HasSSE() == SDL_TRUE ? "(yes)" : "(no)");
	printf("   SDL says has SSE 2 %s\n", SDL_HasSSE2() == SDL_TRUE ? "(yes)" : "(no)");
	printf("   SDL says has SSE 3 %s\n", SDL_HasSSE3() == SDL_TRUE ? "(yes)" : "(no)");
	printf("   SDL says has SSE 4.1 %s\n", SDL_HasSSE41() == SDL_TRUE ? "(yes)" : "(no)");
	printf("   SDL says has SSE 4.2 %s\n", SDL_HasSSE42() == SDL_TRUE ? "(yes)" : "(no)");

	SDL_GetRendererInfo(g_renderer, &rendinfo);
	printf("Output is : %s\n", rendinfo.name);

	if((rendinfo.flags & SDL_RENDERER_SOFTWARE) == SDL_RENDERER_SOFTWARE)
	{
		printf("Render Output is Software Fallback\n");
	}
	if((rendinfo.flags & SDL_RENDERER_ACCELERATED) == SDL_RENDERER_ACCELERATED)
	{
		printf("Render Output is Accelerated\n");
	}
	if((rendinfo.flags & SDL_RENDERER_PRESENTVSYNC) == SDL_RENDERER_PRESENTVSYNC)
	{
		printf("Render Output is PresentVSYNC\n");
	}
	if((rendinfo.flags & SDL_RENDERER_SOFTWARE) == SDL_RENDERER_SOFTWARE)
	{
		printf("Render Output is Software Output\n");
	}
	if((rendinfo.flags & SDL_RENDERER_TARGETTEXTURE) == SDL_RENDERER_TARGETTEXTURE)
	{
		printf("Render Output is Target Texture\n");
	}

	video_name = SDL_GetVideoDriver(0);
	printf("Video Driver : %s\n", video_name);

	printf("\n");
}

static void do_version(void)
{
	printf("Version %s %s\n", VersionStringFull(), REL_STR);
}

static void do_banner(void)
{
	printf("Underdark - by BloodyCactus\n");
	do_version();
}

static void PreStartup(int argc, char **argv)
{
	int i;
#ifdef __linux__
	char hostname[128];
	uint16_t xadd = 0;
#endif
	for(i = 0; i < 16; i++)
	{
		font[i] = NULL;
	}

	memset(g_basicDict, 0x0, 256 * sizeof(uint32_t));

	__msg_buffer = malloc(MSG_BUFFER_LEN);
	assert(__msg_buffer != NULL);

	do_banner();
#ifdef __linux__
	gethostname(hostname, 128);
	for(i = 0, xadd = 5381; hostname[i] != 0; i++)
	{
		xadd = ((xadd << 3) + xadd) + hostname[i];
	}
	if(xadd == 0x7153) { workstationCactus = true; }
#endif

	for(i = 1; i < argc; i++)
	{
		///// cli args
	}

	TestConfigFile();
}

static void PostStartup(void)
{
	SDL_DisplayMode dm;
	uint32_t flags;

	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) < 0)
	{
		printf("Unable to init SDL: %s\n", SDL_GetError());
		exit(1);
	}
	atexit(SDL_Quit);

	TTF_Init();
	atexit(TTF_Quit);

	//IMG_Init(0);
	//atexit(IMG_Quit);

	if(SDL_GetDesktopDisplayMode(0, &dm) != 0)
	{
		printf("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
		exit(1);
	}

	printf("Desktop is %u x %u at %uhz\n", dm.w, dm.h, dm.refresh_rate);

	g_screen_width = dm.w - 32;
	//g_font_size = g_screen_width / 70;
	//g_font_size &= 0xFFE;
	//g_screen_width = g_font_size * 70;
	//g_screen_height = ((dm.h - (8*g_font_size)) / g_font_size) * g_font_size;

	if(dm.h > 768)
	{
		g_screen_height = dm.h - 128;
	}
	else
	{
		g_screen_height = dm.h - 64;
	}

	test_font_size();

	flags = 0;// SDL_WINDOW_FULLSCREEN_DESKTOP;

	//const char *title,int x, int y, int w,int h, Uint32 flags
	g_screen = SDL_CreateWindow(WindowTitle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, g_screen_width, g_screen_height, flags);
	if(g_screen == NULL)
	{
		printf("Unable to set %ix%i video: %s\n", g_screen_width, g_screen_height, SDL_GetError());
		exit(1);
	}
	else
	{
		g_renderer = SDL_CreateRenderer(g_screen, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
		if(g_renderer == NULL)
		{
			// XP fallback in virtualbox even tho it says it can do accelerated + targettexture it fails
			g_renderer = SDL_CreateRenderer(g_screen, -1, SDL_RENDERER_SOFTWARE);
		}

		// If we fail, return error.
		if(g_renderer == NULL)
		{
			printf("Unable to set g_renderer: %s\n", SDL_GetError());
			exit(1);
		}
		else
		{
			// hide mouse
			//SDL_ShowCursor(SDL_DISABLE);
			SDL_ShowCursor(SDL_DISABLE);
			SDL_SetRelativeMouseMode(SDL_TRUE);
		}
	}

	printf("Window Created %"PRIu32" x %"PRIu32"\n", g_screen_width, g_screen_height);
	SDL_SetWindowTitle(g_screen, WindowTitle); //Set the title caption

	dump_sdl_info();
	SDL_SetEventFilter(&eventFilter, NULL);

	/* seed state is not saved..... */
	reset_rnd_seed();
}


int main(int argc, char *argv[])
{
	uContext *ctx;

	PreStartup(argc, argv);

#if defined(_WIN64)
	/* Win (64-bit) */
#elif defined(_WIN32)
	/* Win (32-bit) */
#else
	if(getuid() == 0 || geteuid() == 0)
	{
		SDL_Quit();
		printf("Do not play this game as root!\n");
		exit(0);
	}
#endif

	PostStartup();
	InitScreen();
	load_tileset();
	CalcDict(font[0]);

	ctx = NewContext();
	SetStatusRow(ctx);

	if(ProcessKeybinds(ctx) == 0)
	{
		if(BuildItems(ctx) == 0)
		{
			if(BuildMonsters(ctx) == 0)
			{
				if(IsFirstTime() == true)
				{
					DisplayFixedFile("welcome.txt", "New Player Info");
					RollPlayer(ctx, true);
				}
				else
				{
					RollPlayer(ctx, false);
				}

				SetStatusRow(ctx);
				main_screen(ctx);
			}
		}
	}

	SDL_ShowCursor(SDL_ENABLE);

	unload_tileset();
	ShutdownScreen();

	if(g_renderer != NULL)
	{
		SDL_DestroyRenderer(g_renderer);
	}

	if(g_screen != NULL)
	{
		SDL_DestroyWindow(g_screen);
	}

	ReleaseContext(ctx);
	free(__msg_buffer);

	// trig atexit's
	exit(0);
	//return 0;
}

