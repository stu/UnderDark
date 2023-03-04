#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <sys/types.h>
#include <assert.h>

#include "dlist.h"
#include "ini.h"

#ifdef __linux__
#define stricmp strcasecmp
#endif

#ifdef PATH_MAX
#define MAX_EXECNAME_SIZE PATH_MAX+2
#else
#define MAX_EXECNAME_SIZE  1024
#endif

#define MAX_INILINE_SIZE 4096

static void FreeINIGroup(void *data);
static void INI_ParseFile(FILE *fp, INIFILE *f);
static void INI_ParseBuff(const uint8_t *buff, uint32_t len, INIFILE *f);
static void FreeINIItem(void *data);

INIFILE *INI_EmptyINF(void)
{
	INIFILE *f;

	f = calloc(1, sizeof(INIFILE));
	assert(f != NULL);
	f->lstGroups = NewDList(FreeINIGroup);

	return f;
}

static void SaveComments(DList *lst, FILE *fp)
{
	DLElement *e;

	if(lst == NULL)
	{
		return;
	}

	e = dlist_head(lst);
	while(e != NULL)
	{
		fprintf(fp, "%s\n", (char *)dlist_data(e));
		e = dlist_next(e);
	}
}

uint16_t INI_save(char *fname, INIFILE *f)
{
	FILE *fp;
	uint16_t errl = 1;

	DLElement *egroup;
	DLElement *eitem;

	struct udtINIGroup *g;
	struct udtINIItem *i;

	remove(fname);

	if((fp = fopen(fname, "wt")) != NULL)
	{
		egroup = dlist_head(f->lstGroups);

		while(egroup != NULL)
		{
			g = dlist_data(egroup);

			if(g != NULL)
			{
				SaveComments(g->lstComments, fp);
			}

			fprintf(fp, "%s\n", g->name);

			eitem = dlist_head(g->lstItems);

			while(eitem != NULL)
			{
				i = dlist_data(eitem);
				SaveComments(i->lstComments, fp);
				fprintf(fp, "%s = %s\n", i->name, i->value);

				eitem = dlist_next(eitem);
			}

			fprintf(fp, "\n");

			egroup = dlist_next(egroup);
		}

		fclose(fp);
		errl = 0;
	}

	return errl;
}

void INI_UpdateItem(INIFILE *f, char *group, char *key, char *val)
{
	DLElement *egroup;
	struct udtINIGroup *g;
	struct udtINIItem *i;
	char *gname = NULL;

	assert(f != NULL);

	assert(group != NULL);
	assert(key != NULL);

	gname = calloc(1, 4 + strlen(group));
	assert(gname != NULL);

	strcpy(gname, "[");
	strcat(gname, group);
	strcat(gname, "]");

	assert(f->lstGroups != NULL);
	egroup = dlist_head(f->lstGroups);

	while(egroup != NULL)
	{
		g = dlist_data(egroup);
		assert(g != NULL);
		assert(g->name != NULL);

		if(stricmp(gname, g->name) == 0)
		{
			DLElement *eitem;

			eitem = dlist_head(g->lstItems);
			while(eitem != NULL)
			{
				i = dlist_data(eitem);

				if(stricmp(key, i->name) == 0)
				{
					// release old value and update with new
					assert(i->value != NULL);
					free(i->value);

					if(val == NULL)
					{
						i->value = strdup("");
					}
					else
					{
						i->value = strdup(val);
					}

					free(gname);
					return;
				}

				eitem = dlist_next(eitem);
			}

			// does not exist, so add new item to existing group
			i = calloc(1, sizeof(struct udtINIItem));
			assert(i != NULL);

			i->name = strdup(key);
			if(val == NULL)
			{
				i->value = strdup("");
			}
			else
			{
				i->value = strdup(val);
			}
			dlist_ins(g->lstItems, i);

			free(gname);
			return;
		}

		egroup = dlist_next(egroup);
	}

	// does not exist in any groups...
	g = calloc(1, sizeof(struct udtINIGroup));
	assert(g != NULL);
	g->name = strdup(gname);
	g->lstItems = NewDList(FreeINIItem);
	assert(g->lstItems != NULL);
	dlist_ins(f->lstGroups, g);

	i = calloc(1, sizeof(struct udtINIItem));
	assert(i != NULL);
	i->name = strdup(key);
	if(val == NULL)
	{
		i->value = strdup("");
	}
	else
	{
		i->value = strdup(val);
	}
	dlist_ins(g->lstItems, i);
	free(gname);
}

