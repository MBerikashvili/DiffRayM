#pragma once
#include "basics.h"
#include "reader.h"

class Abund{
	public:
	static int nElements;
	static int nrows;
	static double ***abundances;
	static int nsectors;
	static int iSector;
	static int initSectorials(int nsectors);
	static int readAbunds(char fname[500],int iSector);
	static int putAbunds(int nlines);
	static int putAbundVal(int raw, int col, double val);
	static int getRadii(int nrows);
	static int getElementList(int irow,char str[1000]);
	static int refreshStatistics();
	static double statistics[500000][100];
	static int secRows[20];
	
	static char elements[100][14];
	static int readen;

	static double abmass[100];
	static double abemso[100];
	static double mass;
	static double emso;
};