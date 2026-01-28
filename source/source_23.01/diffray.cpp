#include "basics.h"
#include "integration.h"
#include "app.h"
#include "continuum.h"
#include "lines.h"
#include "iterator.h"
#include "diffray.h"
#include "geometry.h"
#include "abund.h"
#include "grain_temp.h"
#include "physics.h"
#include "output/output_handler.h"
#include <sys/stat.h>
#include "output/debugger.h"

int CDiffRay::nPoints = 0;
double CDiffRay::pointsArr[4000];

int compareRadii (const void * pa, const void * pb)
{
  const double *a = (double *) pa;
  const double *b = (double *) pb;
  if(a[0] > b[0])
  	return -1;
  
  if(a[0] < b[0])
  	return 1;
  
  return 0;
}

int CDiffRay::initializePoints() 
{
	CDiffRay::nPoints = 0;
	if(!App::pointsSource) 
	{
		return 0;
	}

	FILE *FPNTS;
	char fname[255];
	sprintf(fname, "%s/%s",App::input_dir,App::pointsFile);
	FPNTS = fopen(fname, "r");
	if(FPNTS == NULL) 
	{
		printf("Unable to open file with points %s",fname);
		exit(1);
	}
	int i=0;
	CDebugger::log("Started reading points: %s", fname);
	while(!feof(FPNTS)) 
	{
		char chline[1000];
		fgets(chline, 1000, FPNTS);
		if(i < App::nSkipPoints) 
		{
			i++;
			continue;
		}
		double distance;
		sscanf(chline, "%le", &distance);
		// should not push points here if already processed
		if(distance > 0) {
			CDiffRay::pointsArr[CDiffRay::nPoints] = distance;
			CDiffRay::nPoints++;
		}
		CDebugger::log("%d %le\n",CDiffRay::nPoints,CDiffRay::pointsArr[CDiffRay::nPoints-1]);
		
	}
	fclose(FPNTS);
	return 1;
}

int CDiffRay::getPointsProcessed()
{
	if(!App::pointsSource)
	{
		return 0;
	}
	// get file to flush results
	char cfilename[255];
	sprintf(cfilename, "%s/%s", App::output_dir, App::fluxesOutput);
	FILE *CUM_CONT_FILE;
	
	CUM_CONT_FILE = fopen(cfilename, "r");

	int i=0;
	CDebugger::log("Started calculating points to retain progress: %s", cfilename);
	
	if(!CUM_CONT_FILE) {
		return 0;
	}
	
	while(!feof(CUM_CONT_FILE)) 
	{
		char chline[100000];
		fgets(chline, 100000, CUM_CONT_FILE);
		i++;
	}
	i--;
	fclose(CUM_CONT_FILE);
	FILE *FBREAKHIST;
	char fbreakname[255];
	sprintf(fbreakname, "%s/breakouts.dat", App::output_dir);
	FBREAKHIST = fopen(fbreakname, "a+");
	fprintf(FBREAKHIST, "%d %s\n", i, cfilename);
	fclose(FBREAKHIST);
	return i;
}

int CDiffRay::launch() 
{
	if(!App::pointsSource) 
	{
		App::init();
		return CDiffRay::runDiffRay(false, 0);
	}
	int pointsProcessed = CDiffRay::getPointsProcessed();
	//exit(1);
	CDiffRay::initializePoints();
	if(pointsProcessed < 1) {
		pointsProcessed = 1;
	}
	for(int i=pointsProcessed-1;i<CDiffRay::nPoints;i++) 
	{
		App::distance = CDiffRay::pointsArr[i];
		App::init();
		//if(i > 4 && i <= 649)
		{
			CDiffRay::runDiffRay(true, i);
		}
	}
	return 1;
}

