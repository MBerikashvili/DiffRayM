#pragma once

#include "../integration/angular.h"

class StatsAngle: public AngleStep {
	
	public:
	bool wasIncluded;
	double weight;
	StatsAngle(
	    double cphi, double ctheta, double cdphi, double cdtheta
	):AngleStep(
	    cphi, ctheta, cdphi, cdtheta
	)
	{
		wasIncluded = false;
		weight = 0.;
	}

	
};