static void FreeComment(void *s)
{
	if(s == NULL)
		return;

	free(s);
}

static void FreeINIItem(void *data)
{
	struct udtINIItem *x = data;

	if(x == NULL)
	{
		return;
	}

	FreeDList(x->lstComments);

	if(x->name != NULL)
	{
		free(x->name);
	}

	if(x->value != NULL)
	{
		free(x->value);
	}

	memset(x, 0x0, sizeof(struct udtINIItem));
	free(x);
}

static void FreeINIGroup(void *data)
{
	struct udtINIGroup *g = data;

	if(g == NULL)
	{
		return;
	}

	FreeDList(g->lstComments);
	FreeDList(g->lstItems);

	if(g->name != NULL)
	{
		free(g->name);
	}

	memset(g, 0x0, sizeof(struct udtINIGroup));
	free(g);
}

void INI_unload(INIFILE *f)
{
	if(f == NULL)
	{
		return;
	}

	FreeDList(f->lstGroups);
	memset(f, 0x0, sizeof(INIFILE));
	free(f);
}


INIFILE *INI_loadbuff(char *buff, uint32_t len)
{
	INIFILE *f;

	f = calloc(1, sizeof(INIFILE));
	assert(f != NULL);

	f->lstGroups = NewDList(FreeINIGroup);

	INI_ParseBuff((uint8_t *)buff, len, f);

	return f;
}


INIFILE *INI_load_filepointer(FILE *fp)
{
	INIFILE *f;
	off_t offs;

	offs = ftell(fp);

	f = calloc(1, sizeof(INIFILE));
	assert(f != NULL);
	f->lstGroups = NewDList(FreeINIGroup);
	INI_ParseFile(fp, f);

	fseek(fp, offs, SEEK_SET);

	return f;
}

INIFILE *INI_load(char *fname)
{
	INIFILE *f;
	FILE *fp;

	if(fname == NULL)
	{
		return NULL;
	}

	fp = fopen(fname, "rb");
	if(fp == NULL)
	{
		return NULL;
	}

	f = INI_load_filepointer(fp);

	fclose(fp);

	return f;
}

static char *QuotedName(char *q, uint16_t end)
{
	uint16_t quote = 0;
	uint16_t quote1 = 0;

	while(*q != 0)
	{
		if(*q == end)
		{
			return q;
		}

		switch(*q)
		{
			case '#':
				if(quote == 0 && quote1 == 0)
				{
					*q = 0;
					return q;
				}
				else
				{
					q++;
				}
				break;

			case '\"':
				quote = !quote;
				q++;
				break;

			case '\'':
				quote1 = !quote1;
				q++;
				break;

			case '\\':
				q += 2;
				break;

			default:
				q += 1;
				break;
		}
	}

	return q;
}


