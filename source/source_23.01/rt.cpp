#include "basics.h"
#include "geom3d.h"
#include "solver.h"
#include "rt.h"
#include "geometry.h"
#include "continuum.h"
#include "lines.h"
#include "app.h"
#include "integration.h"
#include "output/debugger.h"
#include "statistics/statistics.h"

double *CRT::ray_cont;
double *CRT::ray_lines;
double *CRT::ray_trans;
double *CRT::ray_intr;
long double CRT::dS;
double CRT::Hcenter;
bool CRT::dumpRay;
double CRT::tau_this_zone[5000];

int CRT::get_mesh_ind(double en)
{
	int im=0;
	while(im<CContinuum::cellCount && en>CContinuum::anu[im])
	{
		im++;
	}

	return im;
}

int CRT::calc_ray(int mode)
{

	//CDebugger::debug("getting points");
	CSolver::getPoints();
	//CDebugger::debug("points got");
	CRT::calc_cont(mode);
	return 1;
}

int CRT::obtainIntrinsic()
{
	double Rin = CGeometry::inRadius[0];
	double path = Rin;
	CSolver::getPoints();
	for(int i=0; i < CContinuum::cellCount; i++)
	{
		CRT::tau_this_zone[i] = 0.;
	}
	bool passedCenter = false;
	for(int ish = 0;ish < CSolver::npoints - 1;ish++)
	{
		
		
		int il = CSolver::paths[ish].layer;
		int is = CSolver::paths[ish].sector;
		if(il == -1)
		{
			passedCenter = true;
		}
		
		double h = CSolver::paths[ish].path;
		path += h;
		if(path <= 1.e-50)
			continue;
		double dF = pow(Rin/path, 2.0);
		if(is >= 0) //>=
		{
			for(int i=0; i < CContinuum::cellCount; i++)
			{
				double emitLocalIntr = 0.;
				if(il == -1) {
					emitLocalIntr = CContinuum::in_fluxes[i];
				}
				
				double opfac = 1.0;
				if(CRT::tau_this_zone[i] > 1.e-4)
					opfac = exp(-CRT::tau_this_zone[i]);
				CRT::ray_intr[i] = emitLocalIntr*dF*opfac;
				if(il >= 0)
				    CRT::tau_this_zone[i] += h*CContinuum::opacs[is][il][i];
				
			}
			if(il == -1)
				return 1;
		}
	}
	return 1;
}

int CRT::obtainContinua()
{
	long double dF = 1.0;
	double path = 0.;
	CRT::Hcenter = 0.;
	for(int i=0; i < CContinuum::cellCount; i++)
	{
		CRT::tau_this_zone[i] = 0.;
	}
/*
	printf("RAY: %d %Le\n", CSolver::npoints, CRT::dS);
			    double max = 0.;
			    double maxOpac = 0.;
			    long double maxdS = 0.;
			    double maxh = 0.;
			    printf("linC: %d\n", CLine::linesCount);
*/
	for(int ish = 0;ish < CSolver::npoints - 1;ish++)
	{
		double h = CSolver::paths[ish].path;
		path += h;
		if(path <= 1.e-50)
			continue;

		int il = CSolver::paths[ish].layer;
		int is = CSolver::paths[ish].sector;
		//dF = (0.5/M_PI)*(1 - sqrt(1/(1 + path*path*CRT::dS/pow(path - h/2., 2.0))));
		dF = (0.5/M_PI)*(1 - sqrt(1/(1+CRT::dS)));
		if(is >= 0 && (App::onlySectorNo < 0 || is == App::onlySectorNo)) //>=
		{
			if(il == -1)
			    CRT::Hcenter += h*CRT::dS*path*path;
			//get emisivity
			if(App::CalcCont)
			{
			    for(int i=0; i < CContinuum::cellCount; i++)
			    {
				double emitLocal = 0.;
				double emitLocalIntr = 0.;
				double tau_this = 0.;
				double transEmis = 0.;
				if(il >= 0) {
					//transEmis = h*CContinuum::trans[is][il][i];
					emitLocal = h*CContinuum::emits[is][il][i];
					tau_this = h*CContinuum::opacs[is][il][i];
				}
				if(il == -1) {
					//if(i==1)
					//	printf("passed center: %Le - %Le; theta = %Le\n", CRT::dS, dF, App::rayIntegration.angle.z);
					
					emitLocalIntr = h*CContinuum::effectiveSourceEmissivity[i];
				
				}
				double opfac = 1.0;
				if(App::CalcOpac && CRT::tau_this_zone[i] > 1.e-4)
					opfac = exp(-CRT::tau_this_zone[i]);
				double opfac_this = 1.0;
				if(App::CalcOpac && tau_this > 1.e-4)
					opfac_this = (1 - exp(-tau_this))/tau_this;
				CRT::ray_cont[i] += emitLocal*dF*opfac*opfac_this;
				CRT::ray_intr[i] += emitLocalIntr*dF*opfac;
				CRT::ray_trans[i] += transEmis*dF*opfac;
				if(il >= 0 && App::CalcOpac)
				    CRT::tau_this_zone[i] += h*CContinuum::opacs[is][il][i];
			    }
			}
			// CDebugger::debug("ipcont calculated");
			if(App::CalcLines)
			{
			    for(int i=0; i < CLine::linesCount; i++)
			    {
				double emitLocal = 0.;
				if(il >= 0) emitLocal = h*CLine::emits[is][il][i];
				double opfac = 1.0;
				int lId = CLine::lineIds[i];
				double en = CLine::linesEn[lId];
				if(en<=0)
				{
					en = 1.0;
				}

				int icell = CRT::get_mesh_ind(en);
				if(CRT::tau_this_zone[icell] > 1.e-4 && App::CalcOpac)
					opfac = exp(-CRT::tau_this_zone[icell]);
				double op_this = 1.;
				if(il >= 0 && App::CalcOpac)
				{
					double tau_this = h*CContinuum::opacs[is][il][icell];
					if(tau_this > 1.e-4)
					    op_this = (1 - exp(-tau_this))/tau_this;
				}
				CRT::ray_lines[i] += emitLocal*dF*opfac*op_this;
/*				
*/			    }
			    
			}
			
		}
	}
	return 1;
}

