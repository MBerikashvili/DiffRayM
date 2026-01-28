#include "basics.h"
#include "geom3d.h"
#include "solver.h"
#include "rt.h"
#include "geometry.h"
#include "continuum.h"
#include "lines.h"
#include "integration.h"
#include "app.h"
#include "math.h"
#include "abund.h"
#include "grain_temp.h"
#include "integration/angular.h"
#include "output/debugger.h"

int CIntegration::mode = 1;
double *CIntegration::flux_lines;
double *CIntegration::flux_continua;
double *CIntegration::flux_transitions;
double *CIntegration::flux_intrinsic;
int CIntegration::nPhi = 1;
int CIntegration::nTheta = 1;
double CIntegration::lumfactor;
long long CIntegration::memPerGeom = 0;
long long CIntegration::memPerRT = 0;
int CIntegration::nRay = 0;
bool CIntegration::isCentralRay = false;

bool CIntegration::iterateOverSource(double phi, double theta, double dphi, double dtheta)
{
	if(App::isIrregularSource)
		return true;

	//if source within app position, and fit it comletely
	double objectWidth = 2 * CGeometry::inRadius[0];
	if(dphi * App::distance < objectWidth)
		return false;
	if(dtheta * App::distance < objectWidth)
		return false;

	while(phi > M_PI)
		phi -= 2*M_PI;

	if((phi - dphi/2.) * App::distance < objectWidth / 2.0 &&
	    (phi + dphi/2.) * App::distance > objectWidth / 2.0 &&
	    (theta - dtheta/2.) * App::distance < objectWidth / 2.0 &&
	    (theta + dtheta/2.) * App::distance > objectWidth / 2.0)
	{
		return false;
	}

	return true;
	
}

int CIntegration::statisticRay()
{
	App::isStatMode = true;
	App::rayIntegration = App::rayToObj;

	App::rayIntegration.angle.y += App::AppDPhi;
	App::rayIntegration.angle.z += App::AppDTheta;

	for(int i=0;i<Abund::nElements;i++)
	{
		Abund::abmass[i] = 0.;
		Abund::abemso[i] = 0.;
	}
	Abund::mass = 0.;
	Abund::emso = 0.;

	CRT::calc_ray(1);
	App::isStatMode = false;
	return 1;
}

int CIntegration::InitIntegration(int mode,int nPrecision)
{
	CDebugger::debug("Init Integration: %d,%d\n",mode,nPrecision);
	CIntegration::mode = mode;

	for(int i = 0; i < CContinuum::nbands; i++)
	{
		CContinuum::bands[i].totalLum = 0.;
	}

	for(int ic=0;ic<CContinuum::cellCount; ic++)
	{
		CIntegration::flux_continua[ic] = 0.0;
		CIntegration::flux_transitions[ic] = 0.0;
		CIntegration::flux_intrinsic[ic] = 0.0;
	}

	for(int il=0;il<CLine::linesCount;il++)
	{
		CIntegration::flux_lines[il] = 0.0;
		CLine::cumulative[il] = 0.0;
	
	}

	if(mode>0) //not outward only
	{
		CIntegration::nPhi   = nPrecision*2+1;
		CIntegration::nTheta = nPrecision+1;
		//printf("PREC: %le/%d\n",M_PI,nPrecision);
		//printf("nPhi: %d\n",CIntegration::nPhi);
	}
	else
	{
		CIntegration::nPhi = 1;
		CIntegration::nTheta = 1;
	}

	for(int i=0;i<Abund::nElements;i++)
	{
		Abund::abmass[i] = 0.;
		Abund::abemso[i] = 0.;
	}
	for(int i=0;i<100;i++)
	{
		GrainTemp::massWeighted[i] = 0.0;
	}
	GrainTemp::massTotal = 0.0;
	Abund::mass = 0.;
	Abund::emso = 0.;
	CIntegration::nRay = 0;
	return 1;

}

