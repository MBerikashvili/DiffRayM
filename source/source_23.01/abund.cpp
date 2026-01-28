#include "basics.h"
#include "reader.h"
#include "abund.h"
#include "geometry.h"
#include "string.h"


int Abund::nElements;
int Abund::nsectors = 0;
int Abund::iSector = 0;
int Abund::nrows = 0;
int Abund::readen = 0;
int Abund::secRows[20];
double ***Abund::abundances;

char Abund::elements[100][14];

double Abund::abmass[100];
double Abund::abemso[100];
double Abund::mass;
double Abund::emso;

double Abund::statistics[500000][100];

int Abund::refreshStatistics()
{
	for(int i=0;i<500000;i++)
	{
		for(int j=0; j<100; j++)
		{
			Abund::statistics[i][j] = 0.0;
		}
	}
	return 1;
}

int Abund::initSectorials(int nsectors)
{
	//assuming we have only 1 sector (SS)
	Abund::abundances = new double**[nsectors];
	Abund::nsectors = nsectors;
	return 1;
}

int Abund::readAbunds(char fname[500],int iSector)
{

	if(Abund::nsectors<=0)
	{
		Abund::initSectorials(CGeometry::nSectors);
	}
	FILE *FREAD;

	FREAD = fopen(fname,"r");

	if(!FREAD || FREAD == NULL)
	{
		printf("%s\n", fname);
		CBasics::throwError("ERROR OPENING FILE WITH NULL POINTER!");
	}

	Abund::iSector = iSector;
	printf("Reading abundances\n");
	FREAD = fopen(fname,"r");
	CReader::read_file_array(FREAD,2550,1,0,Abund::getRadii,Abund::putAbunds,Abund::putAbundVal, Abund::getElementList);
	
	fclose(FREAD);

	return 1;
}

int Abund::putAbunds(int nlines)
{
	Abund::nElements = nlines-1;

	int is = Abund::iSector;
	{
		Abund::abundances[is] = new double *[Abund::secRows[is]+20];

		for(int ir=0;ir<Abund::secRows[is]+20;ir++)
		{
			Abund::abundances[is][ir] = new double[Abund::nElements];

			for(int il=0;il<Abund::nElements;il++)
			{
				Abund::abundances[is][ir][il] = 0.0;
			}
		}
	}
	return 1;
}

int Abund::getRadii(int nrows)
{
	Abund::nrows = nrows;
	Abund::secRows[Abund::iSector] = nrows;
	return 1;
}

int Abund::putAbundVal(int raw, int col, double val)
{
	
	if(col>0) // not radii
	{
		Abund::abundances[Abund::iSector][raw][col-1] = pow(10.0,val);
		//Abund::abundances[Abund::iSector][raw][col-1] = pow(10.0,-1.00);//*(1000-raw)/1000;
	}
	/*if(col>1) // not radii
	{
		Abund::abundances[Abund::iSector][raw][col-1] = pow(10.0,-4.00)*(1000-raw)/1000;
	}*/
	//printf("%d %d %d - %lf\n",CLine::iSector,raw,col,CLine::emits[CLine::iSector][raw][col-1] );
	
	return 1;
}

int Abund::getElementList(int irow,char str[1000])
{
	if(irow==0 && Abund::readen==0)
	{
		Abund::readen = 1;

		char abel[14];
		char *data = str;

		int offset;

		int iel = 0;

		
		while (sscanf(data, "%s%n", &abel, &offset) == 1)
    	{
    		data += offset;
    			
			if(iel>1)
    			sprintf(Abund::elements[iel-2],"%s",abel);

    		iel++;
    	}

    	Abund::nElements = iel - 2;
	}
	//printf("%d; %d\n", irow, CLine::readen);
	/*int lineIds[100];
	int linePos[100];

	int idSorted[100];
	printf("-----\n");
	if(irow==1 && Abund::readen==0)
	{
		Abund::readen = 1;
		char * pch;
		int il = 0;
		for(int i=0;i<CLine::ndatabase;i++)
		{
			pch=strstr(str,CLine::linesCapDB[i]);
			
			int pos = pch - str + 1;

			if(pos>4)
			{
				printf("%s  ->  %d\n", CLine::linesCapDB[i], pos);
				bool found = false;
				for(int j=0;j<il;j++)
				{
					if(linePos[j] == pos)
					{
						found = true;
					}
				}
				if(!found)
				{	
					linePos[il] = pos;
					lineIds[il] = i;
					il++;
				}
			}
			
		}

		for(int i=0;i<il;i++)
		{
			for(int j=0;j<il-i-1;j++)
			{
				if(linePos[j]>linePos[j+1])
				{
					int pos_next = linePos[j+1];
					int id_next  = lineIds[j+1];
					linePos[j+1] = linePos[j];
					lineIds[j+1] = lineIds[j];
					linePos[j] = pos_next;
					lineIds[j] = id_next;
				}
			}
		}

		for(int i=0;i<il;i++)
		{
			CLine::lineIds[i] = lineIds[i];
			printf("%d --- %d (%d)\n", lineIds[i],linePos[i],il);
		}

		printf("TTTTT\n");

	}*/

	return 1;
}
