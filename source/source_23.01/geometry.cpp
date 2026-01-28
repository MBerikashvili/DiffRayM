#include "geometry.h"
#include "app.h"
#include "reader.h"
#include "geom3d.h"

int CGeometry::nSectors = 0;
int CGeometry::iSector;
int *CGeometry::nRadiuses;
double *CGeometry::inRadius;
double **CGeometry::outer_radius;

int CGeometry::readSectors(char fpattern[500])
{
	CGeometry::nSectors = App::nSectors;
	CGeometry::nRadiuses = new int[CGeometry::nSectors];
	CGeometry::inRadius = new double[CGeometry::nSectors];
	CGeometry::outer_radius = new double*[CGeometry::nSectors];
	return 1;
}

int CGeometry::readRadiuses(char fpattern[500], int iSector, double iRad)
{

		if(CGeometry::nSectors<=0)
		{
			CGeometry::readSectors();
		}
		FILE *FSEC;

		FSEC = fopen(fpattern,"r");

		CGeometry::iSector = iSector;

		if(!FSEC || FSEC == NULL)
		{
			CGeometry::nRadiuses[iSector] = 0;
			CGeometry::inRadius[iSector] = 0.0;
			CGeometry::outer_radius[iSector] = new double[1];
			CGeometry::outer_radius[iSector][0] = 0.0;
			printf("FILE %s NOT FOUND\n",fpattern);
			return 0;
		}

		CReader::read_file_array(FSEC,255*1024,0,0,CGeometry::getRadii,0,CGeometry::putLineVal);

		fclose(FSEC);

		if(iRad > 0.)
		{
			for(int i=0; i< CGeometry::nRadiuses[iSector]; i++)
			{
				CGeometry::outer_radius[iSector][i] += iRad;
			}
			CGeometry::inRadius[iSector] = iRad;
		}
		return 1;
}

int CGeometry::getRadii(int nrows)
{
	CGeometry::nRadiuses[CGeometry::iSector] = nrows;
	CGeometry::outer_radius[CGeometry::iSector] = new double[nrows+1];
	return 1;
}

int CGeometry::putLineVal(int row,int col,double val)
{
	if(col == 0)
	{
		if(row == 0)
			CGeometry::inRadius[CGeometry::iSector] = val;
		
		CGeometry::outer_radius[CGeometry::iSector][row] = val;
	}

	return -1; //code to jump next line
}