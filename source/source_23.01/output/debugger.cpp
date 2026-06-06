#include <stdarg.h>
#include <vector>
#include <fstream>
#include "debugger.h"
#include <sstream>
#include "../app.h"

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
		va_list args;
		va_start(args,format);
		printf("WARN: ");
		vprintf(format, args);
		printf("\n");
		va_end(args);
	}
}

void CDebugger::error(const char *format, ...)
{
	va_list args;
	va_start(args,format);
	printf("ERROR: ");
	vprintf(format, args);
	printf("\n");
	va_end(args);
}

void CDebugger::writeToFile(std::time_t time, const char* format, ...) 
{
    va_list args;
    va_start(args, format);
    int size = vsnprintf(nullptr, 0, format, args);
    va_end(args);

    if (size <= 0) return;

    std::vector<char> buffer(size + 1);
    va_start(args, format);
    vsnprintf(buffer.data(), buffer.size(), format, args);
    va_end(args);

    std::stringstream filename;
    filename << std::put_time(std::localtime(&time), "/%d-%m-%Y %H:%M:%S");

    std::string path = std::string(App::anglesLogsDirectory) + filename.str();

    std::ofstream file(path, std::ios::app);
    if (file.is_open()) {
        file << buffer.data() << std::endl;
        file.close();
    }
}