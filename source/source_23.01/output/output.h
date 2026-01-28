#pragma once
#include <stdarg.h>
#include "../app.h"
#include <dirent.h>
#include <sys/stat.h>


class Output{
	
	public:
		bool isOpened;
		FILE *FP;

	Output(const char *fname, const char* mode = "w+")
	{
		printf("Try to open\n");
		isOpened = false;
		char ffname[255];
		sprintf(ffname, "%s/%s",App::output_dir, fname);
		printf(ffname);
		printf("\n%s\n",mode);
		FP = fopen(ffname, mode);
		if(FP)
			isOpened = true;
		else
		    {
			printf("Failed to open file %s\n",ffname);
			exit(1);
		    }
	}

	bool prt(const char* format, ...)
	{
		va_list ap;
		va_start(ap,format);
		if(!isOpened)
			return false;
		vfprintf(FP, format, ap);
		va_end(ap);
		return true;
	}

	~Output()
	{
		if(isOpened)
			fclose(FP);
	}

};