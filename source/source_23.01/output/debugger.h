#pragma once
#include "../basics.h"

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
};