#include "headers.h"
#include <i86.h>

int16_t screen_mode;

#ifdef __386__
volatile uint16_t *_pit_tick = (uint16_t*)0x46C;
#else
volatile uint16_t far *_pit_tick = MK_FP(0x40,0x6C);
#endif

char* GetDataFile(char *fn)
{
	return strdup(fn);
}

char* GetConfigurationFile(char *fn)
{
	return strdup(fn);
}

void wait_ticks(uint16_t ticks)
{
	uint16_t start = GetTickCount();

	// base clock is 18 ticks a second?
	while(GetTickCount() - start <= ticks)
		;

}

uint16_t GetTickCount(void)
{
	return *_pit_tick;
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


static void do_help(void)
{
	printf("\n");
	printf("-?      This help\n");
	printf("-v      Display Version\n");
	printf("-80x25  Start in 80x25 mode\n");
	printf("-80x28  Start in 80x28 mode\n");
	printf("-80x43  Start in 80x43 mode\n");
	printf("-80x50  Start in 80x50 mode\n");

}

static bool parse_args(int argc, char *argv[])
{
	bool rc = true;
	uint16_t i;

	for(i = 1; i < argc && rc == true; i++)
	{
		if(stricmp(argv[i], "-?") == 0)
		{
			do_help();
			rc = false;
		}
		else if(stricmp(argv[i], "-v") == 0)
		{
			// already printed
			rc = false;
		}
		else if(stricmp(argv[i], "-80x25") == 0)
		{
			screen_mode = SCREEN_MODE_80X25;
		}
		else if(stricmp(argv[i], "-80x28") == 0)
		{
			screen_mode = SCREEN_MODE_80X28;
		}
		else if(stricmp(argv[i], "-80x43") == 0)
		{
			screen_mode = SCREEN_MODE_80X43;
		}
		else if(stricmp(argv[i], "-80x50") == 0)
		{
			screen_mode = SCREEN_MODE_80X50;
		}
		else if(stricmp(argv[i], "--dev") == 0)
		{
			workstationCactus = true;
		}
		else
		{
			rc = false;
		}
	}

	return rc;
}

uint16_t ConfigGetScreenMode(void)
{
	INIFILE *ini;
	char *p;
	int16_t mode = SCREEN_MODE_80X25;
	char *cfgFile = GetConfigurationFile(CONFIG_FILE);
	assert(cfgFile != NULL);

	if(screen_mode != -1)
	{
		return screen_mode;
	}

	ini = INI_load(cfgFile);
	free(cfgFile);
	if(ini != NULL)
	{
		p = INI_get(ini, "underdark", "mode");
		if(p != NULL && stricmp(p, "80x25") == 0)
		{
			mode = SCREEN_MODE_80X25;
		}
		else if(p != NULL && stricmp(p, "80x28") == 0)
		{
			mode = SCREEN_MODE_80X28;
		}
		else if(p != NULL && stricmp(p, "80x43") == 0)
		{
			mode = SCREEN_MODE_80X43;
		}
		else if(p != NULL && stricmp(p, "80x50") == 0)
		{
			mode = SCREEN_MODE_80X50;
		}

		INI_unload(ini);
	}

	return mode;
}


int main(int argc, char *argv[])
{
	uContext *ctx;

	__msg_buffer = malloc(MSG_BUFFER_LEN);
	assert(__msg_buffer != NULL);
	do_banner();

	TestConfigFile();

	screen_mode = -1;
	screen_set_startmode(SCREEN_MODE_80X25);
	if(parse_args(argc, argv) == false)
	{
		return 1;
	}
	reset_rnd_seed();

	InitScreen();
	hide_cursor();
	ctx = NewContext();
	SetStatusRow(ctx);
	if(ctx->map != NULL)
	{
		if(ProcessKeybinds(ctx) == 0)
		{
			if(BuildItems(ctx) == 0)
			{
				if(BuildMonsters(ctx) == 0)
				{
					ScreenChangeMode(ConfigGetScreenMode());
					hide_cursor();

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

					ScreenChangeMode(SCREEN_MODE_80X25);
					ShutdownScreen();
					do_banner();
				}
			}
		}
	}
	else
	{
		printf("Not enough memory\n");
	}

	ReleaseContext(ctx);

	free(__msg_buffer);
	return 0;
}
