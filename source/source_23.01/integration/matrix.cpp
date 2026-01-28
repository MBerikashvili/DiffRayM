#include "matrix.h"
#include "../integration.h"
#include "../continuum.h"
#include "../lines.h"
#include "../spectra/isophote.h"

long double 	CMatrix::phi = 0.;
long double 	CMatrix::theta = 0.;
long double 	CMatrix::dphi = 0.;
long double 	CMatrix::dtheta = 0.;
long double 	CMatrix::dS = 0.;
long double 	CMatrix::calcPhi = 0.;
long double 	CMatrix::calcTheta = 0.;
long double 	CMatrix::calcDphi = 0.;
long double 	CMatrix::calcDtheta = 0.;
long double 	CMatrix::calcDS = 0.;
long double 	CMatrix::FFac = 0.;
CIntegrationRay *CMatrix::rays[9];
double *CMatrix::ray_intr;
double *CMatrix::ray_cont; 
double *CMatrix::ray_lines;
double CMatrix::Vcenter;
double CMatrix::Scenter;

void CMatrix::addFluxes(double &nPhots)
	{
		if(CMatrix::FFac > 0)
		{
			for(int i=0; i<9; i++)
			{
				CIsophotes::addFlux(CMatrix::rays[i]->phi, CMatrix::rays[i]->theta, CMatrix::rays[i]->dphi, CMatrix::rays[i]->dtheta, CMatrix::rays[i]->ray_cont, CMatrix::rays[i]->ray_line);
			}
		}
		for(int ic = 0; ic < CContinuum::cellCount; ic++)
		{
			double totlQuanta = 0;
			for(int i=0; i<9; i++)
			{
				CIntegration::flux_intrinsic[ic] += CMatrix::rays[i]->ray_intr[ic];
				CIntegration::flux_continua[ic] += CMatrix::rays[i]->ray_cont[ic];
				totlQuanta += CMatrix::rays[i]->ray_intr[ic] + CMatrix::rays[i]->ray_cont[ic];
			}
			for(int iiC = 0; iiC < 2; iiC++)
			{
				if(App::CalcCont && ic >= CContinuum::importantCells[iiC][0]
				    && ic <= CContinuum::importantCells[iiC][1])
					nPhots += totlQuanta;
			}
		}
		for(int il=0;il<CLine::linesCount;il++)
		{
			for(int i=0; i<9; i++)
			{
				CIntegration::flux_lines[il] += CMatrix::rays[i]->ray_line[il];
				if(App::CalcLines && !App::CalcCont && il == 0)
					nPhots += CMatrix::rays[i]->ray_line[0];
			}
		}
	}

double CMatrix::getRaysFi()
{
	for(int i = 0; i<9; i++)
	{
		CMatrix::rays[i]->resetFi();
	}
	double totl = 0.;
	if(App::CalcCont)
	{
		for(int iiC = 0; iiC < 2; iiC++)
		{
			double iLeft = CContinuum::importantCells[iiC][0];
			double iRight = CContinuum::importantCells[iiC][1];
			for(int i = 0; i<9; i++)
			{
				totl += pow(CMatrix::rays[i]->addFi(iLeft, iRight),2.0);
			}
		}
	} else {
		for(int i = 0; i<9; i++)
		{
			CMatrix::rays[i]->Fi = CMatrix::rays[i]->ray_line[0];
			totl += pow(CMatrix::rays[i]->ray_line[0],2.0);
		}
	}
	return sqrt(totl)/3;
}