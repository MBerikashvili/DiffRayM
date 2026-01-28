#include "basics.h"
#include "reader.h"
#include "continuum.h"
#include "geometry.h"
#include "integration/matrix.h"


int CContinuum::cellCount = 0;
int CContinuum::nrows;
double ***CContinuum::emits;
int CContinuum::nsectors = 0;
int CContinuum::iSector;
double ***CContinuum::opacs;
double ***CContinuum::trans;
double *CContinuum::anu;
double CContinuum::endSkip;
int CContinuum::skipInterval;
IRBand CContinuum::bands[40];
int CContinuum::nbands = 0;
double *CContinuum::in_fluxes;
double CContinuum::statistics[500000][41];
double *CContinuum::effectiveSourceEmissivity;
int CContinuum::importantCells[2][2];
double CContinuum::importantIntervals[2][2];
	
int CContinuum::refreshStatistics()
{
	for(int i=0;i<500000;i++)
	{
		for(int j=0; j<41; j++)
		{
			CContinuum::statistics[i][j] = 0.0;
		}
	}
	return 1;
}

double CContinuum::getBandEmissivity(int iBand, int iSector, int iLayer)
{
	double result = 0.;
	for(int i=CContinuum::bands[iBand].iNuLeft; i<=CContinuum::bands[iBand].iNuRight;i++)
	{
		result += CContinuum::emits[iSector][iLayer][i];
	}
	return result;
}

int CContinuum::initSectorials(int nsectors)
{
	//assuming we have only 1 sector (SS)
	printf("initSectorials\n");
	CContinuum::emits = new double**[nsectors];
	CContinuum::opacs = new double**[nsectors];
	CContinuum::trans = new double**[nsectors];
	CContinuum::nsectors = nsectors;
	return 1;
}

int CContinuum::readMesh(char fname[500])
{
	if(CContinuum::nsectors<=0)
	{
		CContinuum::initSectorials(CGeometry::nSectors);
	}
	FILE *FREAD;

	FREAD = fopen(fname,"r");

	if(!FREAD || FREAD == NULL)
	{
		printf("FILE NOT FOUND: %s\n", fname);
		return 0;
		//CBasics::throwError("ERROR OPENING FILE WITH NULL POINTER!");
	}

	CReader::read_file_array(FREAD,600,1,0,CContinuum::setMeshSize,0,CContinuum::putMeshEnergy);
	fclose(FREAD);
	int iImportant = 0;
	CContinuum::importantIntervals[0][0] = 9.e-4;
	CContinuum::importantIntervals[0][1] = 3.e-3;
	CContinuum::importantIntervals[1][0] = 0.9;
	CContinuum::importantIntervals[1][1] = 2.0;
	
	for(int i=0;i<2;i++)
	{
		CContinuum::importantCells[i][0] = 0;
		CContinuum::importantCells[i][1] = 0;
	}
	for(int icell = 0; icell < CContinuum::cellCount; icell++)
	{
		if(iImportant > 1)
			break;
		double nu = CContinuum::anu[icell];
		if(nu > CContinuum::importantIntervals[iImportant][0] && CContinuum::importantCells[iImportant][0] <= 0)
		{
			CContinuum::importantCells[iImportant][0] = icell;
		}
		if(nu > CContinuum::importantIntervals[iImportant][1] && CContinuum::importantCells[iImportant][1] <= 0)
		{
			CContinuum::importantCells[iImportant][1] = icell;
			iImportant++;
		}
		
	}
	return 1;
}

int CContinuum::setMeshSize(int nrows)
{
	if(CContinuum::cellCount<=0)
	{
		CContinuum::cellCount = nrows;

		CContinuum::anu = new double[nrows];

		CContinuum::effectiveSourceEmissivity = new double[nrows];
		for (int ic = 0; ic < CContinuum::cellCount; ++ic)
		{
			CContinuum::effectiveSourceEmissivity[ic] = 0.;
		}
		for(int i=0;i<CGeometry::nSectors;++i)
		{
			int nrads = CGeometry::nRadiuses[i]+1;
			CContinuum::emits[i] = new double*[nrads];
			CContinuum::opacs[i] = new double*[nrads];
			CContinuum::trans[i] = new double*[nrads];
			for(int ir=0;ir<CGeometry::nRadiuses[i];++ir)
			{
				CContinuum::emits[i][ir] = new double[CContinuum::cellCount];
				CContinuum::opacs[i][ir] = new double[CContinuum::cellCount];
				CContinuum::trans[i][ir] = new double[CContinuum::cellCount];
				for (int ic = 0; ic < CContinuum::cellCount; ++ic)
				{
					CContinuum::emits[i][ir][ic] = 0.00;
					CContinuum::opacs[i][ir][ic] = 0.00;
					CContinuum::trans[i][ir][ic] = 0.00;
				}
			}
		}
	}

	CContinuum::in_fluxes = new double[CContinuum::cellCount];
	CMatrix::ray_intr = new double[CContinuum::cellCount];
	CMatrix::ray_cont = new double[CContinuum::cellCount];
	return 1;
}

