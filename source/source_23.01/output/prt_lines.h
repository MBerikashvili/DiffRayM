#pragma once
#include "../basics.h"
#include "output.h"
#include "../lines.h"
#include "../integration.h"

class PrtLifr: public Output {
	public:
	
	PrtLifr():Output("lifr.dat")
	{
		//super("lifr.dat");
	}

	void print()
	{
		for(int i=0; i<CLine::linesCount; i++)
		{
			prt("%s\t",CLine::linesCapDB[CLine::lineIds[i]]);
		}
		prt("\n");
		for(int i=0; i<CLine::linesCount; i++)
		{
			prt("%le\t",CIntegration::flux_lines[i]);
		}
	}
};


class PrtLlum: public Output {
	public:
	
	PrtLlum():Output("llum.dat")
	{
		//super("lifr.dat");
	}

	void print()
	{
		for(int i=0; i<CLine::linesCount; i++)
		{
			prt("%s\t",CLine::linesCapDB[CLine::lineIds[i]]);
		}
		prt("\n");
		for(int i=0; i<CLine::linesCount; i++)
		{
			prt("%.2lf\t",CLine::toLuminosity(CIntegration::flux_lines[i]));
		}
	}
};


class PrtLinesarr: public Output {
	public:
	
	PrtLinesarr():Output("linesarr.dat")
	{
		//super("lifr.dat");
	}

	void print()
	{
		prt("Line\t\tFlux at observer\t\tLuminosity\n");
		
		for(int il=0;il<CLine::linesCount;il++)
		{
			prt("%s\t\t%le\t\t%lf\t\t%lf\n", CLine::linesCapDB[CLine::lineIds[il]], CIntegration::flux_lines[il],log10(1.e-50 + 4*M_PI*CIntegration::flux_lines[il]*pow(App::distance,2.0)),CIntegration::flux_lines[il]/(CIntegration::flux_lines[0]));
		}

	}
};