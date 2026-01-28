#pragma once
#include "basics.h"
#include "geom3d.h"

class App{
public:
	static char *model_name;
	static char *data_suffix;
	static int nSectors;
	static vector3 obj_rotation;
	static double distance;
	static double phi;
	static double theta;
	static double phi_width;
	static double theta_width;
	static double cov_fac;
	static bool CalcCont;
	static bool CalcLines;
	static bool CalcOpac;
	static bool CalcAbund;
	static bool CalcIRBands;
	static bool CalcGrainTemp;
	static bool CalcOverviews;
	static double age;
	static char output_dir[255];
	static char output_dir_in[255];
	static char input_dir[255];
	static char data_base[255];
	static char data_bands[255];
	static char commands_path[255];

	static TRay rayToObj; //ray to center
	static TRay rayIntegration;
	static int AppMode;
	static bool isStatMode;

	static Apperture appertures[10];
	static int nApps;
	static int iApp;
	static int integration_mode;
	static double AppDPhi;
	static double AppDTheta;
	static int maxiterations;
	static double precision;
	static bool punchStatistics;
	static bool printBands;
	static bool isIrregularSource;
	static bool pointsSource;
	static char pointsFile[255];
	static int nSkipPoints;
	static char fluxesOutput[255];

	
	static bool init();
	static bool setRay(double phi, double theta);
	static bool readCommands();
	static bool addApperture(double phi, double theta, double dphi, double dtheta);
	static bool initAperture(int i);
	static int geometryType;
	static int onlySectorNo;
	static int coverSector;
	static double minPhiCen, maxPhiCen, minThetaCen, maxThetaCen;
	static bool usePredictiveMode;
	static bool isSpacial;
};