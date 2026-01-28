#pragma once
#include "basics.h"
#include "reader.h"

const double LIM_TEMP_CAERN = 4.e+4;

class Physics{
	public:
	static int nrows;
	static double **Te;
	static double **Ne;
	static double **nH;
	static int nsectors;
	static int iSector;
	static int initSectorials(int nsectors);
	static int readPhysics(char fname[500],int iSector);
	static int putPhysics(int nlines);
	static int putPhysicsVal(int raw, int col, double val);
	static int getRadii(int nrows);
	
	static int readen;
	static double statistics[500000][100];
	static int refreshStatistics();

};