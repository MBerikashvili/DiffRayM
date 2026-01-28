#pragma once
#include "basics.h"
#include "geom3d.h"
#include "solver.h"
#include "rt.h"
#include "geometry.h"
#include "continuum.h"
#include "lines.h"

class CIntegration
{
public:
	static int mode;
	static double *flux_lines;
	static double *flux_continua;
	static double *flux_transitions;
	static double *flux_intrinsic;
	static int nPhi,nTheta;
	static double lumfactor;
	static int InitIntegration(int mode=0,int nPrecision=10);
	static int doCalc();
	static int finish();
	static int statisticRay();
	static long long memPerGeom;
	static long long memPerRT;
	static int nRay;
	static bool isCentralRay;
	static bool iterateOverSource(double phi, double theta, double dphi, double dtheta);
};