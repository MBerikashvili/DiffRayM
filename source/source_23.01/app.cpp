#include "basics.h"
#include "app.h"
#include "geom3d.h"
#include "math.h"
#include "reader.h"
#include <dirent.h>
#include <sys/stat.h>
#include "spectra/isophote.h"
#include "output/debugger.h"

char *App::model_name = "wew";
char *App::data_suffix = "Age60.00Myr.dat";
char App::data_base[255];
char App::data_bands[255];
int App::nSectors = 20; //set to 20
vector3 App::obj_rotation = vector3(
		0, //that means nothing
		0, //0.376225*M_PI,//0.376094213*M_PI,//M_PI/4, //Ange between sector 0 midline and direction to observer
		//(0.0/40)*M_PI,
		0//0.360278922*M_PI  //Angle between plane, perpendicular to symmethry axis and direction to observer 
	);

double App::distance = 1.e+23;
double App::phi = 0;//0.376094213*M_PI;
double App::theta = 0;//0.360278922*M_PI;
double App::phi_width = 2*M_PI;
double App::theta_width = M_PI;
double App::cov_fac = 0.025;
double App::age = 60.0;
double App::AppDPhi = 0.0;
double App::AppDTheta = 0.0;
double App::precision = 0.02;
double App::minPhiCen = 0.;
double App::maxPhiCen = 0.;
double App::minThetaCen = 0.;
double App::maxThetaCen = 0.;

char App::output_dir[255];
char App::output_dir_in[255];
char App::input_dir[255];
char App::commands_path[255];

bool App::CalcCont = true;
bool App::CalcOpac = true;
bool App::CalcLines = true;
bool App::CalcAbund = true;
bool App::CalcGrainTemp = true; //problem here
bool App::CalcOverviews = true; 
bool App::CalcIRBands = true;
bool App::isStatMode = true;
bool App::punchStatistics = true;
bool App::printBands = true;
int App::geometryType = 0;
bool App::pointsSource = false;
char App::pointsFile[255];
char App::fluxesOutput[255];
bool App::usePredictiveMode = false;
bool App::isSpacial = false;

TRay App::rayToObj;
TRay App::rayIntegration;

int App::AppMode = 0;
int App::nSkipPoints = 0;
int App::iApp = 0;

Apperture App::appertures[10];
int App::nApps = 0;
int App::integration_mode = 1;
int App::maxiterations = 10;
int App::onlySectorNo = -1;
int App::coverSector = -1;
bool App::isIrregularSource = false;

bool App::addApperture(double phi, double theta, double dphi, double dtheta){
	
	App::AppMode = 1;
	App::appertures[App::nApps].reset(phi,theta,dphi,dtheta);

	printf("Resetted: %d %lf %lf\n", App::nApps, App::appertures[App::nApps].phi_width, App::appertures[App::nApps].theta_width);
	fflush(stdout);
	
	App::nApps++;
	printf("New Apps: %d\n", App::nApps);
	return true;
}

bool App::init()
{
	//printf("App Theta: %le\n", App::theta);
	vector3 start_pos = spherical2desc(vector3(App::distance, M_PI + App::phi, -App::theta));
	printf("Start Point: ");
	printvec(start_pos);
	vector3 angle = vector3(0, M_PI + App::phi, App::theta);
	//printvec(App::obj_rotation);
	TRay rayToObj = TRay(start_pos, vector3(0,App::phi,App::theta));
	printvec(rayToObj.start);
	printvec(rayToObj.angle);
	rayToObj = rotateRay(rayToObj,multiply(1.0,App::obj_rotation));
	printf("Acnv:\n");
	printvec(rayToObj.start);
	printvec(rayToObj.angle);
	App::rayToObj = rayToObj;
	//printf("======!!!====\n");
	//printvec(App::rayToObj.angle);
	//exit(1);
	return true;
}

