
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	unsigned long fsize;
	int lcount;
	unsigned char c;
	
	FILE *fp;
	if(argc<3)
	{
		printf("bin2c inputfile recordname\n");
		exit(0);
	}
	
	
	fp = fopen(argv[1], "rb");
	if(fp!=NULL)
	{
		fseek(fp, 0x0L, SEEK_END);
		fsize = ftell(fp);
		fseek(fp, 0x0L, SEEK_SET);
		
		lcount = 0;
		
		printf("#define %s_SIZE\t\t%li\n", argv[2], fsize);
		printf("unsigned char %s[]=\n{\n\t", argv[2]);
		
		while(ftell(fp) < fsize)
		{
			c = fgetc(fp);
			
			if(lcount > 0)
				printf(", ");
			
			printf("0x%02X", c);
			
			lcount++;
			if(lcount == 10)
			{
				lcount = 0;
				printf(", \n\t");
			}
		}
		fclose(fp);
		printf("\n};\n");
	}
	else
	{
		fprintf(stderr, "Could not open file\n");
	}
	
	return 0;
}

