#pragma once
#include "basics.h"
#include "reader.h"

class GrainTemp{
	public:
	static int nBins;
	static int nrows;
	static double ***gridTemps;
	static int nsectors;
	static int iSector;
	static int initSectorials(int nsectors);
	static int readTemps(char fname[500],int iSector);
	static int putTemps(int nlines);
	static int putTempVal(int raw, int col, double val);
	static int getRadii(int nrows);
	static double statistics[500000][100];
	static int refreshStatistics();
	static double massWeighted[100];
	static double massTotal;
};