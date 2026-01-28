#pragma once
#include "../basics.h"
#include "output.h"
#include "../continuum.h"
#include "../integration.h"
#include "../const.h"

class PrtContarr: public Output {
	public:
//!!!BM	PrtContarr(char *fname):Output(fname)
	PrtContarr():Output("contarr.dat")
	{
		//super("lifr.dat");
	}

	void print()
	{
		for(int i=0; i<CContinuum::cellCount; i++)
		{
			prt("%le\t%le\t%le\n",CContinuum::anu[i], CIntegration::flux_intrinsic[i], CIntegration::flux_continua[i]);
		}
	}
};

class PrtSpectra: public Output {
	public:
	
	PrtSpectra():Output("spectraContinuum.dat")
	{
		//super("lifr.dat");
	}

	void print()
	{
		for(int i=0; i<CContinuum::cellCount; i++)
		{
			double cn = EN1RYD*pow(CContinuum::anu[i],2.0)/(CContinuum::anu[i] - CContinuum::anu[i-1]);
			cn *= pow(App::distance/CGeometry::inRadius[0],2.0);
			prt("%le\t%le\t%le\n",CContinuum::anu[i], CIntegration::flux_intrinsic[i]*cn, CIntegration::flux_continua[i]*cn);
		}
	}
};
