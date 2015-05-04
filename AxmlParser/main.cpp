// AxmlParser.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ParseUtil.h"
#include "AxmlParser.h"
#include "FileUtil.h"

#ifdef _WIN32		/* windows */
#pragma warning(disable:4996)
#endif

int main(int argc, char *argv[])
{
	FILE *fp;
	char *inbuf;
	size_t insize;
	int ret;

	/*if(argc != 2)
	{
	fprintf(stderr, "Usage: AxmlPrinter <axmlfile>\n");
	return -1;
	}*/

	//fp = fopen(argv[1], "rb");
	fp = fopen("AndroidManifest.xml", "rb");
	if (fp == NULL)
	{
		fprintf(stderr, "Error: open input file failed.\n");
		return WRONG_FILE;
	}

	fseek(fp, 0, SEEK_END);
	insize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	inbuf = (char *)malloc(insize * sizeof(char));
	if (inbuf == NULL)
	{
		fprintf(stderr, "Error: init file buffer.\n");
		fclose(fp);
		return OUTOF_MEMORY;
	}

	ret = fread(inbuf, 1, insize, fp);
	if (ret != insize)
	{
		fprintf(stderr, "Error: read file.\n");
		free(inbuf);
		fclose(fp);
		return WRONG_FILE;
	}

	ret = AxmlToXml(inbuf, insize);

	free(inbuf);
	fclose(fp);

	
	getchar();
	return ret;
}

