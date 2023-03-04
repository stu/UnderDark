#include "headers.h"

static void FreeScore(void *x)
{
	uScore *s = (uScore *)x;

	if(s == NULL)
	{
		return;
	}

	memset(s, 0x0, sizeof(uScore));
	free(s);
}

static INIFILE *GetScoresINI(void)
{
	INIFILE *ini;
	char *cfgFile;

	cfgFile = GetConfigurationFile("scores.ini");
	ini = INI_load(cfgFile);
	free(cfgFile);

	if(ini == NULL)
	{
		char *modified;

		ini = INI_EmptyINF();
		assert(ini != NULL);
		INI_UpdateItem(ini, "scores", "count", "1");

		modified = (char*)get_current_datetime();
		INI_UpdateItem(ini, "scores", "modified", modified);
		free(modified);

		save_str(ini, "score.1", "name", (uint8_t*)"BloodyCactus");
		save_str(ini, "score.1", "message", (uint8_t*)"Died before starting!");
		save_str(ini, "score.1", "date", (uint8_t*)"202212011212");
		save_str(ini, "score.1", "state", (uint8_t*)"start");
		save_str(ini, "score.1", "build", (uint8_t*)"0.2.0/0000");
		save_u32(ini, "score.1", "score", 10);
		save_i16(ini, "score.1", "level", 0);
		save_u32(ini, "score.1", "score", 10);
	}

	return ini;
}

static void NumberScores(DList *lst)
{
	DLElement *e;
	uScore *s;
	uint16_t i;

	assert(lst != NULL);

	i = 1;
	e = dlist_head(lst);
	while(e != NULL)
	{
		s = dlist_data(e);
		e = dlist_next(e);

		assert(s != NULL);

		s->id = i;
		i += 1;
	}
}

static void SortInsertScore(DList *lst, uScore *s)
{
	DLElement *e;
	uScore *score;

	assert(lst != NULL);
	assert(s != NULL);

	e = dlist_head(lst);
	while(e != NULL)
	{
		score = dlist_data(e);
		assert(score != NULL);

		if((s->score > score->score) || (s->score == score->score && strcmp((char*)s->name, (char*)score->name) < 0))
		{
			dlist_ins_prev(lst, e, s);
			NumberScores(lst);
			return;
		}

		e = dlist_next(e);
	}

	dlist_ins(lst, s);
	NumberScores(lst);
}

static uScore *CreateEmptyScore(void)
{
	uScore *s = (uScore *)calloc(1, sizeof(uScore));
	assert(s != NULL);

	strcpy((char*)s->name, "Anonymous");
	s->score = 0;
	s->level = 1;
	strcpy((char*)s->message, "Something mysterious happenend");
	strcpy((char*)s->date, "198601010800");
	strcpy((char*)s->location, "Nowhere");

	return s;
}


static void AddDefaultScore(uScore *s)
{
	strncpy((char*)s->name, "BloodyCactus", MAX_NAME_LEN);
	s->score = 10;
	strncpy((char*)s->message, "Died before starting!", MAX_DEATH_MESSAGE_LEN);
	strncpy((char*)s->date, "202212011212", 32);
}


static DList *LoadScores(void)
{
	INIFILE *ini;
	DList *lst;

	bool rc;
	char *p;
	char group[32];
	uint16_t mcount;
	uint16_t i;
	char lbuff[512];
	uScore *score;

	lst = NewDList(FreeScore);
	assert(lst != NULL);

	ini = GetScoresINI();
	assert(ini != NULL);

	mcount = get_u16(ini, "scores", "count", &rc);
	if(mcount > 25) mcount = 25;

	for(i = 0; i < mcount; i++)
	{
		sprintf(group, "score.%u", 1 + i);
		score = CreateEmptyScore();
		assert(score != NULL);

		p = INI_get(ini, group, "name");
		if(p != NULL)
		{
			decode64(p, (uint8_t *)lbuff, 512);
			snprintf((char*)score->name, MAX_NAME_LEN, "%s", lbuff);
		}

		score->score = get_u32(ini, group, "score", &rc);

		p = INI_get(ini, group, "message");
		if(p == NULL)
		{
			snprintf((char*)score->message, MAX_DEATH_MESSAGE_LEN, "%s", "Unknown Causes");
		}
		else
		{
			decode64(p, (uint8_t*)lbuff, 512);
			snprintf((char*)score->message, MAX_DEATH_MESSAGE_LEN, "%s", lbuff);
		}

		p = INI_get(ini, group, "date");
		if(p == NULL)
		{
			snprintf((char*)score->date, 32, "%s", "198601010800");
		}
		else
		{
			decode64(p, (uint8_t*)lbuff, 512);
			snprintf((char*)score->date, 32, "%s", lbuff);
		}

		p = INI_get(ini, group, "location");
		if(p != NULL)
		{
			decode64(p, (uint8_t*)lbuff, 512);
			snprintf((char*)score->location, MAX_LOCATION_LEN, "%s", lbuff);
		}

		score->level = get_i16(ini, group, "level", &rc);

		p = INI_get(ini, group, "build");
		if(p != NULL)
		{
			decode64(p, (uint8_t*)lbuff, 512);
			snprintf((char*)score->build, MAX_BUILD_LEN, "%s", lbuff);
		}

		SortInsertScore(lst, score);
	}

	INI_unload(ini);

	if(dlist_size(lst) == 0)
	{
		score = CreateEmptyScore();
		AddDefaultScore(score);
		dlist_ins(lst, score);
	}

	return lst;
}

