#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "sys/types.h"
#include "sys/sysinfo.h"


class CBasics{
    public:
	static int** int2Arr(int **arr, int sizex, int sizey, int defalultval);
	static double** float2Arr(double **arr, int sizex, int sizey, double defalultval);
	static int throwError(const char *string);
	static int startClock();
	static int endClock();
	static double timeStart;
};