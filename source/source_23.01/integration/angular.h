#pragma once
#include "AngleStep.h"
#include "matrix.h"
#include "../app.h"
#include "../lines.h"
#include <algorithm>
#include "../output/debugger.h"
#include "../output/AnglesFileHandler.h"
#include <vector>
#include <chrono>

class Angular {
    public:
	double phi, theta, dphi, dtheta, nPhots, STot;
	AngleStep *angles[1000000];
	int AngleCount;
	double Vcenter, Scenter, SMcenter;
	AnglesFileHandler* anglesFileHandler;
	std::vector<AngleStep> angleStepsToSerialize;
	bool angleStepsLoadedFromFile = false;
	bool angleStepsFileMode = true;
	
	//params are actual aperture constraints here
	Angular(double cphi, double ctheta, double cdphi, double cdtheta, int nSteps, bool doIterationOverSource)
	{
		CDebugger::debug("Init angular obj %le %le; [%le;%le]\n", cphi, ctheta, cdphi, cdtheta);
		phi = cphi;
		theta = ctheta;
		dphi = cdphi;
		dtheta = cdtheta;
		AngleCount = 0;
		long double phiW = dphi/nSteps;
		long double thetaW = dtheta/nSteps;
		
		if(angleStepsFileMode)
		{
			anglesFileHandler = new AnglesFileHandler("r");
			angleStepsLoadedFromFile = TryGetAnglesFromFile();

			if(!angleStepsLoadedFromFile)
			{
				for(int ip = 0; ip < nSteps; ip++)
				{
					for(int it = 0; it < nSteps; it++)
					{
						CDebugger::debug("inserting points: %d\n", AngleCount);
						long double aphi = cphi + (ip-(nSteps-1)/2)*phiW;
						long double atheta = ctheta + (it-(nSteps-1)/2)*thetaW;
						insertPoint(aphi, atheta, phiW, thetaW);
					}
				}
			}
		}
		if(false)//App::usePredictiveMode)
		{
			//Run matrixes in predictive mode
			CDebugger::debug("Predictive\n");
			int nDepth = 0;
			while(predictiveIteration(doIterationOverSource) && nDepth < 3)
				nDepth++;
			sortByFi();
		}
		delete(anglesFileHandler);
	}

	bool sortByFi()
	{
		for(int i = 0; i < AngleCount; i++)
		{
			for(int j = 0; j < AngleCount-1; j++)
			{
				if(angles[j]->Fi > angles[j+1]->Fi)
				{
					AngleStep* buffer = angles[j];
					angles[j] = angles[j+1];
					angles[j+1] = buffer;
				}
			}
		}
		return true;
	}

	bool predictiveIteration(bool doIterationOverSource)
	{
		CDebugger::debug("start\n");
		for(int i = AngleCount - 1; i >= 0; i--)
		{
			if(angles[i]->isPredicted)
				continue;

			double currentPhi = angles[i]->phi;
			double currentTheta = angles[i]->theta;
			double currentDPhi = angles[i]->dphi;
			double currentDTheta = angles[i]->dtheta;
			CDebugger::debug(" -----===--- %d %le;%le\n",i,currentPhi, currentTheta);
			//now should setup matrix
			CMatrix::setup(currentPhi, currentTheta, currentDPhi, currentDTheta);
			CMatrix::run(false, doIterationOverSource);
			angles[i]->Fi = CMatrix::FFac/CMatrix::dS;
			angles[i]->isPredicted = true;
			CMatrix::freeMem();
		
		}
		sortByFi(); 
		double average = 0.;
		for(int ia=0;ia < AngleCount; ia++)
		{
			CDebugger::debug("Sorted: %le %le %le\n", angles[ia]->phi, angles[ia]->theta, angles[ia]->Fi);
			average += pow(angles[ia]->Fi,2.0);
		}
		//Get average
		average = sqrt(average/AngleCount);
		int itA = AngleCount - 1;
		while(itA > AngleCount/2 && (angles[itA]->Fi/average > AngleCount * 1./5))
		{
			//printf("Av: %le; Fi: %le\n",average,angles[itA]->Fi);
			itA--;
		}
		for(int ia = itA + 1; ia < AngleCount; ia++)
		{
			double currentPhi = angles[ia]->phi;
			double currentTheta = angles[ia]->theta;
			double currentDPhi = angles[ia]->dphi;
			double currentDTheta = angles[ia]->dtheta;
			removePoint(ia);
			int nSteps = 3;
			long double phiW = currentDPhi/nSteps;
			long double thetaW = currentDTheta/nSteps;
		
			for(int ip = 0; ip < nSteps; ip++)
			{
				for(int it = 0; it < nSteps; it++)
				{
					long double aphi = currentPhi + (ip-(nSteps-1)/2)*phiW;
					long double atheta = currentTheta + (it-(nSteps-1)/2)*thetaW;
					insertPoint(aphi, atheta, phiW, thetaW);
				}
			}
			CDebugger::log("Finished predictive iterations");
			return true;
		}
		return false;
	}

	void insertPoint(double cphi, double ctheta, double cdphi, double cdtheta)
	{
		angles[AngleCount] = new AngleStep(cphi,ctheta,cdphi,cdtheta);
		AngleCount++;
	}

	void removeLastPoint()
	{
		delete angles[AngleCount-1];
		AngleCount--;
	}
	
