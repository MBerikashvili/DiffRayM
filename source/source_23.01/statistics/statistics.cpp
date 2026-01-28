#include "statistics.h"
#include "stats_angle.h"
#include "../app.h"
#include "../const.h"
#include "../solver.h"
#include "../abund.h"
#include "../grain_temp.h"
#include "../physics.h"
#include "../geometry.h"
#include "../integration.h"
#include "../mathutl.h"

bool CStatistics::wasInited = false;
StatsAngle *CStatistics::angles[225];

bool CStatistics::initAngles()
{
	if(CStatistics::wasInited)
	{
		for(int i=0;i<225;i++)
		{
			delete CStatistics::angles[i];
		}
	}
	double phiw = App::phi_width / 15;
	double thetaw = App::theta_width / 15;
	
	for(int i = 0; i < 225; i++)
	{
		CStatistics::angles[i] = new StatsAngle(
			(floor(i / 15) - 8)*phiw + App::AppDPhi,
			((i % 15) - 8)*thetaw + App::AppDTheta,
			phiw,
			thetaw
		);
	}
	CStatistics::wasInited = true;
	return true;
}

double CStatistics::shouldInclude()
{
	long double rayPhi = App::rayIntegration.angle.y;
	long double rayTheta = App::rayIntegration.angle.z;
	int ip = 7 + floorAbs(15 * (rayPhi - App::AppDPhi - App::rayToObj.angle.y)/App::phi_width);
	int it = 7 + floorAbs(15 * (rayTheta - App::AppDTheta - App::rayToObj.angle.z)/App::theta_width);
	int i = ip*15 + it;
	//printf("i=%d! (%d;%d) <= (%le;%le;%le;%le) \n",i, ip,it, rayPhi, App::AppDPhi, rayTheta, App::AppDTheta);
	if(i < 0 || i > 225)
	{
		printf("i=%d! (%d;%d) <= (%Le;%Le;%Le) Something went not as intended\n",i, ip,it, rayTheta, App::AppDTheta, App::rayToObj.angle.z);
		exit(1);
	}
	if(!CStatistics::angles[i]->wasIncluded)
	{
		CStatistics::angles[i]->wasIncluded = true;
		return CStatistics::angles[i]->dS;
	}
	return 0.;
}

