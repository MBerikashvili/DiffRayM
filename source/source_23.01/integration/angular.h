#pragma once
#define _USE_MATH_DEFINES
#include "AngleStep.h"
#include "matrix.h"
#include "../app.h"
#include "../lines.h"
#include <algorithm>
#include "../output/debugger.h"
#include "../output/AnglesFileHandler.h"
#include <vector>
#include <chrono>
#include <iomanip>
#include <cmath>


class Angular {
    public:
	double phi, theta, dphi, dtheta, nPhots, STot;
	AngleStep *angles[1000000];
	int angleCount;
	double Vcenter, Scenter, SMcenter;
	AnglesFileHandler* anglesFileHandler;
	bool angleStepsLoadedFromFile = false;
	bool angleStepsFileMode;

	long iterationCount = 0;
	
	//params are actual aperture constraints here
	Angular(double cphi, double ctheta, double cdphi, double cdtheta, int nSteps, bool doIterationOverSource, bool acceletratedMode = false)
	{
		CDebugger::debug("Init angular obj %le %le; [%le;%le]\n", cphi, ctheta, cdphi, cdtheta);
		phi = cphi;
		theta = ctheta;
		dphi = cdphi;
		dtheta = cdtheta;
		angleCount = 0;
		iterationCount = 0;
		long double phiW = dphi/nSteps;
		long double thetaW = dtheta/nSteps;
		angleStepsFileMode = acceletratedMode;
		
		if(angleStepsFileMode)
		{
			anglesFileHandler = new AnglesFileHandler();
			angleStepsLoadedFromFile = TryGetAnglesFromFile();
		}

		if(!angleStepsLoadedFromFile)
		{
			for(int ip = 0; ip < nSteps; ip++)
			{
				for(int it = 0; it < nSteps; it++)
				{
					CDebugger::debug("inserting points: %d\n", angleCount);
					long double aphi = cphi + (ip-(nSteps-1)/2)*phiW;
					long double atheta = ctheta + (it-(nSteps-1)/2)*thetaW;
					insertPoint(aphi, atheta, phiW, thetaW);
				}
			}
		}
		
		if(!angleStepsFileMode && App::usePredictiveMode)
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
		for(int i = 0; i < angleCount; i++)
		{
			for(int j = 0; j < angleCount-1; j++)
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
		for(int i = angleCount - 1; i >= 0; i--)
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
		for(int ia=0;ia < angleCount; ia++)
		{
			CDebugger::debug("Sorted: %le %le %le\n", angles[ia]->phi, angles[ia]->theta, angles[ia]->Fi);
			average += pow(angles[ia]->Fi,2.0);
		}
		//Get average
		average = sqrt(average/angleCount);
		int itA = angleCount - 1;
		while(itA > angleCount/2 && (angles[itA]->Fi/average > angleCount * 1./5))
		{
			//printf("Av: %le; Fi: %le\n",average,angles[itA]->Fi);
			itA--;
		}
		for(int ia = itA + 1; ia < angleCount; ia++)
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
		angles[angleCount] = new AngleStep(cphi,ctheta,cdphi,cdtheta);
		angleCount++;
	}

	void removeLastPoint()
	{
		delete angles[angleCount-1];
		angleCount--;
	}
	
	void removePoint(int ip)
	{
		delete angles[ip];
		for(int i=ip;i<angleCount-1;i++)
		    angles[i] = angles[i+1];
		
		angles[angleCount-1] = NULL;
		angleCount--;
	}

	void iterate(bool doIterationOverSource)
	{
		int acceptedCnt = 0;
		nPhots = 0.;
		STot = 0.;
		Vcenter = 0.;
		Scenter = 0.;
		SMcenter = 0.;
		std::vector<AngleStep> angleStepsToSerialize;
		double totalBodyAngle = 0;
		double totalBodyAngleSimple = 0;
		int numberOfAnglesProcessed = 0;
		CDebugger::log("Starting iterations");
		
		auto start = std::chrono::high_resolution_clock::now();

		while(angleCount > 0)
		{
			iterationCount++;
			CDebugger::debug("angleCount: %d (%d) ST: %d; nPhots: %le\n",angleCount, acceptedCnt, CLine::iStat, nPhots);
			if(angles[angleCount-1] == nullptr)
				CDebugger::error("LAST ANGLESTEP IS NULL");
			double currentPhi = angles[angleCount-1]->phi;
			double currentTheta = angles[angleCount-1]->theta;
			double currentDPhi = angles[angleCount-1]->dphi;
			double currentDTheta = angles[angleCount-1]->dtheta;

			removeLastPoint();
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
				if(angleStepsFileMode)// && !angleStepsLoadedFromFile)
				{
					auto tmpAS = new AngleStep(currentPhi, currentTheta, currentDPhi, currentDTheta);
					angleStepsToSerialize.push_back(*tmpAS);
				}
				totalBodyAngleSimple += currentDPhi * currentDTheta * std::sin(abs(currentTheta));
				totalBodyAngle += currentDPhi * (std::cos(abs(currentTheta)) - std::cos(abs(currentTheta) + currentDTheta));
				numberOfAnglesProcessed++;
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
			CMatrix::freeMem();


			/*if(stop_after <= 0)
			{
				CDebugger::warn("FORCEFULLY STOPPING ITERATIONS");
				break;
			}*/

		}

		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
		std::time_t time = std::chrono::system_clock::to_time_t(end);

		if(angleStepsFileMode)
		{
			CDebugger::log("Mode: accelerated");
			CDebugger::writeToFile(time, "Mode: accelerated");
		}
		else
		{
			CDebugger::log("Mode: slow\n");
			CDebugger::writeToFile(time, "Mode: slow\n");
		}

		CDebugger::log("NUMBER OF PROCESSED ANGLES: %d", numberOfAnglesProcessed);
		CDebugger::writeToFile(time, "NUMBER OF PROCESSED ANGLES: %d", numberOfAnglesProcessed);

		CDebugger::log("TIME PASSED FOR ITERATIONS: %d s.\n", duration.count());
		CDebugger::writeToFile(time, "TIME PASSED FOR ITERATIONS: %d s.\n", duration.count());

		CDebugger::log("TIME FOR ONE ANGLE: %le", float(duration.count())/float(numberOfAnglesProcessed));
		CDebugger::writeToFile(time, "TIME FOR ONE ANGLE: %le", float(duration.count())/float(numberOfAnglesProcessed));

		CDebugger::log("Number of iterations performed: %ld", iterationCount);
		CDebugger::writeToFile(time, "Number of iterations performed: %ld\n", iterationCount);

		CDebugger::log("Total body angle of the object = %le *pi radians.", totalBodyAngleSimple/M_PI);
		CDebugger::writeToFile(time, "Total body angle of the object = %le *pi radians.", totalBodyAngleSimple/M_PI);
		
		if(angleStepsFileMode)// && !angleStepsLoadedFromFile)
		{
			CDebugger::log("WRITING ANGLES TO FILE");
			SerializeAngleSteps(angleStepsToSerialize);
		}
	}


	bool TryGetAnglesFromFile()
	{
		angleCount = anglesFileHandler->TryGetAnglesFromFile(angles);

		if(angleCount == 0)
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