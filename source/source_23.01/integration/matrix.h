#pragma once
#include "../basics.h"
#include "integrationRay.h"
#include "../continuum.h"
#include "../lines.h"
#include "../rt.h"
#include "../app.h"
#include "../solver.h"
#include "../const.h"
#include "../app.h"
#include "../output/debugger.h"

class CMatrix{
    public:
	static long double phi, theta, dphi, dtheta, dS;
	static long double calcPhi, calcTheta, calcDphi, calcDtheta, calcDS;
	static CIntegrationRay *rays[9];
	static double *ray_intr, *ray_cont, *ray_lines;
	static long double FFac;
	static double Scenter, Vcenter;

	static int setup(long double cphi, long double ctheta, long double cdphi, long double cdtheta)
	{
		CDebugger::debug("Init: %Le;%Le; %Le X %Le\n", cphi, ctheta, cdphi, cdtheta);
		CMatrix::phi = cphi;
		CMatrix::theta = ctheta;
		CMatrix::dphi = cdphi;
		CMatrix::dtheta = cdtheta;
		CMatrix::dS = dphi*dtheta;
		CMatrix::calcPhi = cphi;
		CMatrix::calcTheta = ctheta;
		CMatrix::calcDphi = cdphi;
		CMatrix::calcDtheta = cdtheta;
		CMatrix::calcDS = dphi*dtheta;
		CMatrix::Scenter = 0.;
		CMatrix::Vcenter = 0.;
		for(int ic = 0;ic < CContinuum::cellCount; ic++)
		{
			CMatrix::ray_intr[ic] = 0.;
			CMatrix::ray_cont[ic] = 0.;
		}
		CMatrix::initRays();
		return 1;
	}

	static int initRays()
	{
		
		for(int i = 0; i < 9; i++)
		{
			CMatrix::rays[i] = new CIntegrationRay(
				CMatrix::calcPhi + ((i % 3)-1)*CMatrix::calcDphi/3,
				CMatrix::calcTheta + (floor(i/3)-1)*CMatrix::calcDtheta/3,
				CMatrix::calcDphi/3,
				CMatrix::calcDtheta/3
			);
			CMatrix::rays[i]->initMesh(CContinuum::cellCount);
			CMatrix::rays[i]->initLines(CLine::linesCount);
		}
		return 1;
	}

	static int run(bool bWithStat, bool doIntrIter)
	{
		CMatrix::FFac = 0.;
		//CDebugger::debug("start run");
		for(int i=0;i<9; i++)
		{
			App::rayIntegration = App::rayToObj;

			App::rayIntegration.angle.y = CMatrix::rays[i]->phi;
			App::rayIntegration.angle.z = CMatrix::rays[i]->theta;
			CRT::dS = CMatrix::rays[i]->dS;
			if(i == 4 && bWithStat)
			    App::punchStatistics = true;
			else
			    App::punchStatistics = false;
			//CDebugger::debug("calc ray");
			CRT::calc_ray(1);
			//CDebugger::debug("calced ray");
			
			for(int ic = 0; ic < CContinuum::cellCount; ic++)
			{
				if(doIntrIter)
					CMatrix::rays[i]->ray_intr[ic] = CRT::ray_intr[ic];
				
				CMatrix::rays[i]->ray_cont[ic] = CRT::ray_cont[ic];
			}
			for(int il = 0; il < CLine::linesCount; il++)
			{
				CMatrix::rays[i]->ray_line[il] = CRT::ray_lines[il];
			}
			

			/*for(int ic = 0; ic < CContinuum::cellCount; ic++)
			{
				if(doIntrIter)
					CMatrix::rays[i]->ray_intr[ic] = CRT::dS;
				
				CMatrix::rays[i]->ray_cont[ic] = CRT::dS;
			}
			for(int il = 0; il < CLine::linesCount; il++)
			{
				CMatrix::rays[i]->ray_line[il] = CRT::dS;
			}*/

			if(CRT::Hcenter > 1.e-50)
			{
				CMatrix::Vcenter += CRT::Hcenter;
				CMatrix::Scenter += CRT::dS*App::distance*App::distance;
				if(App::maxPhiCen < CMatrix::rays[i]->phi)
					App::maxPhiCen = CMatrix::rays[i]->phi;
				if(App::minPhiCen > CMatrix::rays[i]->phi)
					App::minPhiCen = CMatrix::rays[i]->phi;
				if(App::maxThetaCen < CMatrix::rays[i]->theta)
					App::maxThetaCen = CMatrix::rays[i]->theta;
				if(App::minThetaCen > CMatrix::rays[i]->theta)
					App::minThetaCen = CMatrix::rays[i]->theta;
				
			} 
		
		}
		//CDebugger::debug("end run\n");
		CMatrix::FFac = CMatrix::getRaysFi();
		//CDebugger::debug("got ffac");
		return 1;
	}

