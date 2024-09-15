#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bmp.h"

char *loadbmp(const char *filename, bmp_header *head, bmp_info *info)
{
	FILE *file;
	char id[2];

	file = fopen(filename, "rb");

	if (!file)
		return NULL;

	fread(id, sizeof(char), 2, file);

	if (strncmp(id, "BM", 2) != 0)
	{
		fclose(file);
		return NULL;
	}

	fread(head, sizeof(bmp_header), 1, file);
	fread(info, sizeof(bmp_info), 1, file);

	char *data = (char *)malloc(info->datasize * sizeof(char));
//	fseek(file, head->dataoffset, SEEK_SET);
	fread(data, sizeof(char), info->datasize, file);

	fclose(file);

	return data;
}

int savebmp(const char *filename, bmp_header *head, bmp_info *info, char *data)
{
	FILE *file;
	char id[] = "BM";

	file = fopen(filename, "wb");

	if (!file)
		return 0;

	fwrite(id, sizeof(char), 2, file);

	fwrite(head, sizeof(bmp_header), 1, file);
	fwrite(info, sizeof(bmp_info), 1, file);

//	fseek(file, head->dataoffset, SEEK_SET);
	fwrite(data, sizeof(char), info->datasize, file);

	fclose(file);

	return 1;
}

char *loadbm3(const char *filename, bm3_info *info)
{
	FILE *file;
	char id[3];

	file = fopen(filename, "rb");

	if (!file)
		return NULL;

	fread(id, sizeof(char), 3, file);

	if (strncmp(id, "BM3", 3) != 0)
	{
		fclose(file);
		return NULL;
	}

	fread(info, sizeof(bm3_info), 1, file);

	int datasize = info->width * info->height * info->depth * info->bpp/8;

	char *data = (char *)malloc(datasize * sizeof(char));
	fread(data, sizeof(char), datasize, file);

	fclose(file);

	return data;
}

int savebm3(const char *filename, bm3_info *info, char *data)
{
	FILE *file;
	char id[] = "BM3";

	file = fopen(filename, "wb");

	if (!file)
		return 0;

	fwrite(id, sizeof(char), 3, file);

	fwrite(info, sizeof(bm3_info), 1, file);

	int datasize = info->width * info->height * info->depth * info->bpp/8;
	fwrite(data, sizeof(char), datasize, file);

	fclose(file);

	return 1;
}

