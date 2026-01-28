#pragma once
#include "../basics.h"
#include "../app.h"

class CIntegrationRay {
	
	public:
		long double phi, theta, dphi, dtheta, dS, Fi;
		double *ray_intr, *ray_cont, *ray_line;
		bool isProcessed;
	CIntegrationRay(long double rphi, long double rtheta, long double rdphi, long double rdtheta)
	{
		phi = rphi;
		theta = rtheta;
		dphi = rdphi;
		dtheta = rdtheta;
		if(!App::isSpacial) 
		{
			dS = tan(dphi)*tan(dtheta);
		}
		else
		{
			dS = dphi * dtheta * sin(theta);
		}
		
		isProcessed = false;
		Fi = 0;
	}

	int initMesh(int cellCount)
	{
		ray_intr = new double[cellCount];
		ray_cont = new double[cellCount];
		for(int ic=0;ic<cellCount;ic++)
		{
			ray_intr[ic] = 0.;
			ray_cont[ic] = 0.;
		}
		return 1;
	}

	int initLines(int linesCount)
	{
		ray_line = new double[linesCount];
		for(int il=0;il<linesCount;il++)
		{
			ray_line[il] = 0.;
		}
		return 1;
	}

	bool setProcessed(bool flag)
	{
		isProcessed = flag;
		return flag;
	}

	double addFi(int low, int hii)
	{
		double delta = 0.;
		for(int i = low; i < hii; i++)
		{
			delta += (ray_intr[i] + ray_cont[i]);
		}
		Fi += delta;
		return delta;
	}

	double resetFi()
	{
		Fi = 0.;
		return Fi;
	}
};