static void INI_ParseBuff(const uint8_t *buff, uint32_t len, INIFILE *f)
{
	char *line;
	char *z, *x;

	char *sym;
	char *val;

	uint32_t read_idx;
	uint16_t idx;

	struct udtINIGroup *current_group;

	DList *lstComments;

	line = malloc(MAX_INILINE_SIZE + 16);
	assert(line != NULL);
	sym = NULL;
	val = NULL;

	current_group = NULL;

	read_idx = 0;

	lstComments = NULL;

	if(buff != NULL)
	{
		do
		{
			idx = 0;
			line[idx] = 0;

			while(idx < MAX_INILINE_SIZE && read_idx < len && !(buff[read_idx] == 0x0D || buff[read_idx] == 0x0A))
			{
				line[idx++] = buff[read_idx++];
				line[idx] = 0;
			}

			if(idx < MAX_INILINE_SIZE && read_idx < len && buff[0 + read_idx] == 0x0D)
			{
				line[idx++] = buff[read_idx++];
			}

			if(idx < MAX_INILINE_SIZE && read_idx < len && buff[0 + read_idx] == 0x0A)
			{
				line[idx++] = buff[read_idx++];
			}

			if(read_idx >= len)
			{
				read_idx = len;
			}

			if(*line != 0x0)
			{
				x = strchr(line, 0x0A);        // strip cr/lf
				if(x != NULL)
				{
					*x = 0x0;
				}

				x = strchr(line, 0x0D);    // included just in case ^_^
				if(x != NULL)
				{
					*x = 0x0;
				}

				if(*line != 0)
				{
					z = line;

					// skip over whitespace.
					while(*z != 0 && isspace(*z) != 0)
					{
						z++;
					}

					if(*z == 0)
					{
						break;
					}

					if(*z == '#')
					{
						if(lstComments == NULL)
						{
							lstComments = NewDList(FreeComment);
						}
						dlist_ins(lstComments, strdup(z));
					}

					sym = z;
					val = QuotedName(sym, '=');
					if(*val == '=')
					{
						*val = 0;
						val++;
					}
					QuotedName(val, 0);

					z = val;
					while(z != NULL && *z != 0 && isspace(*z) != 0)
					{
						z++;
					}
					val = z;

					if(*sym != 0)
					{
						z = strchr(sym, 0x0);
						z--;

						while(z != sym && isspace(*z) != 0)
						{
							*z = 0;
							z--;
						}

						if(*val != 0)
						{
							z = strchr(val, 0x0);
							z--;

							while(z != val && isspace(*z) != 0)
							{
								*z = 0;
								z--;
							}
						}
					}

					if(*sym == '[' && sym[strlen(sym) - 1] == ']')
					{
						current_group = calloc(1, sizeof(struct udtINIGroup));
						assert(current_group != NULL);
						current_group->name = strdup(sym);
						current_group->lstComments = lstComments;
						current_group->lstItems = NewDList(FreeINIItem);
						dlist_ins(f->lstGroups, current_group);

						lstComments = NULL;
					}
					else
					{
						/* skip items outside a group */
						if(current_group != NULL && strlen(sym) > 0)
						{
							struct udtINIItem *i;

							i = calloc(1, sizeof(struct udtINIItem));
							assert(i != NULL);
							i->name = strdup(sym);
							i->value = strdup(val);
							i->lstComments = lstComments;
							lstComments = NULL;
							dlist_ins(current_group->lstItems, i);
						}
					}
				}
			}
		} while(read_idx < len);
	}

	if(lstComments != NULL)
	{
		FreeDList(lstComments);
	}

	free(line);
}

static void INI_ParseFile(FILE *fp, INIFILE *f)
{
	uint8_t *buff;
	size_t blen;
	size_t olen;

	olen = ftell(fp);
	fseek(fp, 0x0L, SEEK_END);
	blen = ftell(fp);
	fseek(fp, olen, SEEK_SET);

	buff = calloc(1, blen + 16);
	assert(buff != NULL);
	if(buff != NULL)
	{
		size_t qlen;

		qlen = fread(buff, 1, blen, fp);

		if(qlen != blen)
		{
			// rb vs rt! might not match
		}

		INI_ParseBuff(buff, blen, f);
		free(buff);
	}
}

