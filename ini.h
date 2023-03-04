#ifndef _INIFILE
#define _INIFILE

#ifdef __cplusplus
extern "C"{
#endif

#pragma pack(push, 1)

typedef struct INIFILE
{
	DList *lstGroups;
} INIFILE;
#pragma pack(pop)


struct udtINIItem
{
	DList *lstComments;
	char *name;
	char *value;
};

struct udtINIGroup
{
	DList *lstComments;
	char *name;
	DList *lstItems;
};

extern INIFILE *INI_load(char *fname);
extern INIFILE *INI_loadbuff(char *buff, uint32_t len);
extern INIFILE *INI_load_filepointer(FILE *fp);

extern void INI_unload(INIFILE *f);
extern char* INI_get(INIFILE *ini, char *group, char *item);

extern INIFILE* INI_EmptyINF(void);
extern uint16_t INI_save(char *fname, INIFILE *f);
extern void INI_UpdateItem(INIFILE *f, char *group, char *key, char *val);

extern uint32_t INI_get_unumber(INIFILE *ini, char *group, char *item, uint32_t minimum, uint32_t maximum, uint32_t def);
extern int32_t INI_get_number(INIFILE *ini, char *group, char *item, int32_t minimum, int32_t maximum, int32_t def);

extern bool INI_get_bool(INIFILE *ini, char *group, char *item, bool def);
extern char* INI_get_string(INIFILE *ini, char *group, char *item, char *def);

extern char* INI_getcopy(INIFILE *ini, char *group, char *item);

extern uint16_t INI_section_item_count(INIFILE *ini, char *group);

#ifdef __cplusplus
};
#endif
#endif
