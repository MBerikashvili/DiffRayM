#pragma once
#include "basics.h"

class CReader{
public:
    static int readLine(FILE *MF, char *string,int(*fn)(int));
    static char** read_commands(FILE *MF);
    static int calc_lines_number(FILE *MF, int maxsize = 255);
    static int read_file_array(FILE *MF,int maxsize=255,int nskiprows=0, int nskiprcols=0, int (*oncalcrows)(int) = 0, int (*oncalccols)(int) = 0, int (*onelementset)(int,int,double) = 0, int (*onskiprow)(int,char[1000]) = 0);
    static int calc_cols_number(char *line, const char *pattern = "%s[\t\r\n]");
    static int callable(int numb);
};