	void removePoint(int ip)
	{
		delete angles[ip];
		for(int i=ip;i<AngleCount-1;i++)
		    angles[i] = angles[i+1];
		
		angles[AngleCount-1] = NULL;
		AngleCount--;
	}

	void iterate(bool doIterationOverSource)
	{
		int acceptedCnt = 0;
		nPhots = 0.;
		STot = 0.;
		Vcenter = 0.;
		Scenter = 0.;
		SMcenter = 0.;
		CDebugger::log("Starting iterations");
		
		//int stop_after = 7000;

		/*if(angleStepsFileMode && !angleStepsLoadedFromFile)
		{
			anglesFileHandler = new AnglesFileHandler("a+");
		}*/
		
		auto start = std::chrono::high_resolution_clock::now();

		while(AngleCount > 0)
		{

			CDebugger::debug("AngleCount: %d (%d) ST: %d; nPhots: %le\n",AngleCount, acceptedCnt, CLine::iStat, nPhots);
			if(angles[AngleCount-1] == nullptr)
				CDebugger::error("NULL BITCH");
			double currentPhi = angles[AngleCount-1]->phi;
			double currentTheta = angles[AngleCount-1]->theta;
			double currentDPhi = angles[AngleCount-1]->dphi;
			double currentDTheta = angles[AngleCount-1]->dtheta;
			CDebugger::debug("Last point removed");
			//now should setup matrix
			CMatrix::setup(currentPhi, currentTheta, currentDPhi, currentDTheta);
			bool bWithStat = false;
			if(fabs(phi - currentPhi) < currentDPhi && fabs(theta - currentTheta) < currentDTheta)
				bWithStat = true;

			if(fabs(phi + dphi/2 - currentPhi) < currentDPhi &&
				fabs(theta + dtheta/2 - currentTheta) < currentDTheta)
				bWithStat = true;
			if(fabs(phi - dphi/2 - currentPhi) < currentDPhi &&
				fabs(theta - dtheta/2 - currentTheta) < currentDTheta)
				bWithStat = true;

			if(fabs(phi + dphi/2 - currentPhi) < currentDPhi &&
				fabs(theta - dtheta/2 - currentTheta) < currentDTheta)
				bWithStat = true;
			if(fabs(phi - dphi/2 - currentPhi) < currentDPhi &&
				fabs(theta + dtheta/2 - currentTheta) < currentDTheta)
				bWithStat = true;

			CDebugger::debug("Running matrix");
			CMatrix::run(bWithStat, doIterationOverSource);
			CDebugger::debug("runned\n");
			bool bPassed = CMatrix::getDelta(nPhots);
			if(bPassed)
			{
				//add everything to total fluxes
				CMatrix::addFluxes(nPhots);
				STot += CMatrix::dS;
				acceptedCnt++;
				if(CMatrix::Vcenter > 1.e-50)
				{
					Vcenter += CMatrix::Vcenter;
					Scenter += CMatrix::Scenter;
					SMcenter += CMatrix::dS*App::distance*App::distance;
				}
				//if(!angleStepsLoadedFromFile)
				//	angleStepsToSerialize.push_back(*angles[AngleCount-1]);
				if(angleStepsFileMode && !angleStepsLoadedFromFile)
				{
					//CDebugger::log("WRITING ANGLE TO FILE");
					angleStepsToSerialize.push_back(*angles[AngleCount-1]);
				}
				//--stop_after;
			}
			else
			{
				for(int ip = 0; ip < 9; ip++)
				{
					insertPoint(CMatrix::rays[ip]->phi,
						CMatrix::rays[ip]->theta,
						CMatrix::rays[ip]->dphi,
						CMatrix::rays[ip]->dtheta
					);
				}
			}

			removeLastPoint();
			CMatrix::freeMem();


			/*if(stop_after <= 0)
			{
				CDebugger::warn("FORCEFULLY STOPPING ITERATIONS");
				break;
			}*/

		}
		
		

		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
		CDebugger::log("TIME PASSED FOR ITERATIONS: %d s.\n", duration.count());
		
		if(angleStepsFileMode && !angleStepsLoadedFromFile)
		{
			CDebugger::log("WRITING ANGLES TO FILE");
			SerializeAngleSteps(angleStepsToSerialize);
		}
		exit(1);
	}

	bool TryGetAnglesFromFile()
	{
		AngleCount = anglesFileHandler->TryGetAnglesFromFile(angles);

		if(AngleCount == 0)
		{
			return false;
		}

		return true;
	}

	void SerializeAngleSteps(std::vector<AngleStep> angleSteps)
	{
		anglesFileHandler = new AnglesFileHandler();
		CDebugger::log("NUMBER OF ANGLE STEPS TO BE WRITTEN IS %d", angleSteps.size());
		auto success = anglesFileHandler->TryWriteAnglesToFile(angleSteps);
		if(!success)
		{
			CDebugger::error("Could not write angle steps to file");
			return;
		}
		CDebugger::log("Angle steps successfully written to file");
		delete(anglesFileHandler);
	}

	void SerializeAngleStep(AngleStep *angleStep)
	{
		auto success = anglesFileHandler->TryWriteAngleToFile(angleStep);
		if(!success)
		{
			CDebugger::error("Could not write angle step to file");
			return;
		}
		CDebugger::log("Angle step successfully written to file");
	}
};