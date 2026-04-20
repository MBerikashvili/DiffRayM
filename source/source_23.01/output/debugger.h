#pragma once
#include <string>
#include "../basics.h"
#include <iomanip>

class CDebugger
{
public:
	static int level;
	static void init(int level);
	static void write(const char* format, ...);
	static void debug(const char* format, ...);
	static void log(const char* format, ...);
	static void warn(const char* format, ...);
	static void error(const char* format, ...);

	static void writeToFile(std::time_t time, const char* format, ...);
};