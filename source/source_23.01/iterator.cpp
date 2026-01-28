#include "basics.h"
#include "rt.h"
#include "geometry.h"
#include "continuum.h"
#include "lines.h"
#include "integration.h"
#include "iterator.h"
#include "output/debugger.h"

//double *CIteration::line_fluxes;
//double *CIteration::line_lumunosity;
//double *CIteration::continuum_fluxes;
//double *CIteration::totl_lines;
int CIteration::nIteration;
int CIteration::mode;
//double *CIteration::line_fluxes_prev;
//double *CIteration::continuum_fluxes_prev;

int CIteration::obtainStatistics()
{
	CIntegration::statisticRay();
	return 1; //!!!MB
}

int CIteration::initIteration(int mode)
{
	CDebugger::log("initIteration\n");
	/*
	CIteration::line_fluxes = new double[CLine::linesCount];
	CIteration::line_lumunosity = new double[CLine::linesCount];
	CIteration::continuum_fluxes = new double[CContinuum::cellCount];
	CIteration::totl_lines = new double[CLine::linesCount];
	CIteration::line_fluxes_prev = new double[CLine::linesCount];
	CIteration::continuum_fluxes_prev = new double[CContinuum::cellCount];
	*/
	CIteration::nIteration = 1;
	CIteration::mode = mode;
	/*
	
	for(int i=0;i<CLine::linesCount;i++)
	{
		CIteration::line_fluxes_prev[i] = 0.0;
	}

	for(int i=0;i<CContinuum::cellCount;i++)
	{
		CIteration::continuum_fluxes_prev[i] = 0.0;
	}
	*/
	CIntegration::flux_continua = new double[CContinuum::cellCount];
	CIntegration::flux_lines = new double[CLine::linesCount];
	CIntegration::flux_transitions = new double[CContinuum::cellCount];
	CIntegration::flux_intrinsic = new double[CContinuum::cellCount];
	
	CRT::ray_cont = new double[CContinuum::cellCount];
	CRT::ray_lines = new double[CLine::linesCount];
	CRT::ray_trans = new double[CContinuum::cellCount];
	CRT::ray_intr = new double[CContinuum::cellCount];
	return 1;
}

int CIteration::doIteration()
{
	CDebugger::log("doIteration: %d\n", CIteration::mode);
	/*
	if(CIteration::nIteration>1)
	{
		for(int i=0;i<CLine::linesCount;i++)
		{
			CIteration::line_fluxes_prev[i] = CIteration::line_fluxes[i];
		}

		for(int i=0;i<CContinuum::cellCount;i++)
		{
			CIteration::continuum_fluxes_prev[i] = CIteration::continuum_fluxes[i];
		}
	}
	*/
	CIntegration::InitIntegration(CIteration::mode,6+4*nIteration);
	CIntegration::doCalc();

	
	CIntegration::finish();
	return 1;
}

int CIteration::finalizeIterations()
{
	delete []CIntegration::flux_continua;
	delete []CIntegration::flux_lines;
	delete []CIntegration::flux_transitions;
	delete []CIntegration::flux_intrinsic;
	
	delete []CRT::ray_cont;
	delete []CRT::ray_lines;
	delete []CRT::ray_trans;
	delete []CRT::ray_intr;
	return 1;
}