char *INI_get(INIFILE *ini, char *group, char *item)
{
	DLElement *egroup;
	struct udtINIGroup *g;
	char *gname;

	gname = calloc(1, 4 + strlen(group));
	assert(gname != NULL);
	sprintf(gname, "[%s]", group);

	egroup = dlist_head(ini->lstGroups);

	while(egroup != NULL)
	{
		g = dlist_data(egroup);
		egroup = dlist_next(egroup);

		if(stricmp(gname, g->name) == 0)
		{
			DLElement *eitem;
			struct udtINIItem *i;

			eitem = dlist_head(g->lstItems);
			while(eitem != NULL)
			{
				i = dlist_data(eitem);
				eitem = dlist_next(eitem);

				if(stricmp(item, i->name) == 0)
				{
					free(gname);
					if(i->value[0] == 0)
					{
						return NULL;
					}
					else
					{
						return i->value;
					}
				}
			}

			/* no matching item in group */
			free(gname);
			return NULL;
		}
	}

	free(gname);
	return NULL;
}

uint32_t INI_get_unumber(INIFILE *ini, char *group, char *item, uint32_t minimum, uint32_t maximum, uint32_t def)
{
	uint32_t rc;
	char *p;

	rc = def;
	if(ini != NULL)
	{
		p = INI_get(ini, group, item);
		if(p != NULL)
		{
			if(p[0] == '0' && p[1] == 'x')
			{
				rc = strtoul(p + 2, NULL, 16);
			}
			else
			{
				rc = strtoul(p, NULL, 10);
			}

			if(rc < minimum)
			{
				rc = minimum;
			}
			if(rc > maximum)
			{
				rc = maximum;
			}
		}
	}

	return rc;
}

int32_t INI_get_number(INIFILE *ini, char *group, char *item, int32_t minimum, int32_t maximum, int32_t def)
{
	int32_t rc;
	char *p;

	rc = def;
	if(ini != NULL)
	{
		p = INI_get(ini, group, item);
		if(p != NULL)
		{
			if(p[0] == '0' && p[1] == 'x')
			{
				rc = strtoul(p + 2, NULL, 16);
			}
			else
			{
				rc = strtoul(p, NULL, 10);
			}

			if(rc < minimum)
			{
				rc = minimum;
			}
			if(rc > maximum)
			{
				rc = maximum;
			}
		}
	}

	return rc;
}

bool INI_get_bool(INIFILE *ini, char *group, char *item, bool def)
{
	bool rc = def;
	char *p;

	if(ini != NULL)
	{
		p = INI_get(ini, group, item);
		if(p != NULL)
		{
			rc = false;
			if(stricmp(p, "true") == 0 || stricmp(p, "yes") == 0)
			{
				rc = true;
			}
		}
	}

	return rc;
}

char *INI_get_string(INIFILE *ini, char *group, char *item, char *def)
{
	char *rc;
	char *p;

	if(ini != NULL)
	{
		p = INI_get(ini, group, item);
		if(p != NULL)
		{
			rc = strdup(p);
		}
		else
		{
			if(def == NULL)
			{
				rc = NULL;
			}
			else
			{
				rc = strdup(def);
			}
		}
	}
	else
	{
		if(def == NULL)
		{
			rc = NULL;
		}
		else
		{
			rc = strdup(def);
		}
	}

	return rc;
}

char *INI_getcopy(INIFILE *ini, char *group, char *item)
{
	char *q = INI_get(ini, group, item);
	char *z;

	if(q == NULL)
	{
		return NULL;
	}

	z = calloc(1, strlen(q) + 4);
	assert(z != NULL);
	memmove(z, q, strlen(q) + 1);

	return z;
}

uint16_t INI_section_item_count(INIFILE *ini, char *group)
{
	DLElement *egroup;
	struct udtINIGroup *g;
	char *gname;

	gname = calloc(1, 4 + strlen(group));
	assert(gname != NULL);
	sprintf(gname, "[%s]", group);

	egroup = dlist_head(ini->lstGroups);

	while(egroup != NULL)
	{
		g = dlist_data(egroup);
		egroup = dlist_next(egroup);

		if(stricmp(gname, g->name) == 0)
		{
			free(gname);
			if(dlist_size(g->lstItems) > 0xF000)
			{
				return UINT16_MAX;
			}
			else
			{
				return dlist_size(g->lstItems);
			}
		}
	}

	free(gname);
	return 0;
}