int CDiffRay::runDiffRay(bool usePoints, int nPoint)
{
	int mode = App::integration_mode;
	CIteration::initIteration(mode);

	App::isStatMode = false;
	printf("Starting DiffRay main app\n");


	// while(/*CIteration::nIteration <= App::maxiterations*/)
	{
		Abund::refreshStatistics();
		CLine::refreshStatistics();
		GrainTemp::refreshStatistics();
		Physics::refreshStatistics();
		CContinuum::refreshStatistics();
		
		CDebugger::debug("Refreshed statistics");
		printf("---\n");
		CIteration::doIteration();

		CDebugger::debug("Printing output");
		

		if(App::AppMode != 0)
		{
			CIntegration::lumfactor *= (1/(4*M_PI));
		}

		if(usePoints) 
		{
			CDebugger::debug("nPoint: %d\n", nPoint);
			char cfilename[255];
			sprintf(cfilename, "%s/%s", App::output_dir, App::fluxesOutput);
			FILE *CUM_CONT_FILE;
			if(nPoint < 1) 
			{
				CUM_CONT_FILE = fopen(cfilename, "w+");
			}
			else
			{
				CUM_CONT_FILE = fopen(cfilename, "a+");
			}
			
			if(CUM_CONT_FILE != NULL)
			{
				if(nPoint < 1) {
					fprintf(CUM_CONT_FILE, "%le", App::distance);
					for(int il=0;il<CContinuum::cellCount;il++)
					{
						fprintf(CUM_CONT_FILE, "\t%le", CContinuum::anu[il]);
					}
					fprintf(CUM_CONT_FILE, "\n" );
				}
				fprintf(CUM_CONT_FILE, "%le", App::distance);
				for(int il=0;il<CContinuum::cellCount;il++)
				{
					fprintf(CUM_CONT_FILE, "\t%le", CIntegration::flux_continua[il]);
				}
				fprintf(CUM_CONT_FILE, "\n" );
				fclose(CUM_CONT_FILE);
			}

		}


		if(App::CalcCont)
		{
		//!!BM UPD:
		//	char fname[255];
		//	sprintf(fname,"contarr_Sector%d.dat",(App::onlySectorNo+1));
		//	OutputHandler::contarr(fname);
		//	OutputHandler::spectra();
		//!!END OF BM UPD:
			OutputHandler::contarr();
			OutputHandler::spectra();
			
		}
		if(App::CalcLines)
		{
			OutputHandler::lifr();
			OutputHandler::llum();
			OutputHandler::linesarr();
		}
		OutputHandler::isophotes();
			
		if(App::CalcLines)
		{
			char fname[255];
			sprintf(fname,"%s/abmass.txt",App::output_dir);
			FILE *FP;

			CDebugger::debug("ammass file %s\n", fname);
			FP = fopen(fname,"w+");

			for(int i=0;i<Abund::nElements;i++){
				fprintf(FP, "%s\t\t",Abund::elements[i]);
			}

			fprintf(FP, "\n");

			
			for(int i=0;i<Abund::nElements;i++){
				fprintf(FP, "%lf\t\t",12+log10(Abund::abmass[i]/(Abund::mass+1.e-80)));
			}

			fclose(FP);	

			sprintf(fname,"%s/abemso.txt",App::output_dir);
			
			CDebugger::debug("abemso file %s\n", fname);
			FP = fopen(fname,"w+");

			for(int i=0;i<Abund::nElements;i++){
				fprintf(FP, "%s\t\t",Abund::elements[i]);
			}

			fprintf(FP, "\n");

			
			for(int i=0;i<Abund::nElements;i++){
				printf("CMP: %le\t%le\n", Abund::abemso[i], Abund::emso);
				fprintf(FP, "%lf\t\t",12+log10(Abund::abemso[i]/(Abund::emso + 1.e-80)));
			}

			fclose(FP);	
		}
		//if(App::CalcGrains)
		{
			FILE* GRAIN_FILE;
			char grain_fname[255];
			sprintf(grain_fname, "%s/grains_weighted.dat",App::output_dir);
			GRAIN_FILE = fopen(grain_fname, "w+");
			for(int i=0;i<GrainTemp::nBins;i++)
			{
				fprintf(GRAIN_FILE,"%lf\t",GrainTemp::massWeighted[i]/GrainTemp::massTotal);
			}
			fclose(GRAIN_FILE);
		}
		
		if(CContinuum::nbands > 0)
		{

			char fname[255];
			char fnameFmt[255];
			char fname_corr[255];
			char fnameFmt_corr[255];
			
			sprintf(fname,"%s/bands.txt",App::output_dir);
			sprintf(fnameFmt,"%s/bands_formatted.txt",App::output_dir);
			sprintf(fname_corr,"%s/bands_nocorr.txt",App::output_dir);
			sprintf(fnameFmt_corr,"%s/bands_formatted_nocorr.txt",App::output_dir);
			FILE *FP;
			FILE *bandFileFmt;
			FILE *FP_cor;
			FILE *bandFileFmt_cor;
				
			//printf("%s\n", fname);
			char strnm1[255];
			char strnm2[255];
			sprintf(strnm1,"mips24_app%d.dat",App::iApp);
			sprintf(strnm2,"mips70_app%d.dat",App::iApp);
			
			FP = fopen(fname,"w+");
			FP_cor = fopen(fname_corr,"w+");
			
			bandFileFmt = fopen(fnameFmt, "w+");
			bandFileFmt_cor = fopen(fnameFmt_corr, "w+");
			fprintf(FP, "Band\t\tFlux at observer\t\tFlux, Yan\t\tLuminosity\n");
			fprintf(FP_cor, "Band\t\tFlux at observer\t\tFlux, Yan\t\tLuminosity\n");
			for(int i = 0; i < CContinuum::nbands; i++)
			{
				fprintf(bandFileFmt, "%s\t", CContinuum::bands[i].label);
			}
			fprintf(bandFileFmt, "\n");
			for(int i = 0; i < CContinuum::nbands; i++)
			{
				fprintf(bandFileFmt_cor, "%s\t", CContinuum::bands[i].label);
			}
			fprintf(bandFileFmt_cor, "\n");
			for(int i = 0; i < CContinuum::nbands; i++)
			{
				IRBand cBand = CContinuum::bands[i];

				double bandInten = 0.;
				double Yan = 0.0;
				double YanNoCorr = 0.;
				double bandIntenNoCorr = 0.;

				int iT = cBand.nAnus-1;
				double trans = 1.0;

				char bandsFNm[255];
				char bandsDir[255];
				FILE *bandFile;
				if(App::printBands) 
				{
					sprintf(bandsDir, "%s/bands", App::output_dir);
					sprintf(bandsFNm, "%s/bands/%s.dat", App::output_dir, cBand.label);
					mkdir(bandsDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
					bandFile = fopen(bandsFNm, "w+");
					CDebugger::debug("ATTEMPT_BAND_FILE %ld, %ld\n", cBand.iNuLeft, cBand.iNuRight);
				}

				for(int iCl = cBand.iNuLeft; iCl < cBand.iNuRight; iCl++)
				{
					double cEn = CContinuum::anu[iCl];
					//trans = 1.0;
					while(iT >= 0 && cEn > 912./cBand.ennu[iT])
					{
						trans = cBand.transmitions[iT];
						iT--;
					}
					
					//printf("Trans[%d]: %le; %le > %le\n", iT, trans, cEn, 912.0/cBand.ennu[iT]);
					double x = ((CIntegration::flux_continua[iCl] + CIntegration::flux_transitions[iCl]) + CIntegration::flux_intrinsic[iCl])*2.179874099E-11/(1.e-23*3.e+8/(912*1.e-10));
					double y = ((CIntegration::flux_continua[iCl] + CIntegration::flux_transitions[iCl]) + CIntegration::flux_intrinsic[iCl])*2.179874099E-11*CContinuum::anu[iCl]/(1.e-23/(912*1.e-10));
					double xc = trans*((CIntegration::flux_continua[iCl] + CIntegration::flux_transitions[iCl]) + CIntegration::flux_intrinsic[iCl])*2.179874099E-11/(1.e-23*3.e+8/(912*1.e-10));
					double yc = trans*((CIntegration::flux_continua[iCl] + CIntegration::flux_transitions[iCl])*App::cov_fac + CIntegration::flux_intrinsic[iCl])*2.179874099E-11*CContinuum::anu[iCl]/(1.e-23/(912*1.e-10));
					
					bandInten += y;
					Yan += x;
					bandIntenNoCorr += yc;
					YanNoCorr += xc;
					if(App::printBands && bandFile) {
						fprintf(bandFile, "%le\t%le\t%le\t%le\t%le\n", cEn, x, y, CIntegration::flux_continua[iCl],CIntegration::flux_intrinsic[iCl]);
					}
				}


				//bandInten *= CIntegration::lumfactor;
				//Yan *= CIntegration::lumfactor;
				if(bandFile) {
					fclose(bandFile);
				}
				fprintf(bandFileFmt, "%le\t", Yan);
				fprintf(bandFileFmt_cor, "%le\t", YanNoCorr);
				fprintf(FP, "%s\t\t%le\t\t%le\t\t%lf\t\t%lf\n", cBand.label, bandInten, Yan, log10(1.e-50 + 4*M_PI*bandInten*pow(App::distance,2.0)),bandInten/(CIntegration::flux_lines[0]*CIntegration::lumfactor));
				fprintf(FP_cor, "%s\t\t%le\t\t%le\t\t%lf\t\t%lf\n", cBand.label, bandIntenNoCorr, YanNoCorr, log10(1.e-50 + 4*M_PI*bandIntenNoCorr*pow(App::distance,2.0)),bandIntenNoCorr/(CIntegration::flux_lines[0]*CIntegration::lumfactor));
			}

			fclose(FP);	
			fclose(bandFileFmt);
			fclose(FP_cor);	
			fclose(bandFileFmt_cor);
		}

		CIteration::nIteration++;	
	}

	
	


	CIteration::obtainStatistics();


	/* Halt! We should sort statistics first! 
	   We can use qsort for this
	*/
	CDebugger::debug("OS\n");
	qsort (Abund::statistics, 500000, sizeof Abund::statistics[0], compareRadii);
	CDebugger::debug("SORTEDA\n");
	qsort (CLine::statistics, 500000, sizeof CLine::statistics[0], compareRadii);
	CDebugger::debug("SORTEDL\n");
	qsort (GrainTemp::statistics, 500000, sizeof GrainTemp::statistics[0], compareRadii);
	CDebugger::debug("SORTEDL\n");
	qsort (Physics::statistics, 500000, sizeof Physics::statistics[0], compareRadii);
	CDebugger::debug("SORTEDP\n");
	qsort (CContinuum::statistics, 500000, sizeof CContinuum::statistics[0], compareRadii);
	CDebugger::debug("SORTEDB\n");
	int i = 0;

	FILE *AS;

	char fnamedas[255];
	sprintf(fnamedas,"%s/cont_distribution_saved.txt",App::output_dir);

	AS = fopen(fnamedas, "w+");

	for(int i=0;i<500000;i++) 
	{
		if(CContinuum::statistics[i][0] > 1.e-36) {
			fprintf(AS, "%le\t%le\t%le\n", CContinuum::statistics[i][0], CContinuum::statistics[i][18], CContinuum::statistics[i][22]);
		}		
	}

	fclose(AS);


	/*** Smoothing and averaging ***/
	for(int ir=0; ir<1; ir++)
	{
		for(int is=1;is<CLine::iStat-1; is++)
		{
			if(Abund::statistics[is][1] < (Abund::statistics[is-1][1] + Abund::statistics[is+1][1])/3)
			{
				for(int k=1;k<=Abund::nElements;k++)
				{
					Abund::statistics[is][k] = (Abund::statistics[is-1][k] + Abund::statistics[is][k] + Abund::statistics[is+1][k])/3;
				}
			}
		}
	}

	for(int ir=0; ir<1; ir++)
	{
		for(int is=1;is<CLine::iStat-1; is++)
		{
			if(CLine::statistics[is][1] < (CLine::statistics[is-1][1] + CLine::statistics[is+1][1])/3)
			{
				for(int k=1;k<=CLine::linesCount;k++)
				{
					CLine::statistics[is][k] = (CLine::statistics[is-1][k] + CLine::statistics[is][k] + CLine::statistics[is+1][k])/3;
				}
			}
		}
	}

	for(int ir=0; ir<1; ir++)
	{
		for(int is=1;is<CLine::iStat-1; is++)
		{
			if(GrainTemp::statistics[is][1] < (GrainTemp::statistics[is-1][1] + GrainTemp::statistics[is+1][1])/3)
			{
				for(int k=1;k<=GrainTemp::nBins+1;k++)
				{
					GrainTemp::statistics[is][k] = (GrainTemp::statistics[is-1][k] + GrainTemp::statistics[is][k] + GrainTemp::statistics[is+1][k])/3;
				}
			}
		}
	}
	
	/*
	for(int ir=0; ir<1; ir++)
	{
		for(int is=1;is<CLine::iStat-1; is++)
		{
			if(Physics::statistics[is][1] < (Physics::statistics[is-1][1] + Physics::statistics[is+1][1])/3)
			{
				for(int k=1;k<=3;k++)
				{
					Physics::statistics[is][k] = (Physics::statistics[is-1][k] + Physics::statistics[is][k] + Physics::statistics[is+1][k])/2;
				}
			}
		}
	}
	*/

	for(int ir=0; ir<1; ir++)
	{
		for(int is=1;is<CLine::iStat-1; is++)
		{
			if(CContinuum::statistics[is][1] < (CContinuum::statistics[is-1][1] + CContinuum::statistics[is+1][1])/3)
			{
				for(int k=1;k<=CContinuum::nbands;k++)
				{
					CContinuum::statistics[is][k] = (CContinuum::statistics[is-1][k] + CContinuum::statistics[is][k] + CContinuum::statistics[is+1][k])/3;
				}
			}
		}
	}

	int nLines = CLine::iStat;
	int nAbunds = CLine::iStat; 
	int nTemps = CLine::iStat; 
	int nPhys = CLine::iStat;

	int ik = 0;
	int ic = 0;
	int ikprev = 0;
	double av[100];
	for(int k=0;k<100;k++)
	{
		av[k] = 0.;
	}

	while(ic < CLine::iStat)
	{
		double dR = 0.01;

		for(int k=0;k<100;k++)
		{
			av[k] = 1.e-42;
		}
		int nElems = 0;

		while(dR < 1.e+19 && ik<CLine::iStat)
		{
			ik++;
			dR = fabs(Abund::statistics[ik][0] - Abund::statistics[ikprev][0]);
			
			for(int k=1;k<Abund::nElements;k++)
			{
				av[k] += Abund::statistics[ik][k];
			}
			nElems++;
			
		}

		ikprev = ik;

		//printf("av: %le; n=%d; LC=%d\n",av[1], nElems, CLine::iStat);

		for(int k=1;k<Abund::nElements;k++)
		{
			Abund::statistics[ic][k] = av[k]/nElems;
		}

		
		

		if(ik >= CLine::iStat)
		{
			Abund::statistics[ic][0] = Abund::statistics[CLine::iStat-1][0];
			nAbunds = ic+1;
			break;
		}
		else
		{
			Abund::statistics[ic][0] = Abund::statistics[ik][0];

		}

		ic++;

	}
	
	CDebugger::debug("Abunds selected: %d\n",nAbunds);
	i = 0;
	int abundIp = 0;
	char fnameda[255];
	sprintf(fnameda,"%s/abund_distribution.txt",App::output_dir);
	
	FILE *AbundF;

	AbundF = fopen(fnameda, "w+");
	while(i<nAbunds && i<500000)
	{
		if(Abund::statistics[i][1] < 1.e-35 /*|| CLine::statistics[i][0] >= 1.e+25*/)
		{
			i++;
			continue;
		}
		if(abundIp > 0)
		{
			int j = 0;
			while(j <= Abund::nElements && j < 100)
			{
				fprintf(AbundF, "%le\t", Abund::statistics[i][j]);
				j++;
			}
			fprintf(AbundF, "\n");
		}
		abundIp++;
		i++;
	}
	fclose(AbundF);


	ik = 0;
	ic = 0;
	ikprev = 0;
	
	for(int k=0;k<100;k++)
	{
		av[k] = 0.;
	}

	while(ic < CLine::iStat)
	{
		double dR = 0.01;

		for(int k=0;k<100;k++)
		{
			av[k] = 1.e-42;
		}
		int nElems = 0;

		while(dR < 1.e+19 && ik<CLine::iStat)
		{
			ik++;
			dR = fabs(CLine::statistics[ik][0] - CLine::statistics[ikprev][0]);
			
			for(int k=1;k<CLine::linesCount;k++)
			{
				av[k] += CLine::statistics[ik][k];
			}
			nElems++;
			
		}

		ikprev = ik;

		//printf("av: %le; n=%d; LC=%d\n",av[1], nElems, CLine::iStat);

		for(int k=1;k<CLine::linesCount;k++)
		{
			CLine::statistics[ic][k] = av[k]/nElems;
		}

		
		

		if(ik >= CLine::iStat)
		{
			CLine::statistics[ic][0] = CLine::statistics[CLine::iStat-1][0];
			nLines = ic+1;
			break;
		}
		else
		{
			CLine::statistics[ic][0] = CLine::statistics[ik][0];

		}

		ic++;

	}
	
	CDebugger::debug("Lines selected: %d\n",nLines);

	
	
	int nmaxchunk = 10;

	for(int is=1;is<nLines-1; is++)
	{
		//determine where we need to smooth
		if(nmaxchunk > nLines - 1 - is)
		{
			nmaxchunk = nLines - 1 - is;
		}
		for(int ils = is; ils < is + nmaxchunk; ils++)
		{
			//printf("LT: %le; %le; %le\n", Physics::statistics[ils][1], Physics::statistics[is][1], Physics::statistics[is+nmaxchunk][1]);
			if((CLine::statistics[ils][1] < 0.5*CLine::statistics[is][1] 
			    && CLine::statistics[ils][1] < 0.5*CLine::statistics[is+nmaxchunk][1])
			|| (CLine::statistics[ils][1] > 2*CLine::statistics[is][1] 
			    && CLine::statistics[ils][1] < 2*CLine::statistics[is+nmaxchunk][1]))
			{
				for(int k=1;k<=CLine::linesCount;k++)
				{
					CLine::statistics[ils][k] = (CLine::statistics[is][k] + CLine::statistics[ils][k] + CLine::statistics[is+nmaxchunk][k])/3;
				}
			}
		}
	}
	
	i = 0;
	int linesIp = 0;
	char fnameel[255];
	sprintf(fnameel,"%s/lineems_distribution.txt",App::output_dir);
	
	FILE *LineEmF;
	//printf("LC: %d\n",CLine::iStat);
	LineEmF = fopen(fnameel, "w+");
	int j = 1;
	fprintf(LineEmF, "# Path\t");
	while(j <= CLine::linesCount && j < 100)
	{
		fprintf(LineEmF, "%s\t", CLine::linesCapDB[CLine::lineIds[j]]);
		j++;
	}
	fprintf(LineEmF, "\n");
	while(i<nLines && i<500000)
	{
		if(CLine::statistics[i][0] > 1.e-35 && CLine::statistics[i][0] < 1.e+25)
		{
			if(linesIp > 0)
			{
				int j = 0;
				while(j <= CLine::linesCount && j < 100)
				{
					fprintf(LineEmF, "%le\t", CLine::statistics[i][j]);
					j++;
				}
				fprintf(LineEmF, "\n");
			}
			linesIp++;
		}
		
		i++;
	}
	fclose(LineEmF);





	ik = 0;
	ic = 0;
	ikprev = 0;
	
	for(int k=0;k<100;k++)
	{
		av[k] = 0.;
	}

	while(ic < CLine::iStat)
	{
		double dR = 0.01;

		for(int k=0;k<100;k++)
		{
			av[k] = 1.e-42;
		}
		int nElems = 0;

		while(dR < 1.e+19 && ik<CLine::iStat)
		{
			ik++;
			dR = fabs(GrainTemp::statistics[ik][0] - GrainTemp::statistics[ikprev][0]);
			
			for(int k=1;k<GrainTemp::nBins+1;k++)
			{
				av[k] += GrainTemp::statistics[ik][k];
			}
			nElems++;
			
		}

		ikprev = ik;

		//printf("av: %le; n=%d; LC=%d\n",av[1], nElems, CLine::iStat);

		for(int k=1;k<GrainTemp::nBins+1;k++)
		{
			GrainTemp::statistics[ic][k] = av[k]/nElems;
		}

		
		

		if(ik >= CLine::iStat)
		{
			GrainTemp::statistics[ic][0] = GrainTemp::statistics[CLine::iStat-1][0];
			nTemps = ic+1;
			break;
		}
		else
		{
			GrainTemp::statistics[ic][0] = GrainTemp::statistics[ik][0];

		}

		ic++;

	}
	
	CDebugger::debug("Temps selected: %d\n",nTemps);


	i = 0;

	char fnametmp[255];
	sprintf(fnametmp,"%s/graintemp_distribution.txt",App::output_dir);
	
	FILE *GrainTempF;
	CDebugger::debug("TC: %d\n",CLine::iStat);
	GrainTempF = fopen(fnametmp, "w+");
	while(i<nTemps && i<500000)
	{
		if(GrainTemp::statistics[i][0] > 1.e-35 && GrainTemp::statistics[i][0] < 1.e+25)
		{
			int j = 0;
			while(j <= GrainTemp::nBins+1 && j < 100)
			{
				fprintf(GrainTempF, "%le\t", GrainTemp::statistics[i][j]);
				j++;
			}
			fprintf(GrainTempF, "\n");
		}
		
		i++;
	}
	fclose(GrainTempF);



	ik = 0;
	ic = 0;
	ikprev = 0;
	
	for(int k=0;k<100;k++)
	{
		av[k] = 0.;
	}

	while(ic < CLine::iStat)
	{
		double dR = 0.01;

		for(int k=0;k<100;k++)
		{
			av[k] = 1.e-42;
		}
		int nElems = 0;

		while(dR < 1.e+19 && ik<CLine::iStat)
		{
			ik++;
			dR = fabs(CContinuum::statistics[ik][0] - CContinuum::statistics[ikprev][0]);
			
			for(int k=1;k<CContinuum::nbands+1;k++)
			{
				av[k] += CContinuum::statistics[ik][k];
			}
			nElems++;
			
		}

		ikprev = ik;

		//printf("av: %le; n=%d; LC=%d\n",av[1], nElems, CLine::iStat);

		for(int k=1;k<CContinuum::nbands+1;k++)
		{
			CContinuum::statistics[ic][k] = av[k]/nElems;
		}

		
		

		if(ik >= CLine::iStat)
		{
			CContinuum::statistics[ic][0] = CContinuum::statistics[CLine::iStat-1][0];
			nTemps = ic+1;
			break;
		}
		else
		{
			CContinuum::statistics[ic][0] = CContinuum::statistics[ik][0];

		}

		ic++;

	}
	
	CDebugger::debug("IREmits selected: %d\n",nTemps);


	i = 0;

	char fnameemit[255];
	sprintf(fnameemit,"%s/bandsemis_distribution.txt",App::output_dir);
	
	FILE *BandsEmisF;
	CDebugger::debug("TC: %d\n",CLine::iStat);
	BandsEmisF = fopen(fnameemit, "w+");
	while(i<nTemps && i<500000)
	{
		if(CContinuum::statistics[i][0] > 1.e-35 && CContinuum::statistics[i][0] < 1.e+25)
		{
			int j = 0;
			while(j <= CContinuum::nbands+1 && j < 100)
			{
				fprintf(BandsEmisF, "%le\t", CContinuum::statistics[i][j]);
				j++;
			}
			fprintf(BandsEmisF, "\n");
		}
		
		i++;
	}
	fclose(BandsEmisF);



	ik = 0;
	ic = 0;
	ikprev = 0;
	
	for(int k=0;k<100;k++)
	{
		av[k] = 0.;
	}

	while(ic < CLine::iStat)
	{
		double dR = 0.01;

		for(int k=0;k<100;k++)
		{
			av[k] = 1.e-42;
		}
		int nElems = 0;
		int maxSector = Physics::statistics[ikprev][3];
		int maxLayer = Physics::statistics[ikprev][4];
		double maxNe = Physics::statistics[ikprev][2];

		while(dR < 1.e+19 && ik<CLine::iStat)
		{
			ik++;
			dR = fabs(Physics::statistics[ik][0] - Physics::statistics[ikprev][0]);
			if(Physics::statistics[ik][2] > maxNe) {
				maxNe = Physics::statistics[ik][2];
				maxSector = Physics::statistics[ik][3];
				maxLayer = Physics::statistics[ik][4];
				
			}
			for(int k=1;k<3;k++)
			{
				av[k] += Physics::statistics[ik][k];
			}
			av[3] = maxSector;
			av[4] = maxLayer;
			av[5] = maxNe;
			nElems++;
			
		}

		ikprev = ik;

		//printf("av: %le; n=%d; LC=%d\n",av[1], nElems, CLine::iStat);

		for(int k=1;k<3;k++)
		{
			Physics::statistics[ic][k] = av[k]/nElems;
		}

		for(int k=3;k<6;k++)
		{
			Physics::statistics[ic][k] = av[k];
		}		
		

		if(ik >= CLine::iStat)
		{
			Physics::statistics[ic][0] = Physics::statistics[CLine::iStat-1][0];
			nPhys = ic+1;
			break;
		}
		else
		{
			Physics::statistics[ic][0] = Physics::statistics[ik][0];

		}

		ic++;

	}
	
	CDebugger::debug("Physics selected: %d\n",nPhys);

	
	nmaxchunk = 10;

	for(int is=1;is<nPhys-1; is++)
	{
		//determine where we need to smooth
		if(nmaxchunk > nPhys - 1 - is)
		{
			nmaxchunk = nPhys - 1 - is;
		}
		for(int ils = is; ils < is + nmaxchunk; ils++)
		{
			//printf("LT: %le; %le; %le\n", Physics::statistics[ils][1], Physics::statistics[is][1], Physics::statistics[is+nmaxchunk][1]);
			if(Physics::statistics[ils][1] < 0.5*Physics::statistics[is][1] 
			    && Physics::statistics[ils][1] < 0.5*Physics::statistics[is+nmaxchunk][1])
			{
				for(int k=1;k<=3;k++)
				{
					Physics::statistics[ils][k] = (Physics::statistics[is][k] + Physics::statistics[ils][k] + Physics::statistics[is+nmaxchunk][k])/3;
				}
				//printf("NW: %le; %le; %le\n", Physics::statistics[ils][1], Physics::statistics[is][1], Physics::statistics[is+nmaxchunk][1]);
				//printf("c: %d; P: %le\n",ils, Physics::statistics[ils][0]);
			}/*
			if(pow(Physics::statistics[ils][1]-AVG,2)/pow(AVG,2) > 0.5)
			{
				for(int k=1;k<=3;k++)
				{
					Physics::statistics[ils][k] = (Physics::statistics[ils-1][k] + Physics::statistics[ils][k] + Physics::statistics[ils+1][k])/3;
				}
			}*/
		}
	}
	

	i = 0;

	char fnamephys[255];
	sprintf(fnamephys,"%s/phys_distribution.txt",App::output_dir);
	int ipc = 0;
	FILE *OverviewF;
	CDebugger::debug("TC: %d\n",CLine::iStat);
	OverviewF = fopen(fnamephys, "w+");
	while(i<nPhys && i<500000)
	{
		if(Physics::statistics[i][0] > 1.e-35 && Physics::statistics[i][0] < 1.e+25)
		{
			if(ipc > 0)
			{
				int j = 0;
				while(j <= 6)
				{
					fprintf(OverviewF, "%le\t", Physics::statistics[i][j]);
					j++;
				}
				fprintf(OverviewF, "\n");
			}
			ipc++;
		}
		i++;
	}
	fclose(OverviewF);

	CIteration::finalizeIterations();
	CDebugger::log("Iteratiom was finished!");
	return 1;
}