void PrintScores(DList *lst, int16_t from, int16_t me)
{
	DLElement *e;
	uScore *s;

	int16_t i;
	int16_t j;
	char *dte;

	uint8_t clr_l, clr_h;

	assert(lst != NULL);


	if(from >= dlist_size(lst) || from < 0)
	{
		from = 0;
	}

	i = 1;
	e = dlist_head(lst);
	while(from > 0 && e != NULL)
	{
		e = dlist_next(e);
		from -= 1;
		i += 1;
	}

	toggle_swap_draw_to_backbuffer();
	screen_clear(0x7);
	screen_line_clear(0, 0x70);
	msg(0, 0, 0x70, "Underdark - List of Fallen Heroes");
	msg(1, 0, 0xF, "Rank  Score  Person");

	for(j = 0; j < 10 && e != NULL; j++)
	{
		s = dlist_data(e);
		assert(s != NULL);
		e = dlist_next(e);

		clr_l = 7;
		clr_h = 0xF;
		if(s->id == me)
		{
			clr_l = HL_CLR_LO;
			clr_h = HL_CLR_HI;
		}

		dte = (char*)datetime_tostring((char*)s->date);

		msg_hl(2 + (j * 2), 1, clr_l, clr_h, "`%2u`) %6"PRIu32", The story of `%s` ended on %s", i, s->score, s->name, dte);
		msg_hl(2 + (j * 2) + 1, 13, clr_l, clr_h, "%s on level %"PRIu16"", s->message, s->level);

		free(dte);

		i += 1;
	}

	msg(screen_get_height() - 1, (screen_get_width() - 12) / 2, 7, "Press any key");

	toggle_swap_draw_to_backbuffer();
	restore_from_backbuffer();

	get_key();
}

void DisplayScores(void)
{
	DList *lst;

	lst = LoadScores();

	PrintScores(lst, 0, 9999);
	FreeDList(lst);
}

void CreateScore(uContext *ctx, char *msg)
{
	uScore *s;
	char group[32];
	DList *lst;
	int16_t q;
	char *dte;
	INIFILE *ini;
	DLElement *e;
	char *cfgFile;

	lst = LoadScores();
	assert(lst != NULL);

	s = CreateEmptyScore();
	assert(s != NULL);
	dte = (char*)get_current_datetime();
	s->level = ctx->locn.level;
	s->score = ctx->player->score;
	strncpy((char*)s->build, VersionString(), MAX_BUILD_LEN);
	strncpy((char*)s->name, (char*)ctx->player->name, MAX_NAME_LEN);
	strncpy((char*)s->message, msg, MAX_DEATH_MESSAGE_LEN);
	strcpy((char*)s->date, dte);
	free(dte);

	SortInsertScore(lst, s);
	assert(dlist_size(lst) >= 1);

	assert(s != NULL);
	q = s->id;
	if(q < 5 || dlist_size(lst) < 10)
	{
		q = 0;
	}
	else if(q > dlist_size(lst) - 10)
	{
		q = dlist_size(lst) - 10;
	}
	else
	{
		q = s->id - 5;
	}

	PrintScores(lst, q, s->id);
	assert(lst != NULL);

	cfgFile = GetConfigurationFile("scores.ini");
	remove(cfgFile);

	ini = INI_EmptyINF();
	dte = (char*)get_current_datetime();
	save_str(ini, "scores", "modified",(uint8_t*)dte);
	free(dte);

	save_u16(ini, "scores", "count", 0);

	e = dlist_head(lst);
	for(q = 0; q < 25 && e != NULL; q++)
	{
		s = dlist_data(e);
		e = dlist_next(e);

		assert(s != NULL);

		sprintf(group, "score.%u", 1 + q);
		save_str(ini, group, "name", s->name);
		save_u32(ini, group, "score", s->score);
		save_str(ini, group, "message", s->message);
		save_str(ini, group, "date", s->date);
		save_str(ini, group, "location", s->location);
		save_i16(ini, group, "level", s->level);
		save_str(ini, group, "build", s->build);
	}

	save_u16(ini, "scores", "count", q);


	INI_save(cfgFile, ini);
	INI_unload(ini);
	free(cfgFile);

	FreeDList(lst);
}

