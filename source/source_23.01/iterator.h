#include "basics.h"
#include "rt.h"
#include "geometry.h"
#include "continuum.h"
#include "lines.h"

class CIteration
{
public:
//	static double *line_fluxes;
//	static double *line_lumunosity;
//	static double *continuum_fluxes;
//	static double *totl_lines;
	static int nIteration;
	static int mode;
//	static double *line_fluxes_prev;
//	static double *continuum_fluxes_prev;

	static int initIteration(int mode);
	static int doIteration();
//	static double getdCont();
//	static double getdLine();
	static int obtainStatistics();
	static int finalizeIterations();
};