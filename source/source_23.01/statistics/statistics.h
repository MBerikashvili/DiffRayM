#pragma once

#include "stats_angle.h"

class CStatistics {
	public:
	static StatsAngle *angles[225];
	static bool wasInited;

	static bool initAngles();

	static double shouldInclude();

	static int run();
};