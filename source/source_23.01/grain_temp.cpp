#include "basics.h"
#include "reader.h"
#include "grain_temp.h"
#include "geometry.h"
#include "string.h"
#include "app.h"


int GrainTemp::nBins = 0;
int GrainTemp::nsectors = 0;
int GrainTemp::iSector = 0;
int GrainTemp::nrows = 0;
double ***GrainTemp::gridTemps;
double GrainTemp::statistics[500000][100];
double GrainTemp::massTotal;
double GrainTemp::massWeighted[100];

int GrainTemp::refreshStatistics()
{
	for(int i=0;i<500000;i++)
	{
		for(int j=0; j<100; j++)
		{
			GrainTemp::statistics[i][j] = 0.0;
		}
	}
	for(int j=0; j<100; j++)
	{
		GrainTemp::massWeighted[j] = 0.0;
	}
	GrainTemp::massTotal = 0.0;
	return 1;
}


int GrainTemp::initSectorials(int nsectors)
{
	if(App::CalcGrainTemp)
		GrainTemp::nBins = 20;
	
	GrainTemp::gridTemps = new double**[nsectors];
	GrainTemp::nsectors = nsectors;
	return 1;
}

int GrainTemp::readTemps(char fname[500],int iSector)
{

	if(GrainTemp::nsectors<=0)
	{
		GrainTemp::initSectorials(CGeometry::nSectors);
	}
	FILE *FREAD;

	FREAD = fopen(fname,"r");

	if(!FREAD || FREAD == NULL)
	{
		printf("%s\n", fname);
		CBasics::throwError("ERROR OPENING FILE WITH NULL POINTER!");
	}

	GrainTemp::iSector = iSector;
	printf("Reading temps\n");
	FREAD = fopen(fname,"r");
	CReader::read_file_array(FREAD,2550,1,0,GrainTemp::getRadii,GrainTemp::putTemps,GrainTemp::putTempVal);
	
	fclose(FREAD);

	return 1;
}

int GrainTemp::putTemps(int nlines)
{
	GrainTemp::nBins = nlines-1;
	printf("nLinesTEMP::::%d\n",nlines-1);

	int is = GrainTemp::iSector;
	{
		GrainTemp::gridTemps[is] = new double *[GrainTemp::nrows+20];

		for(int ir=0;ir<GrainTemp::nrows+20;ir++)
		{
			GrainTemp::gridTemps[is][ir] = new double[GrainTemp::nBins];

			for(int il=0;il<GrainTemp::nBins;il++)
			{
				GrainTemp::gridTemps[is][ir][il] = 0.0;
			}
		}
	}
	return 1;
}

int GrainTemp::getRadii(int nrows)
{
	GrainTemp::nrows = nrows;
	return 1;
}

int GrainTemp::putTempVal(int raw, int col, double val)
{
	
	if(col>0) // not radii
	{
		GrainTemp::gridTemps[GrainTemp::iSector][raw][col-1] = val;
	}
	//printf("%d %d %d - %lf\n",GrainTemp::iSector,raw,col,GrainTemp::gridTemps[GrainTemp::iSector][raw][col-1] );
	
	return 1;
}
