#pragma once
#include "../basics.h"
#include "output.h"

class CLogger extends Output {
	bool isDebugMode;
	
	CLogger()
	{
		super("apperture.log");
	}
}