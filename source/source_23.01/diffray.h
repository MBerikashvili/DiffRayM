#include "basics.h"
#include "rt.h"
#include "geometry.h"
#include "continuum.h"
#include "lines.h"

class CDiffRay
{
public:
	static int nPoints;
	static double pointsArr[4000];
	static int initDiffRay();
	static int runDiffRay(bool usePoints, int nPoint);
	static int initializePoints();
	static int launch();
	static int getPointsProcessed();
};