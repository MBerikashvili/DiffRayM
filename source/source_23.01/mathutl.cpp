#include "mathutl.h"
#include <math.h>

int sign(double A)
{
	if(A == 0)
		return 0;

	return round(A/fabs(A));
}

int floorAbs(double A)
{
	return sign(A)*floor(fabs(A));
}