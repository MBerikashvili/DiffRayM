#include <stdarg.h>
#include <vector>
#include <fstream>
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

void CDebugger::writeToFile(const char* filename, const char* format, ...) {
        // 1. Обробка списку аргументів (ваші старі знайомі)
        va_list args;
        
        // --- КРОК 1: Визначаємо довжину повідомлення ---
        va_start(args, format);
        // vsnprintf з nullptr замість буфера повертає кількість символів, які б записалися
        int size = vsnprintf(nullptr, 0, format, args);
        va_end(args);

        if (size <= 0) return; // Якщо щось пішло не так

        // --- КРОК 2: Форматуємо рядок у тимчасовий буфер ---
        std::vector<char> buffer(size + 1); // +1 для нуль-термінатора \0
        va_start(args, format);
        vsnprintf(buffer.data(), buffer.size(), format, args);
        va_end(args);

        // Запис у файл
        std::ofstream file(filename, std::ios::app);
        if (file.is_open()) {
            file << buffer.data() << std::endl;
            file.close();
        }
    }
