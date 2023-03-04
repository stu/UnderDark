// shitty gcc vomit
#ifdef __llvm__
#else
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <inttypes.h>

#include <assert.h>

#ifndef MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#endif

#include "version.h"

#include "dlist.h"
#include "ini.h"
#include "time.h"
#include "strings.h"

#include "structs.h"

#include "screen.h"
#include "play.h"
#include "message.h"

#include "item.h"
#include "monster.h"

#include "rnd.h"
#include "name.h"
#include "keys.h"
#include "keycodes.h"
#include "main.h"
#include "scores.h"
#include "news.h"
#include "dgen.h"
#include "death.h"
#include "inv.h"
#include "levup.h"
#include "save.h"
#include "save_ext.h"
#include "load.h"

#ifdef NDEBUG
#define REL_STR "(RELEASE)"
#else
#define REL_STR "(DEBUG)"
#endif

#ifdef __linux__
#define stricmp strcasecmp
#endif

// in hardware driver
extern uint16_t GetTickCount(void);
extern void wait_ticks(uint16_t ticks);