int CContinuum::putMeshEnergy(int raw, int col, double val)
{
	
	if(col<=0) // not radii
	{
		CContinuum::anu[raw] = val;
	}
	//printf("%d %d %d - %lf\n",CLine::iSector,raw,col,CLine::emits[CLine::iSector][raw][col-1] );
	
	return -1;
}

int CContinuum::readEmissivity(char fname[500],int iSector)
{
		printf("OK-3\n");
	if(CContinuum::nsectors<=0)
	{
		CContinuum::initSectorials(CGeometry::nSectors);
	}
		printf("OK-2\n");
	FILE *FREAD;

	FREAD = fopen(fname,"r");

	if(!FREAD || FREAD == NULL)
	{
		printf("%s\n", fname);
		printf("WARNING: FILE NOT FOUND");
		return 0;
	}

	CContinuum::iSector = iSector;
		printf("OK-1\n");
	CReader::read_file_array(FREAD,128*1024,1,0,CContinuum::getRadii,0,CContinuum::putContEmissivity);
	fclose(FREAD);

	return 1;
}

int CContinuum::getRadii(int nrows)
{
	CContinuum::nrows = nrows;

	/*if(nrows != CGeometry::nRadiuses[CContinuum::iSector] && nrows+1 != CGeometry::nRadiuses[CContinuum::iSector])
	{
		printf("Radiuses[%d] = %d, but %d continuums found!",CContinuum::iSector,CGeometry::nRadiuses[CContinuum::iSector],nrows);
		CBasics::throwError("ERROR! Wrong continuum data!");
	}*/
	return 1;
}

int CContinuum::putContEmissivity(int raw, int col, double val)
{
	
	if(col>0 && col-1<CContinuum::cellCount) // not radii
	{
		CContinuum::emits[CContinuum::iSector][raw][col-1] = val;
	}
	
	return 1;
}

int CContinuum::readOpacity(char fname[500],int iSector)
{

	if(CContinuum::nsectors<=0)
	{
		CContinuum::initSectorials(CGeometry::nSectors);
	}
	FILE *FREAD;

	FREAD = fopen(fname,"r");

	if(!FREAD || FREAD == NULL)
	{
		printf("%s\n", fname);
		printf("WARNING: FILE NOT FOUND");
		return 0;
	}

	CContinuum::iSector = iSector;

	CReader::read_file_array(FREAD,128*1024,1,0,CContinuum::getRadii,0,CContinuum::putContOpacity);
	fclose(FREAD);

	return 1;
}

int CContinuum::putContOpacity(int raw, int col, double val)
{

	if(col>0 && col-1<CContinuum::cellCount) // not radii
	{
		

		CContinuum::opacs[CContinuum::iSector][raw][col-1] = val;
	}
	
	return 1;
}

int CContinuum::readTransitions(char fname[500],int iSector)
{

	if(CContinuum::nsectors<=0)
	{
		CContinuum::initSectorials(CGeometry::nSectors);
	}
	FILE *FREAD;

	FREAD = fopen(fname,"r");

	if(!FREAD || FREAD == NULL)
	{
		printf("%s\n", fname);
		printf("WARNING: FILE NOT FOUND");
		return 0;
	}

	CContinuum::iSector = iSector;

	CReader::read_file_array(FREAD,255*1024,1,0,CContinuum::getRadii,0,CContinuum::putTransitions);
	fclose(FREAD);

	return 1;
}

int CContinuum::putTransitions(int raw, int col, double val)
{

	if(col>0 && col-1<CContinuum::cellCount) // not radii
	{
		

		CContinuum::trans[CContinuum::iSector][raw][col-1] = val;
	}
	
	return 1;
}

