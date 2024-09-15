#ifndef BMP_H
#define BMP_H

typedef struct
{
	int filesize;
	int reserved;
	int dataoffset;
} bmp_header;

typedef struct
{
	int headersize;
	int width;
	int height;
	short numplanes;
	short bpp;
	int compression;
	int datasize;
	int widthppm;
	int heightppm;
	int numcolors;
	int impcolors;
} bmp_info;

typedef struct
{
	int width;
	int height;
	int depth;
	int bpp;
	char format;
} bm3_info;


char *loadbmp(const char *filename, bmp_header *head, bmp_info *info);
int savebmp(const char *filename, bmp_header *head, bmp_info *info, char *data);
char *loadbm3(const char *filename, bm3_info *info);
int savebm3(const char *filename, bm3_info *info, char *data);

#endif