int CRT::calc_cont(int mode)
{
	
	//FILE *RT;

	//RT = fopen("ray.info","w+");
	for(int ic=0;ic<CContinuum::cellCount; ic++)
	{
		CRT::ray_cont[ic] = 0.0;
		CRT::ray_trans[ic] = 0.0;
		CRT::ray_intr[ic] = 0.0;
	}

	double *face_opacities;
	double *total_opacities;

	face_opacities = new double[CLine::linesCount];
	total_opacities = new double[CLine::linesCount];
	
	int lowp = CSolver::npoints-1;
	CRT::dumpRay = false;
	FILE* FP_DUMP;
	
	if(mode == 0)
	{
		for(int ish=CSolver::npoints-1;ish>=0;--ish)
		{
			int ilayer = CSolver::paths[ish].layer;
			if(ilayer == -1)
			{
				lowp = ish;
				break;
			}
		}
	}

	for(int il=0;il<CLine::linesCount;il++)
	{
		CRT::ray_lines[il] = 0.0;
		CLine::cumulative[il] = 0.0;
		face_opacities[il] = 0.0;
		total_opacities[il] = CRT::TauAbsToPoint(il,lowp);
	}


	//if(App::CalcCont)
	//CDebugger::debug("do calcs cont");
	CRT::obtainContinua();
	//CDebugger::debug("run statistics");


	CStatistics::run();

	delete[] face_opacities;
	delete[] total_opacities;

	return 1;
}

double CRT::TauAbsTotal(int iline)
{
	double en = CLine::linesEn[iline];

	if(en<=0)
	{
		en = 1.0;
	}

	int icell = CRT::get_mesh_ind(en);
	
	double tau = 1.e-40;

	if(icell<0 || icell>CContinuum::cellCount-1)
	    return 0.;
	
		
	for(int i=0;i<CSolver::npoints-2;i++)
	{
		int nlw = CSolver::paths[i].layer;
		int nsc = CSolver::paths[i].sector;
		
		tau += CContinuum::opacs[nsc][nlw][icell]*CSolver::paths[i].path;
	}
	
	return tau;
}


double CRT::TransferFromPoint(double emis, int icont, int from)
{
	//return emis;
	double tau = 1.e-50;
	double R0 = CSolver::paths[from].path;
	double R1 = R0;
	if(R0 < 1.e-50) //No emitting volume
		return 0.;
	for(int i=from-1;i>=0;i--)
	{
	//	int nlw = CSolver::paths[i].layer;
	//	int nsc = CSolver::paths[i].sector;
	//	R1 += CSolver::paths[i].path;
	//	if(nlw > -1 && nsc >= -1)
	//	    tau += CContinuum::opacs[nsc][nlw][icont]*CSolver::paths[i].path;
		tau += 1.e-5;
	}
	return emis*pow(R0/R1,2.0)*exp(-tau);
}

double CRT::TauAbsTotalCont(int icont)
{
	double tau = 1.e-50;
	
		
	for(int i=0;i<CSolver::npoints-2;i++)
	{
		int nlw = CSolver::paths[i].layer;
		int nsc = CSolver::paths[i].sector;
		
		tau += CContinuum::opacs[nsc][nlw][icont]*CSolver::paths[i].path;
	}
	
	return tau;
}


double CRT::TauAbsToPoint(int iline, int lowp)
{
	double en = CLine::linesEn[iline];

	if(en<=0)
	{
		en = 1.0;
	}

	int icell = CRT::get_mesh_ind(en);
	
	if(icell<0 || icell>CContinuum::cellCount-1)
	    return 0.;
	
	double tau = 1.e-40;
	
		
	for(int i=0;i<lowp;i++)
	{
		int nlw = CSolver::paths[i].layer;
		int nsc = CSolver::paths[i].sector;
		//printf("point: %d; mlw=%d\n",i,nlw);
		if(nlw>=0)
		{
			double dtau = CContinuum::opacs[nsc][nlw][icell]*CSolver::paths[i].path;
			
			tau += dtau;
		}
	}
	
	return tau;
}


double CRT::TauAbsToPointCont(int icont, int lowp)
{
	
	double tau = 1.e-40;
	
		
	for(int i=0;i<lowp;i++)
	{
		int nlw = CSolver::paths[i].layer;
		int nsc = CSolver::paths[i].sector;
		if(nlw>=0)
		{
			double dtau = CContinuum::opacs[nsc][nlw][icont]*CSolver::paths[i].path;
			if(dtau>1.e-40)
			tau += dtau;
		}
	}
	
	return tau;
}