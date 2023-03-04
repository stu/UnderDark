#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

int main(int argc, char *argv[])
{
	FILE *fp;

	char l1[64];
	char l2[64];
	char l3[64];

	unsigned short major = 0;
	unsigned short minor = 0;
	unsigned short build = 0;

	fp = fopen("version","rt+");
	if(fp == NULL)
	{
		fp = fopen("VERSION", "rt+");
	}

	if(fp == NULL)
	{
		sprintf(l1, "0\n");
		sprintf(l2, "0\n");
		sprintf(l3, "0\n");
	}
	else
	{
		fseek(fp, 0x0L, SEEK_SET);

		fgets(l1, 64, fp);
		fgets(l2, 64, fp);
		fgets(l3, 64, fp);
		fclose(fp);

		remove("VERSION");
		remove("version");
	}


	major = atoi(l1);
	minor = atoi(l2);
	build = atoi(l3);

	if(argc > 1 && strcmp(argv[1], "--major") == 0)
	{
		major += 1;
		minor = 0;
		build = 0;
	}
	else if(argc > 1 && strcmp(argv[1], "--minor") == 0)
	{
		minor += 1;
		build = 0;
	}
	else
	{
		build += 1;
		if(build > 9999)
		{
			minor += 1;
			build = 0;
			if(minor > 255)
			{
				minor = 0;
				major += 1;
			}
		}
	}

	fp = fopen("version", "wt");
	fprintf(fp, "%i\n%i\n%i\n", major, minor, build);
	fclose(fp);

	remove("VERSION.C");
	remove("version.c");
	fp = fopen("version.c", "wt");

	fprintf(fp, "#include <stdint.h>\n");
	fprintf(fp, "\n");
	fprintf(fp, "#define VER_MAJ\t\t%i\n", major);
	fprintf(fp, "#define VER_MIN\t\t%i\n", minor);
	fprintf(fp, "#define VER_BUILD\t%i\n", build);
	fprintf(fp, "\n");

	fprintf(fp, "uint16_t VersionMajor(void)\n{\n\treturn VER_MAJ;\n}\n");
	fprintf(fp, "uint16_t VersionMinor(void)\n{\n\treturn VER_MIN;\n}\n");
	fprintf(fp, "uint16_t VersionBuild(void)\n{\n\treturn VER_BUILD;\n}\n");
	fprintf(fp, "char* VersionString(void)\n{\n\treturn \"v%i.%02i/%04i\";\n}\n", major, minor, build);
	fprintf(fp, "char* VersionStringFull(void)\n{\n\treturn \"v%i.%02i/%04i; \" __DATE__ \", \" __TIME__ \"\";\n}\n", major, minor, build);
	fprintf(fp, "const char* VersionTime(void)\n{\n\treturn __TIME__;\n}\n");
	fprintf(fp, "const char* VersionDate(void)\n{\n\treturn __DATE__;\n}\n");

	fprintf(fp, "\n");

	remove("VERSION.H");
	remove("version.h");

	fp = fopen("version.h", "wt");
	fprintf(fp, "#ifndef VERSION_H\n");
	fprintf(fp, "#define VERSION_H\n");
	fprintf(fp, "#ifdef __cplusplus\n");
	fprintf(fp, "extern \"C\"{\n");
	fprintf(fp, "#endif\n");
	fprintf(fp, "\n");
	fprintf(fp, "extern uint16_t VersionMajor(void);\n");
	fprintf(fp, "extern uint16_t VersionMinor(void);\n");
	fprintf(fp, "extern uint16_t VersionBuild(void);\n");
	fprintf(fp, "extern char* VersionString(void);\n");
	fprintf(fp, "extern char* VersionStringFull(void);\n");
	fprintf(fp, "extern const char* VersionTime(void);\n");
	fprintf(fp, "extern const char* VersionDate(void);\n");
	fprintf(fp, "\n");
	fprintf(fp, "#ifdef __cplusplus\n");
	fprintf(fp, "};\n");
	fprintf(fp, "#endif\n");
	fprintf(fp, "#endif        //  #ifndef VERSION_H\n");
	fprintf(fp, "\n");

	fclose(fp);

	return 0;
}