int CStatistics::run()
{
	//printf("BeforeStat\n");
	//printvec(App::rayToObj.angle);
			
	if(!CStatistics::wasInited)
		return 0;

	double dS = CStatistics::shouldInclude();

	if(dS <= SMALL_NUMBER)
		return 0;
	
	int nshells = CSolver::npoints-1;
	//printf("Reaady calc cont: %ld",CSolver::npoints);
	double path_total = 0.;

	int nump = 0;


	for(int ish=nshells;ish>=0;--ish)
	{
		int ilayer = CSolver::paths[ish].layer;
		int sector = CSolver::paths[ish].sector;

		path_total += CSolver::paths[ish].path;

		//if(!App::isStatMode)
		{
			double Z_H = 0.;
			double ems_O = 0.;

			if(sector > -1 && ilayer > -1 && ilayer < Abund::secRows[sector])
			{
				//printf("Layer: %d\n");
				Z_H = CSolver::paths[ish].path*Abund::abundances[sector][ilayer][0];
				ems_O = CSolver::paths[ish].path*(CLine::emits[sector][ilayer][3] + 
				CLine::emits[sector][ilayer][4] +
				CLine::emits[sector][ilayer][5] +
				CLine::emits[sector][ilayer][10]);
			}

			double Z = 0.;

			for(int i=0;i<Abund::nElements;i++)
			{
				Z = 0.;
				// printf("AbR: %d; %d\n", Abund::secRows[sector], ilayer);
				if(sector > -1 && ilayer > -1 && ilayer < Abund::secRows[sector])
				{
					
					Z = Abund::abundances[sector][ilayer][i]/(Abund::abundances[sector][ilayer][0]+1.e-50);
					Abund::abmass[i] += Z*Z_H;
					Abund::abemso[i] += Z*ems_O;
					// printf("Z=%le\n",Z);
				}
				
			}
			Abund::mass += Z_H;
			Abund::emso += ems_O;
			
		}
		
		//if(App::isStatMode || App::punchStatistics)
		{
			Abund::statistics[CLine::iStat][0] = path_total;
			CLine::statistics[CLine::iStat][0] = path_total;
			GrainTemp::statistics[CLine::iStat][0] = path_total;	
			Physics::statistics[CLine::iStat][0] = path_total;	
			CContinuum::statistics[CLine::iStat][0] = path_total;	
			
			for(int i=0;i<Abund::nElements;i++)
			{
				double Z = 0.;

				if(sector > -1 && ilayer > -1)
				{

					
					Z = Abund::abundances[sector][ilayer][i];
					Abund::statistics[CLine::iStat][i+1] = Z;
				}

				if(i==0 && Z > pow(10.0, -2.0) && App::iApp == 5)
				{
					printf("App%d peak: %le at %d;%d path=%Le\n",App::iApp,Z,sector,ilayer,CSolver::paths[ish].path);
					printf("iRay: %d; point: %d of %d\n", CIntegration::nRay, ish, CSolver::npoints);
					printf("Struct data: \n");
					printf("%d: Sector: %d; Layer: %d; P=%Le\n", ish, CSolver::paths[ish].sector, CSolver::paths[ish].layer,CSolver::paths[ish].path);
				}
				
			}
			

			for(int i=0;i<CLine::linesCount;i++)
			{
				double Z = 0.;

				if(sector > -1 && ilayer > -1 && ilayer < CGeometry::nRadiuses[sector])
				{
					
					Z = CLine::emits[sector][ilayer][i];
					CLine::statistics[CLine::iStat][i+1] = Z;
				}
				
			}

			for(int i=0;i<CContinuum::nbands;i++)
			{
				double Z = 0.;

				if(sector > -1 && ilayer > -1 && ilayer < CGeometry::nRadiuses[sector])
				{
					
					Z = CContinuum::getBandEmissivity(i, sector, ilayer);
					/*if(i == 21) {
						printf("%d %le Continuum at %d %d %d = %le\n", CLine::iStat, CContinuum::statistics[CLine::iStat][0], sector, ilayer, i+1, Z);	
					}*/
					CContinuum::statistics[CLine::iStat][i+1] = Z;
				}
				
			}
			
			double Z_H = 0.;
			if(sector > -1 && ilayer > -1)
			{
				Z_H = CSolver::paths[ish].path*Abund::abundances[sector][ilayer][0];
			}
			
			for(int i=0;i<GrainTemp::nBins;i++)
			{
				double Z = 0.;

				if(sector > -1 && ilayer > -1)
				{
					Z = GrainTemp::gridTemps[sector][ilayer][i];
					/*if(i == GrainTemp::nBins-1)
					{
						printf("%d (%d) GT: %le\n", CLine::iStat, i, Z);
					}*/
					
					GrainTemp::statistics[CLine::iStat][i+1] = Z;
					GrainTemp::massWeighted[i] += Z*Z_H;
					//printf("S: %d/19, L: %d, B: %d (%d) %le %le\n", sector, ilayer, i, GrainTemp::nBins, Z_H, Z*Z_H);
					
				}
				
			}
			GrainTemp::massTotal += Z_H;
			if(App::CalcOverviews)
			{
				if(sector > -1 && ilayer > -1)
				{
					//printf("S: %d/19, L: %d, B: %d (%d)\n", sector, ilayer, i, GrainTemp::nBins);
					Physics::statistics[CLine::iStat][1] = Physics::Te[sector][ilayer];
					Physics::statistics[CLine::iStat][2] = Physics::Ne[sector][ilayer];
					Physics::statistics[CLine::iStat][3] = sector;
					Physics::statistics[CLine::iStat][4] = ilayer;
					
				
				}
				
			}
			
			CLine::iStat++;

		}

		nump++;

	}

	return 1;

}