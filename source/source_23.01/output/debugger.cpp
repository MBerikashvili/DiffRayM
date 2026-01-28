#include <stdarg.h>
#include "debugger.h"

int CDebugger::level = 1;


void CDebugger::init(int level = 1) 
{
	CDebugger::level = level;	
}

void CDebugger::write(const char* format, ...)
{
	va_list ap;
	va_start(ap,format);
	vprintf(format, ap);
	va_end(ap);
}

void CDebugger::debug(const char* format, ...)
{
	if(CDebugger::level >= 2)
	{
		va_list ap;
		va_start(ap,format);
		printf("DEBUG: ");
		vprintf(format, ap);
		printf("\n");
		va_end(ap);
	}
}

void CDebugger::log(const char* format, ...)
{
	if(CDebugger::level >= 1)
	{
		va_list ap;
		va_start(ap,format);
		printf("LOG: ");
		vprintf(format, ap);
		printf("\n");
		va_end(ap);
	}
}

void CDebugger::warn(const char* format, ...)
{
	if(CDebugger::level >= 0)
	{
		va_list ap;
		va_start(ap,format);
		printf("WARN: ");
		vprintf(format, ap);
		printf("\n");
		va_end(ap);
	}
}

void CDebugger::error(const char *format, ...)
{
	va_list ap;
	va_start(ap,format);
	printf("ERROR: ");
	vprintf(format, ap);
	printf("\n");
	va_end(ap);
}
