#pragma once
#include "basics.h"
#include "geom3d.h"

struct intersection
{
	vector3 point;
	int sec_p,sec_n;
	int lay_p,lay_n;
	long double R;

	int is;
	double phi;

	intersection()
	{
		sec_p = -1;
		sec_n = -1;
		lay_p = -1;
		lay_n = -1;
		R = 0.;
		is = -1;
		phi = 0.;
	}
};

struct  TPath
{
	int layer;
	int sector;
	long double path;
};

class CSolver
{
public:
	static intersection points[5000];
	static TPath paths[5000];
	static int npoints;

	static int getPoints();
	static int getPointsSectors();
	static int getPointsCones();
};