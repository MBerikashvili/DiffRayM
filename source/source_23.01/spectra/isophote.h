#pragma once
#include "../app.h"
#include "../continuum.h"
#include "../integration/angular.h"
#include "../const.h"
#include "../mathutl.h"

const int RESOLUTION = 81;

const int ISOPHOTE_CONT = 0;
const int ISOPHOTE_LINE = 1;
const int ISOPHOTE_RATIO = 2;

struct IsophoteAngleStep: AngleStep {
	double FiBase;
	IsophoteAngleStep(double cphi, double ctheta, double cdphi, double cdtheta):AngleStep(cphi, ctheta, cdphi, cdtheta)
	{
		FiBase = SMALL_NUMBER;
	}
};

class Isophote {
    public:
	//double *emits[RESOLUTION, RESOLUTION];
	//double *opacs[RESOLUTION, RESOLUTION];
	double contCentral, contWidth;
	int cellLeft, cellRight, cellCenter;
	IsophoteAngleStep *points[RESOLUTION][RESOLUTION];
	bool settedUp;
	int isophoteType, lineId, lineIdTo;
	char chLin[14];
	double dSMesh;
	char chLinTo[14];

    Isophote(int type, double contCentralA, double contWidthA)
    {
	cellLeft = 0;
	cellRight = 0;
	cellCenter = 0;
	contCentral = contCentralA;
	contWidth = contWidthA;
	settedUp = false;
	isophoteType = type;
    }

    ~Isophote()
    {
	for(int i=0;i<RESOLUTION;i++)
	{
		for(int j=0;j<RESOLUTION;j++)
		{
			delete points[i][j];
		}
	}
    }

    Isophote* setLine(char chlab[14])
    {
	if(isophoteType == ISOPHOTE_LINE || isophoteType == ISOPHOTE_RATIO)
	{
	    sprintf(chLin, "%s", chlab);
	    
	}
	return this;
    }

    Isophote* setTo(char chlab[14])
    {
	if(isophoteType == ISOPHOTE_RATIO)
	{
	    sprintf(chLinTo, "%s", chlab);
	    
	}
	return this;
    }


    double min()
    {
	double val = BIG_NUMBER;
	for(int i=0;i<RESOLUTION;i++)
	{
		for(int j=0;j<RESOLUTION;j++)
		{
			double comparii = points[i][j]->Fi;
			if(isophoteType == ISOPHOTE_RATIO)
			{
				comparii = comparii/(points[i][j]->FiBase+SMALL_NUMBER);
				
			}
			if(comparii < val && comparii > SMALL_NUMBER)
				val = comparii;
		}
	}
	return val;
    }

    double max()
    {
	double val = SMALL_NUMBER;
	for(int i=0;i<RESOLUTION;i++)
	{
		for(int j=0;j<RESOLUTION;j++)
		{
			double comparii = points[i][j]->Fi;
			if(isophoteType == ISOPHOTE_RATIO && comparii < BIG_NUMBER)
			{
				comparii = comparii/(points[i][j]->FiBase+SMALL_NUMBER);
			}
			if(comparii > val)
				val = comparii;
			
		}
	}
	return val;
    }

    void initAngles()
    {
	if(settedUp)
	{
		for(int i=0;i<RESOLUTION;i++)
		{
			for(int j=0;j<RESOLUTION;j++)
			{
				delete points[i][j];
			}
		}
	}
	for(int i=0;i<RESOLUTION;i++)
	{
		for(int j=0;j<RESOLUTION;j++)
		{
			double phi = App::rayToObj.angle.y + App::AppDPhi + (floorAbs((1-RESOLUTION)/2.) + i)*App::phi_width/RESOLUTION;
			double theta = App::rayToObj.angle.z + App::AppDTheta + (floorAbs((1-RESOLUTION)/2.) + j)*App::theta_width/RESOLUTION;
			points[i][j] = new IsophoteAngleStep(phi, theta, App::phi_width/RESOLUTION, App::theta_width/RESOLUTION);
		}
	}
	settedUp = true;
	dSMesh = App::phi_width * App::theta_width / (RESOLUTION * RESOLUTION);
    }
    