int CIntegration::doCalc()
{
	printf("do Calc: %d\n", CIntegration::mode);
	
	App::rayIntegration = App::rayToObj;
	double lumfactor = 1.0;
				
	long long sum = 0;
	CIntegration::memPerGeom = 0;
	CIntegration::memPerRT = 0;

	if(CIntegration::mode == 0)
	{
		CIntegration::nRay++;
		CRT::dS = 4*M_PI/3;
		CRT::calc_ray(0);

		for(int i=0;i<CLine::linesCount;i++)
		{
			CIntegration::flux_lines[i] += CRT::ray_lines[i];
		}



		for(int i=0;i<CContinuum::cellCount;i++)
		{
			CIntegration::flux_continua[i] += CRT::ray_cont[i];
			CIntegration::flux_transitions[i] += CRT::ray_trans[i]*CRT::dS;
			CIntegration::flux_intrinsic[i] += CRT::ray_intr[i];
		}

		
		
	}
	else
	{
		//printf("nPhi: %d\n", CIntegration::nPhi);
		double dphi = App::phi_width/CIntegration::nPhi;
		double dtheta = App::theta_width/CIntegration::nTheta;
		CDebugger::debug("nPhi: %d; nTheta: %d\n",CIntegration::nPhi,CIntegration::nTheta);
		double phi = App::AppDPhi - (App::phi_width-dphi)/2;
		double theta = App::AppDTheta - (App::theta_width-dtheta)/2;
		
		double phi_max = App::AppDPhi + (App::phi_width-dphi)/2;
		double theta_max = App::AppDTheta + (App::theta_width-dtheta)/2;			

		double STOT = 0.0;
		int iskip_phi = 0;
		int iskip_theta = 0;
		int skip_param = 5;
		CDebugger::debug("Check need iter: %b, %le\n", App::isIrregularSource, CGeometry::inRadius[0]);
		printf("iterateSourceCheck\n");
		fflush(stdout);
		bool doIterationsOverSource = CIntegration::iterateOverSource(App::AppDPhi, App::AppDTheta, App::phi_width, App::theta_width);
		CDebugger::debug("START ITERATE OVER IRREGULAR GEOMETRY SOURCE: %b\n", doIterationsOverSource);
		fflush(stdout);
		if(!doIterationsOverSource)
		{
			//init intrinsic from one central ray
			App::rayIntegration.angle.y += App::AppDPhi;
			App::rayIntegration.angle.z += App::AppDTheta;
			CRT::obtainIntrinsic();
			/*for(int ic = 0; ic < CContinuum::cellCount; ic++)
			{
				if(ic < 840)
					printf("F=%le\n",CRT::ray_intr[ic]);
				CIntegration::flux_intrinsic[ic] = CRT::ray_intr[ic];
			}*/
			//exit(1);
		}
		CDebugger::debug("Initializing integrations\n");
		fflush(stdout);
		App::rayIntegration = App::rayToObj;
		CBasics::startClock();
		Angular* stepIterator = new Angular(App::rayIntegration.angle.y + App::AppDPhi, App::rayIntegration.angle.z + App::AppDTheta, App::phi_width, App::theta_width, 5, doIterationsOverSource);
		CDebugger::debug("Starting integration flow\n");
		fflush(stdout);
		printf("Iterate\n");
		stepIterator->iterate(doIterationsOverSource);
		CBasics::endClock();
		CDebugger::debug("STEP_S_TOT: %le; Scen: %le; Vcen: %le; [%le]\n",stepIterator->STot, stepIterator->Scenter, stepIterator->Vcenter, stepIterator->SMcenter);
		CDebugger::debug("phi: %le - %le; theta: %le - %le;\n",App::minPhiCen, App::maxPhiCen, App::minThetaCen, App::maxThetaCen);
		delete stepIterator;

		
		lumfactor = 1.0/(STOT);
		
	

	}
	
	
	

	CIntegration::lumfactor = lumfactor;
	CDebugger::log("INTEGRATION_ENDED\n");
	return 1;
}

int CIntegration::finish()
{
	//delete[] CIntegration::flux_continua;
	//delete[] CIntegration::flux_lines;
	return 1;
}