bool App::initAperture(int i)
{
	App::iApp = i;
	Apperture a = App::appertures[i];
	App::AppDPhi = a.phi;
	App::AppDTheta = a.theta;
	App::phi_width = a.phi_width;
	App::theta_width = a.theta_width;

	//char path[500];

	sprintf(App::output_dir,"%s/app_%d",App::output_dir_in,i);

	CDebugger::debug("App to %s\n",App::output_dir);

	DIR *st;

	st = opendir(App::output_dir);
	if (st == NULL || !st)
	{
    		/* Directory does not exist. EEXIST for race condition */
    		mkdir(App::output_dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}

	App::init();
	return true;
}

bool App::setRay(double phi, double theta)
{
	vector3 start_pos = spherical2desc(vector3(App::distance, M_PI + phi, -theta));

	vector3 angle = vector3(0, M_PI + phi, theta);

	TRay rayToObj = TRay(start_pos, vector3(0, phi, theta));

	rayToObj = rotateRay(rayToObj,multiply(1.0,App::obj_rotation));

	/*printf("RAY SET TO:\n");
	printf("--- %le; %le -----\n", phi, theta);
	printvec(rayToObj.start);
	printvec(rayToObj.angle);*/

	App::rayToObj = rayToObj;
	//printf("|||||||\n");
	//printvec(App::rayToObj.angle);
	return true;
}

bool App::readCommands()
{
	FILE *F;

	F = fopen(App::commands_path,"r");
	char **commands;
	CDebugger::init(0);
	commands = CReader::read_commands(F);
	while(commands != NULL && commands[0] != NULL)
	{
		printf("CMD:------------%s------------------\n",commands[0]);
		if(strcmp(commands[0],"distance") == 0)
		{
			double factor = 1.0;

			printf("distance\n");
			if(strcmp(commands[1],"m") == 0)
			{
				factor = 100;
			}
			else
			if(strcmp(commands[1],"km") == 0)
			{
				factor = 1.e+5;
			}
			else
			if(strcmp(commands[1],"file") == 0)
			{
				sscanf(commands[2],"%s",&App::pointsFile);
				App::pointsSource = true;
				sscanf(commands[3],"%d",&App::nSkipPoints);
			}	
			

			double distance;

			printf("%s\n", commands[2]);

			sscanf(commands[2],"%lf",&distance);

			printf("DIST: %le",distance);
			
			App::distance = pow(10.0,distance)*factor;
		}
		else
		if(strcmp(commands[0],"object") == 0)
		{
			printf("object\n");
			if(strcmp(commands[1],"phi") == 0)
			{
				sscanf(commands[2],"%lf",&App::phi);
			}
			else
			if(strcmp(commands[1],"theta") == 0)
			{
				sscanf(commands[2],"%lf",&App::theta);
			}
			else
			if(strcmp(commands[1],"age") == 0)
			{
				sscanf(commands[2],"%lf",&App::age);
			}
			else
			if(strcmp(commands[1],"geometry") == 0)
			{
				if(strcmp(commands[2],"cones") == 0)
				{
					App::geometryType = 1;
				}
				else
				if(strcmp(commands[2],"sector") == 0)
				{
					sscanf(commands[3],"%d",&App::onlySectorNo);
				}
				else
				if(strcmp(commands[2],"coverSector") == 0)
				{
					sscanf(commands[3],"%d",&App::coverSector);
					printf("coverSector: %d\n", App::coverSector);
				}
			}
			else
			if(strcmp(commands[1],"source") == 0)
			{
				if(strcmp(commands[2],"sphere") == 0)
				{
					App::isIrregularSource = false;
				}
				else
				if(strcmp(commands[2],"irregular") == 0)
				{
					App::isIrregularSource = true;
				}
			}
		}
		else
		if(strcmp(commands[0],"apperture") == 0)
		{
			printf("aperture\n");
			if(strcmp(commands[1],"width") == 0)
			{
				if(strcmp(commands[2],"full") == 0)
				{
					App::phi_width = 2.0*M_PI;
					App::theta_width = M_PI;
					App::isSpacial = true;
				}
			}
			else if(strcmp(commands[1],"add") == 0)
			{
				printf("Apperture added\n");
				double phi, theta, phi_width, theta_width;
				sscanf(commands[2],"%lf",&phi);
				sscanf(commands[3],"%lf",&theta);
				sscanf(commands[4],"%lf",&phi_width);
				sscanf(commands[5],"%lf",&theta_width);
				printf("addApp itself: %lf %lf\n", phi_width, theta_width);
				fflush(stdout);
				App::addApperture(phi,theta,phi_width,theta_width);
				printf("addApperture called\n");
				fflush(stdout);
			}
			printf("Finish adding app!");
			fflush(stdout);
		}
		else
		if(strcmp(commands[0],"isophote") == 0)
		{
			if(strcmp(commands[1],"add") == 0)
			{
				printf("Isophote added\n");
				double enC, enW;
				sscanf(commands[2],"%lf",&enC);
				sscanf(commands[3],"%lf",&enW);
				CIsophotes::addIsophote(ISOPHOTE_CONT, enC, enW);
			}
			else
			if(strcmp(commands[1],"line") == 0)
			{
				printf("Isophote added\n");
				char chlab[14];
				//sscanf(commands[2],"%11[0-9a-zA-Z ]",&chlab);
				sprintf(chlab, "%s", commands[2]);
				printf("CHLAB: %s\n");
				CIsophotes::addIsophote(ISOPHOTE_LINE, 0,0)->setLine(chlab);
			}
			else
			if(strcmp(commands[1],"ratio") == 0)
			{
				printf("Isophote added\n");
				char chlab[14];
				sprintf(chlab, "%s", commands[2]);
				//sscanf(commands[2],"%11[0-9a-zA-Z ]",&chlab);
				printf("CHLAB: %s\n", chlab);
				Isophote* isp = CIsophotes::addIsophote(ISOPHOTE_RATIO, 0,0)->setLine(chlab);
				if(strcmp(commands[3],"to") == 0)
				{
					char chlabTo[14];
					sprintf(chlabTo, "%s", commands[4]);
					isp->setTo(chlabTo);
				}
				else
				{
					char chlabTo[14];
					sprintf(chlabTo, "TOTL  4861A");
					isp->setTo(chlabTo);
				}
			}
		}
		else
		if(strcmp(commands[0],"print") == 0)
		{
			printf("print\n");
			if(strcmp(commands[1],"directory") == 0)
			{
				sscanf(commands[2],"%s",&App::output_dir);
				sscanf(commands[2],"%s",&App::output_dir_in);
			}
			else
			if(strcmp(commands[1],"bands") == 0)
			{
				App::printBands = true;
			}
			else
			if(strcmp(commands[1], "fluxes") == 0)
			{
				sscanf(commands[2], "%s", &App::fluxesOutput);
			}
			else
			if(strcmp(commands[1], "loglevel") == 0)
			{
				if(strcmp(commands[2], "debug") == 0)
				{
					CDebugger::init(2);
				}
				else
				if(strcmp(commands[2], "log") == 0)
				{
					CDebugger::init(1);
				}
				else
				if(strcmp(commands[2], "warn") == 0)
				{
					CDebugger::init(0);
				}
								
			}
		}
		else
		if(strcmp(commands[0],"input") == 0)
		{
			printf("input\n");
			if(strcmp(commands[1],"location") == 0 || strcmp(commands[1],"directory") == 0) //for compability with DiffRay v2.0
			{
				sscanf(commands[2],"%s",&App::input_dir);
			}
			else
			if(strcmp(commands[1],"database") == 0) //for compability with DiffRay v2.0
			{
				sscanf(commands[2],"%s",&App::data_base);
			}
			else
			if(strcmp(commands[1],"bands") == 0) //for compability with DiffRay v2.0
			{
				sscanf(commands[2],"%s",&App::data_bands);
			}
			printf("DIR: %s\n",App::input_dir);
		}
		else
		if(strcmp(commands[0],"integration") == 0)
		{
			if(strcmp(commands[1],"mode") == 0)
			{
				if(strcmp(commands[2],"otw") == 0)
				{
					App::integration_mode = 0;
				}
				else
				{
					App::integration_mode = 1;
				}
			}
			else
			if(strcmp(commands[1],"maxiterations") == 0)
			{
				sscanf(commands[2],"%d",&App::maxiterations);
			}
			else
			if(strcmp(commands[1],"precision") == 0)
			{
				sscanf(commands[2],"%lf",&App::precision);
			}
			else
			if(strcmp(commands[1],"predictive") == 0)
			{
				printf("PREDICTIVE!\n");
				if(strcmp(commands[2],"on") == 0)
				{
					printf("on\n");
					App::usePredictiveMode = true;
				}
				else
				{
					App::usePredictiveMode = false;
				}
			}
		}
		if(strcmp(commands[0],"calculation") == 0)
		{
			if(strcmp(commands[1],"opacity") == 0)
			{
				if(strcmp(commands[2],"on") == 0)
				{
					App::CalcOpac = true;
				}
				else
				{
					App::CalcOpac = false;
				}
			}
			else
			if(strcmp(commands[1],"continuum") == 0)
			{
				if(strcmp(commands[2],"on") == 0)
				{
					App::CalcCont = true;
				}
				else
				{
					App::CalcCont = false;
				}
			}
			else if(strcmp(commands[1],"abundance") == 0)
			{
				if(strcmp(commands[2],"on") == 0)
				{
					App::CalcAbund = true;
				}
				else
				{
					App::CalcAbund = false;
				}
			}
			else
			if(strcmp(commands[1],"overview") == 0)
			{
				if(strcmp(commands[2],"on") == 0)
				{
					App::CalcOverviews = true;
				}
				else
				{
					App::CalcOverviews = false;
				}
			}
			else
			if(strcmp(commands[1],"grains") == 0)
			{
				if(strcmp(commands[2],"on") == 0)
				{
					App::CalcGrainTemp = true;
				}
				else
				{
					App::CalcGrainTemp = false;
				}
			}
			else
			if(strcmp(commands[1],"bands") == 0)
			{
				if(strcmp(commands[2],"on") == 0)
				{
					App::CalcIRBands = true;
					App::printBands = true;
				}
				else
				{
					App::CalcIRBands = false;
					App::printBands = false;
				}
			}
		}
		
		commands = CReader::read_commands(F);
	};

	printf("preDONE\n");
	fflush(stdout);
	fclose(F);

	App::setRay(App::phi, App::theta);

	printf("DONE\n");
	return true;
}