    void initSpectra()
    {
	if(isophoteType == ISOPHOTE_CONT)
	{
		double enLeft = contCentral - contWidth/2.0;
		double enRight = contCentral + contWidth/2.0;
		if(CContinuum::cellCount > 1)
		{
			int i = 0;
			while(i < CContinuum::cellCount && CContinuum::anu[i] < enRight)
			{
				if(CContinuum::anu[i] > enLeft && cellLeft == 0)
					cellLeft = i;

				if(CContinuum::anu[i] > contCentral && cellCenter == 0)
					cellCenter = i;
				i++;
			}
			cellRight = i;
		}
	}
	else
	if(isophoteType == ISOPHOTE_LINE || isophoteType == ISOPHOTE_RATIO)
	{
		for(int i=0; i<CLine::linesCount;i++)
		{
			char *chALab;
			chALab = CLine::linesCapDB[CLine::lineIds[i]];
			printf("[%d %d] %s VS %s\n", i, CLine::lineIds[i], chALab, chLin);
			if(strcmp(chALab,chLin)==0)
			{
				lineId = i;
				printf("FOUND AT %d\n",i);

				if(isophoteType == ISOPHOTE_RATIO)
				{
					for(int j=0; j<CLine::linesCount;j++)
					{
						char *chALabTo;
						chALabTo = CLine::linesCapDB[CLine::lineIds[j]];
						if(strcmp(chALabTo,chLinTo)==0)
						{
							lineIdTo = j;
							printf("FOUND AT %d\n",j);
							return;
						}
					}
				}
				else
				{
					return;
				}
			}
		}
		printf("Line %s not found\n",chLin);
		
		exit(1);
	}
    }

    //get square that intersects specific angle
    double addFrom(double phi, double theta, double dphi, double dtheta, double *fluxes)
    {
	bool bFound = false;
	for(int iPhi = 0; iPhi < RESOLUTION; iPhi++)
	{
		for(int iTheta = 0; iTheta < RESOLUTION; iTheta++)
		{
			//printf("Theta: %le\n", theta);
			//exit(1);
			double dS = points[iPhi][iTheta]->intersectionSquare(phi, theta, dphi, dtheta);
			if(dS <= 0)
				continue;

			bFound = true;
			//SINCE WE INTERSECT SOMETHING - LETS ADD
			if(isophoteType == ISOPHOTE_CONT)
			{
				for(int ic = cellLeft; ic < cellRight; ic++)
				{
					points[iPhi][iTheta]->Fi += fluxes[ic]*dS/dSMesh;
				}
			}
			else
			if(isophoteType == ISOPHOTE_LINE)
			{
				points[iPhi][iTheta]->Fi += fluxes[lineId]*dS/dSMesh;
			}
			else
			if(isophoteType == ISOPHOTE_RATIO)
			{
				points[iPhi][iTheta]->Fi += fluxes[lineId]*dS/dSMesh;
				points[iPhi][iTheta]->FiBase += fluxes[lineIdTo]*dS/dSMesh;
				//printf("Added to isophote at %le %le; Q=%le; (from %le;%le) [%d;%d]\n", points[iPhi][iTheta]->phi, points[iPhi][iTheta]->theta, points[iPhi][iTheta]->Fi, phi, theta, iPhi, iTheta);
				//exit(1);
			}
		}
	}
	if(!bFound)
	{
		printf("NotFound from %le %le\n",phi, theta);
		
		printf("Angle: %le; %le [%le X %le]\n", 
		    points[80][48]->phi,
		    points[80][48]->theta,
		    points[80][48]->dphi,
		    points[80][48]->dtheta
		);
		exit(1);
	}
	return 1.;
    }
    
};

class CIsophotes {
    public:
	static int nIsophotes;
	static Isophote *isophotes[25];

	static Isophote* addIsophote(int type, double enC, double enW);

	static int initAll();

	static int resetApp();

	static int addFlux(double phi, double theta, double dphi, double dtheta, double *fluxes, double *lines);
};