	static void addFluxes(double &nPhots);

	static double getRaysFi();

	static bool getDelta(long double nPhots)
	{
		CDebugger::debug("getDelta: %Le of %le\n",CMatrix::FFac, nPhots);
		for(int i = 0; i < 9; i++)
		{
			double deltaHere = fabs(1.e-50 + CMatrix::rays[i]->Fi - CMatrix::FFac)/(CMatrix::FFac + 1.e-45);
			//if more than 15% fluctuations within matrix found - there should be smthg interesting
			if(!CMatrix::isErrorAcceptable(nPhots, deltaHere)) {
				CDebugger::debug("%d [%Le;%Le]Delta here %le; %Le - %Le\n", i, CMatrix::rays[i]->phi, CMatrix::rays[i]->theta, deltaHere, CMatrix::rays[i]->Fi, CMatrix::FFac);
				return false;
			}
		}
		return true;
	}

	static bool freeMem()
	{
		for(int i = 0; i < 9; i++)
		{
			delete []CMatrix::rays[i]->ray_intr;
			delete []CMatrix::rays[i]->ray_line;
			delete []CMatrix::rays[i]->ray_cont;
			delete CMatrix::rays[i];
		}
		return true;
	}

	static bool isErrorAcceptable(double nPhots, double deltaHere)
	{
		//Nothing to do here, fluxes are close to zero so far
		if(CMatrix::FFac * deltaHere <= SMALL_DELTA)
		    return true;


		if(deltaHere < 0.05)
		    return true;
		//Too great deviations. Even if fluxes are small comparing to 
		//total amount of photons - we should never accept deviations of order
		//since they can mean powerfull sources, weaken by small resolution
		if(deltaHere > 1.e+1)
		    return false;

		long double deviation = CMatrix::FFac * deltaHere;
		
		//get nPhots per radian
		//long double nPhotsPerRadian = nPhots / (App::phi_width * App::theta_width);
		if(CMatrix::dS/(App::phi_width * App::theta_width) < 1.e-4)
		{
			CDebugger::debug("TooSmall\n");
			return true;
		}
		
		if(deviation > log10(1.00001+App::precision)*(nPhots + CMatrix::FFac))
		{
			CDebugger::debug("DELTA [%Le > %le x %Le] %Le\n", 
			    deviation,
			    log10(1.00001+App::precision),
			    nPhots + CMatrix::FFac,
			    log10(1.00001+App::precision)*(nPhots + CMatrix::FFac)
			);
			return false;
		}
		if(CMatrix::dS/(App::phi_width * App::theta_width) > 1./775.0)
		{
			CDebugger::debug("TooLarge: %le/%le > 0.0133\n", CMatrix::dS, App::phi_width * App::theta_width);
			return false;
		}

		if(deviation < 0.2*log10(1.00001+App::precision)*(nPhots + CMatrix::FFac))
			return true;
		
		if(CMatrix::dS/(App::phi_width * App::theta_width) > 1.e-4)
		{
			CDebugger::debug("Too Big: %Le/%le x %le > 0.001;\n", CMatrix::dS, App::phi_width, App::theta_width);
			CDebugger::debug("DELTA [%Le > %le x %Le] %Le\n", 
			    deviation,
			    0.2*log10(1.00001+App::precision),
			    nPhots + CMatrix::FFac,
			    0.2*log10(1.00001+App::precision)*(nPhots + CMatrix::FFac)
			);
			return false;
		}
		
		return true;
	}
};