int CContinuum::readBands()
{
	DIR *dir;
	struct dirent *ent;
	char fnamebase[250];

	if ((dir = opendir (App::data_bands)) != NULL) {
	  /* print all the files and directories within directory */
	  while ((ent = readdir (dir)) != NULL) 
	  {
	   
	    
	  	if(strlen(ent->d_name)>3)
	    {
	    	sscanf(ent->d_name," %[^\t\n\.]",fnamebase);
	    	//CContinuum::bands[CContinuum::nbands] = new IRBand;
	    	CContinuum::bands[CContinuum::nbands].read_band(ent->d_name,fnamebase);
	  		CContinuum::nbands++;
	  	}

	  }
	  closedir (dir);
	} else {
	  /* could not open directory */
	  printf("Unable open dir %s\n", App::data_bands);
	  exit(1);
	}
	return 1;
}

int CContinuum::assignBands()
{



	for(int iBand = 0; iBand < CContinuum::nbands; iBand++)
	{
		int icell = 0;

		while(icell<CContinuum::cellCount && CContinuum::anu[icell] < CContinuum::bands[iBand].leftCont)
		{
			icell++;
		}

		CContinuum::bands[iBand].iNuLeft = icell-1;

		if(CContinuum::bands[iBand].iNuLeft < 0)
			CContinuum::bands[iBand].iNuLeft = 0;

		while(icell<CContinuum::cellCount && CContinuum::anu[icell] < CContinuum::bands[iBand].rightCont)
		{
			icell++;
		}

		CContinuum::bands[iBand].iNuRight = icell-1;


		if(CContinuum::bands[iBand].iNuRight >= CContinuum::cellCount)
			CContinuum::bands[iBand].iNuRight = CContinuum::cellCount-1;

	}
	return 1;
}

int CContinuum::readInwardFluxes(char fname[500])
{

	FILE *FP;

	FP = fopen(fname,"r");

	if(!FP)
		return 0;

	double anu = 0.;
	double flx = 0.;
	double anu_prev = 0.;
	int icl = 0;
	char chLab[500];

	fgets(chLab,500,FP);

	//FILE *F2P;

	//F2P = fopen("fluxes.dat","w+");

	while(!feof(FP) && icl < CContinuum::cellCount)
	{
		fscanf(FP,"%le%le", &anu, &flx);
		
		double delta_anu = anu - anu_prev;

		double C_conv = delta_anu/(pow(anu,2.0)*2.179874099E-11);
		//C_conv = 1.0;
		CContinuum::in_fluxes[icl] = C_conv*flx/**4*M_PI*pow(CGeometry::inRadius[0],2.0)*/; //LUMINOSITY!
		//printf("Cont: \n");
		anu_prev = anu;
		icl++;

		//fprintf(F2P, "%le\t%le\t%le\n", anu, flx, C_conv);
	}
	
	fclose(FP);
	CContinuum::getIonizingSourceEmissivity();
	return 1;
}

int CContinuum::getIonizingSourceEmissivity()
{
	for(int ic = 0; ic < CContinuum::cellCount; ic++)
	{
		CContinuum::effectiveSourceEmissivity[ic] = CContinuum::in_fluxes[ic]*3/CGeometry::inRadius[0];
	}
	return 1;
}

/*int CContinuum::readLines(char fname[500],int iSector)
{

	if(CLine::nsectors<=0)
	{
		CLine::initSectorials(CGeometry::nSectors);
	}
	FILE *FREAD;

	FREAD = fopen(fname,"r");

	if(!FREAD || FREAD == NULL)
	{
		printf("%s\n", fname);
		CBasics::throwError("ERROR OPENING FILE WITH NULL POINTER!");
	}

	CLine::iSector = iSector;

	FREAD = fopen(fname,"r");
	CReader::read_file_array(FREAD,2550,2,0,CLine::getRadii,CLine::putLine,CLine::putLineVal);
	fclose(FREAD);

	return 1;
}

int CLine::putLine(int nlines)
{
	CLine::linesCount = nlines-1;

	int is = CLine::iSector;
	{
		CLine::emits[is] = new double *[CLine::nrows];

		for(int ir=0;ir<CLine::nrows;ir++)
		{
			CLine::emits[is][ir] = new double[CLine::linesCount];

			for(int il=0;il<CLine::linesCount;il++)
			{
				CLine::emits[is][ir][il] = 0.0;
			}
		}
	}
}

int CLine::getRadii(int nrows)
{
	CLine::nrows = nrows;
}

int CLine::putLineVal(int raw, int col, double val)
{
	
	if(col>0) // not radii
	{
		CLine::emits[CLine::iSector][raw][col-1] = pow(10.0,val);
	}
	//printf("%d %d %d - %lf\n",CLine::iSector,raw,col,CLine::emits[CLine::iSector][raw][col-1] );
	
	return 1;
}*/