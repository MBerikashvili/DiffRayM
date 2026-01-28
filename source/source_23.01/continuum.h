#pragma once
#include "basics.h"
#include "reader.h"
#include "dirent.h"
#include "app.h"

struct IRBand{
	public:
		double ennu[1000];
		double transmitions[1000];
		int nAnus;
		double leftCont, rightCont;
		char label[500];
		int iNuLeft;
		int iNuRight;
		double totalLum = 0.;
		double lumYan = 0.;


		int read_band(char *fname, char blabel[250])
		{
			iNuLeft = 0;

			iNuRight = 0;

			char fpath[500];

			sprintf(fpath,"%s/%s",App::data_bands,fname);

			sprintf(label,"%s", blabel);

			FILE *FLb;

			FLb = fopen(fpath,"r");

			nAnus = 0;

			char chLab[500];
			char flname[500];
			sprintf(flname, "%s/bands_continuum.dat",App::data_base);

			double mLeft = 0.;
			double mRight = 0.;
			//trying to get real band constrains
			FILE *bdb;

			bdb = fopen(flname,"r");
			bool bFound = false;
			if(NULL != bdb)
			{

				
				char name[5];
				char len[5];
				double from;
				double to;
				while(!feof(bdb) && !bFound)
				{
					fscanf(bdb,"%s %s %lfm %lfm",name, len, &from, &to);
					
				
					char bl1[5];
					char bl2[5];

					sscanf(blabel,"%s %s",bl1,bl2);
					if(strcmp(bl1,"F") == 0)
					{
						printf("%s %s %lf %lf\n",name, len, from, to);
						printf("FROM LAB: %s; %s\n",bl1,bl2);
					}
					if(strcmp(bl1,name) == 0 && strcmp(bl2,len) == 0)
					{
						//printf("")
						bFound = true;
						mLeft = from;
						mRight = to;
						break;
					}
				}
				
				fclose(bdb);
			}
			else
			{
				printf("Unable to open file %s\n",flname);
				exit(1);
			}
			if(FLb)
			{
				fgets(chLab,500,FLb);

				double nu,tr = 1.;

				nu = 1.0;
				
				while(!feof(FLb) && nu>1.e-5)
				{

					

					fscanf(FLb, " %lf %lf ", &nu, &tr);

					//printf("hello %le %le\n", nu, tr);

					if(nu > 1.e-5 && (!bFound || (nu >= 0.995*mLeft*1.e+4 && nu <= 1.005*mRight*1.e+4)))
					{
						ennu[nAnus] = nu;

						transmitions[nAnus] = tr;

						nAnus++;
					}
				}
		
				
				rightCont = 1.e+5;
				leftCont = 1.e+5;

				if(nAnus > 0)
				{
					rightCont = 912./ennu[0];
					leftCont = 912./ennu[nAnus-1];
				}
				
				
				fclose(FLb);
			}
			return 1;
		}

};

class CContinuum{
	public:

	static double importantIntervals[2][2];
	static int importantCells[2][2];
	static int cellCount;
	static int nrows;
	static double ***emits;
	static double ***opacs;
	static double ***trans;
	static double *effectiveSourceEmissivity;
	static double *anu;
	static double endSkip;
	static int skipInterval;
	static int nsectors;
	static int iSector;
	static IRBand bands[40];
	static int nbands;
	static double *in_fluxes;
	static double statistics[500000][41];
	//Init sectorial data
	static int initSectorials(int nsectors);
	//Read contmesh
	static int readMesh(char fname[500]);

	static int getIonizingSourceEmissivity();
	//Assign Mesh Size
	static int setMeshSize(int cells);
	//Put Mesh cell enerhy
	static int putMeshEnergy(int row, int col, double val);

	//Read cont. emissivity
	static int readEmissivity(char fname[500],int iSector);

	//Put cont. emissivity
	static int putContEmissivity(int row, int col, double val);

	//Read cont. opacity
	static int readOpacity(char fname[500],int iSector);
	
	//Put cont. opacity
	static int putContOpacity(int row, int col, double val);

	//Read transitions
	static int readTransitions(char fname[500],int iSector);
	
	//Put transitions
	static int putTransitions(int row, int col, double val);

	static int getRadii(int nrows);

	static int readBands();

	static int assignBands();

	static int readInwardFluxes(char fname[500]);

	static double getBandEmissivity(int iBand, int iSector, int iLayer);

	static int refreshStatistics();
};
