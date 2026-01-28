#pragma once
#include "basics.h"

class CGeometry{
public:
	static int nSectors;
	static int iSector;
	static int *nRadiuses;
	static double *inRadius;
	static double **outer_radius;
	static int readSectors(char fpattern[500] = "");
    static int readRadiuses(char fpattern[500], int iSector, double inRad = 0.);
    static int putLineVal(int raw, int col, double val);
	static int getRadii(int nrows);
	
};