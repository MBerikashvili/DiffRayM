#include "basics.h"
#include "geom3d.h"
#include "solver.h"
#include "geometry.h"
#include "integration.h"
#include "app.h"
#include "const.h"
#include "output/debugger.h"

int CSolver::npoints = 0;
intersection CSolver::points[5000];
TPath CSolver::paths[5000];

int normalizeSector(int phs)
{
	if(phs>=3*App::nSectors)
		phs = 4*App::nSectors-1-phs;
	else
	if(phs>=2*App::nSectors)
		phs = phs - 2*App::nSectors;
	else
	if(phs>=App::nSectors)
		phs = 2*App::nSectors-1-phs;
	
	if(phs<0)
		phs = 0;

	return phs;	
}

int normalizeCone(int phs)
{
	if(phs >= 2*App::nSectors)
	{
		phs = App::nSectors - 1;
	}
	else
	if(phs >= App::nSectors)
	{
		phs = phs - App::nSectors;
	}
	else
	{
		phs = App::nSectors - 1 - phs;
	}
	if(phs < 0)
	{
		phs = 0;
	}
	return phs;
}

int CSolver::getPointsCones()
{
	//CDebugger::debug("GETTING POINTS\n");
	//For the first time just use demo-ray
	TRay ray = App::rayIntegration;
	ray.angle.y = App::rayIntegration.angle.y / (cos(App::rayIntegration.angle.z) + SMALL_NUMBER);
	int npoints = 0;

	intersection curpoint;
	curpoint.point = App::rayIntegration.start;
	curpoint.sec_p = 0;
	curpoint.sec_n = 0;
	curpoint.lay_p = -2;
	curpoint.lay_n = -2;
	curpoint.is = 0;
	curpoint.phi = 0;
	curpoint.R = 0.0;

	CSolver::points[npoints] = curpoint;

	npoints++;
	
	//CDebugger::debug("initing first point\n");
	FILE *FST;

	char fname2[255];

	sprintf(fname2, "%s/points_stat.dat",App::output_dir);

	if(App::isStatMode)
	{
		

		
		FST = fopen(fname2,"w+");
		
		fclose(FST);
	}
	
	vector3 *iPoint;
	iPoint = new vector3[2];
	//get all points from spheres
	for(int is=0; is<2*App::nSectors; is++)
	{
		int phys_sec = normalizeCone(is);

		//printf("%d - > %d\n", is, phys_sec);
		
		int geom_sec = phys_sec;

		int nlayers = CGeometry::nRadiuses[geom_sec];

		//printf("R_out[%d] = %le(%d)\n", geom_sec, CGeometry::outer_radius[geom_sec][nlayers-1],nla7yers);
		//CDebugger::debug("Layer intersections for %d [%d]\n", is, npoints);
	
		double sect_top  = M_PI/2.0 - is*M_PI/(2*App::nSectors);
		double sect_bottom = M_PI/2.0 - (is+1)*M_PI/(2*App::nSectors);
		
		for(int ilayer=0;ilayer<nlayers;ilayer++)
		{
			long double Radius = CGeometry::outer_radius[geom_sec][ilayer];
			
			intersectSphere(ray,Radius,iPoint);

			
			for(int k=0;k<2;k++)
			{

				double theta = getTheta(iPoint[k]);
				//phi = normalize_phi(phi);
				
				if(App::isStatMode)
				{	
					FST = fopen(fname2,"a+");
					
					fprintf(FST, "Sector: %d; (%d) Layer: %d; theta: %lf (%lf - %lf); [%Le;%Le;%Le] |%le|\n", geom_sec, is, ilayer, theta, sect_bottom, sect_top, iPoint[k].x, iPoint[k].y, iPoint[k].z, module(iPoint[k]));

					fclose(FST);

				}

				if(module(iPoint[k])<1.e+80)
				{
					/*
					if(fabs((module(iPoint[k]) - Radius)/Radius) > 1.e-1)
					{
						printvec(iPoint[k]);
						printf("A: %Le; R=%le\n", module(iPoint[k]), Radius);
						printvec(App::rayIntegration.start);
						printvec(App::rayIntegration.angle);
						exit(1);
					
					}*/
					/*if(fabs(ray.angle.z + 8.88e-7)<5.e-8)
					{
						if(Radius < 1.75e+19)
						{
							printf("%d Theta is %le; should be between %le %le\n", k, theta, sect_bottom, sect_top);
						}
					}*/
					if(checkTopAngleWithin(sect_bottom, sect_top, theta))
					{
						/*if(geom_sec > 0)
						{
						printf("Intersected theta: %le; %d; %d\n",theta,is,geom_sec);
						printvec(iPoint[k]);
						exit(1);
						}*/
						
						intersection curpoint;
						curpoint.point = iPoint[k];
						curpoint.sec_p = geom_sec;
						curpoint.sec_n = geom_sec;
						curpoint.lay_p = ilayer-1;
						curpoint.lay_n = ilayer;
						curpoint.is = is;
						curpoint.phi = getPhi(iPoint[k]);
						curpoint.R = calc_dist(iPoint[k],App::rayIntegration.start);
						CSolver::points[npoints] = curpoint;
						/*if(fabs(ray.angle.z + 8.88e-7)<5.e-8)
						{
							if(Radius < 1.75e+19)
							{
								printf("%d %d %d Theta is %le; Accepting! r=%Le\n", k, geom_sec, ilayer, theta, sect_bottom, sect_top, curpoint.R - pow(10.0, 25.017));
								printf("nPointsNow: %d\n",npoints);
							}
						}*/
						npoints++;
					}
					
				}
			}
		
			//delete[] iPoint;
		
		}
		//CDebugger::debug("Cone intersections for %d [%d]\n", is, npoints);
	
		intersectRayWithCone(ray,sect_bottom,iPoint);
		for(int k = 0; k < 2; k++)
		{
			double is_theta = getTheta(iPoint[k]);
			if(App::isStatMode)
				{	
					FST = fopen(fname2,"a+");
					
					fprintf(FST, "Cone Sector: %d (%d); theta: %lf (%lf - %lf); [%Le;%Le;%Le] |%le|\n", normalizeCone(is), is, is_theta, sect_bottom, sect_top, iPoint[k].x, iPoint[k].y, iPoint[k].z, module(iPoint[k]));

					fclose(FST);

				}
			if(module(iPoint[k])>=1.e+80)
			{
				continue;
			}	
			
			if(fabs(is_theta-sect_bottom)<=1.e-6)
			{
				intersection curpoint;
				curpoint.point = iPoint[k];
				curpoint.sec_p = normalizeCone(is);
				curpoint.sec_n = normalizeCone(is-1);
				double pointR = module(iPoint[k]);
				int clayer = -5;
				//printf("R: %le\n", pointR);
				for(int ilayer=0;ilayer<nlayers;ilayer++)
				{
					double Radius = CGeometry::outer_radius[geom_sec][ilayer];
					if(pointR<Radius)
					{
						clayer = ilayer-1;
						break;
					}
				}

				

				if(clayer>-5)
				{
					curpoint.lay_p = clayer;
					curpoint.lay_n = clayer;
					curpoint.R = calc_dist(iPoint[k], App::rayIntegration.start);
					CSolver::points[npoints] = curpoint;

					npoints++;
				}
			}
		}
	}
	delete[] iPoint;
	CSolver::npoints = npoints-1;
	return npoints;
}


int CSolver::getPointsSectors()
{
	//printf("GETTING POINTS\n");
	//For the first time just use demo-ray
	TRay ray = App::rayIntegration;
	//ray.angle.y += 0.2;
	int npoints = 0;
	
	intersection curpoint;
	curpoint.point = App::rayIntegration.start;
	curpoint.sec_p = 0;
	curpoint.sec_n = 0;
	curpoint.lay_p = -2;
	curpoint.lay_n = -2;
	curpoint.is = 0;
	curpoint.phi = 0;
	curpoint.R = 0.0;

	CSolver::points[npoints] = curpoint;

	npoints++;
	//printvec(App::rayToObj.start);
	//printvec(App::rayToObj.angle);


	FILE *FST;

	char fname2[255];

	sprintf(fname2, "%s/points_stat.dat",App::output_dir);

	if(App::isStatMode)
	{
		

		
		FST = fopen(fname2,"w+");
		
		fclose(FST);
	}
	
	vector3 *iPoint;
	iPoint = new vector3[2];
	//get all points from spheres
	for(int is=0; is<4*App::nSectors; is++)
	{
		int phys_sec = normalizeSector(is);

		//printf("%d - > %d\n", is, phys_sec);
		
		int geom_sec = phys_sec;

		int nlayers = CGeometry::nRadiuses[geom_sec];

		//printf("R_out[%d] = %le(%d)\n", geom_sec, CGeometry::outer_radius[geom_sec][nlayers-1],nla7yers);

		double sect_left  = is*M_PI/(2*App::nSectors) - M_PI/(4*App::nSectors);
		double sect_right = is*M_PI/(2*App::nSectors) + M_PI/(4*App::nSectors);
		
		double left  = normalize_phi(sect_left);
		double right = normalize_phi(sect_right);

		for(int ilayer=0;ilayer<nlayers;ilayer++)
		{
			double Radius = CGeometry::outer_radius[geom_sec][ilayer];
			
			intersectSphere(ray,Radius,iPoint);

			
			for(int k=0;k<2;k++)
			{

				double phi = getPhi(iPoint[k]);

				phi = normalize_phi(phi);
							
				if(App::isStatMode)
				{	
					FST = fopen(fname2,"a+");
					
					fprintf(FST, "Sector: %d; Layer: %d; phi: %lf (%lf - %lf); [%Le;%Le;%Le] |%le|\n", is, ilayer, phi, left, right, iPoint[k].x, iPoint[k].y, iPoint[k].z, module(iPoint[k]));

					fclose(FST);

				}

				if(module(iPoint[k])<1.e+80)
				{

					
					if(checkAngleWithin(left, right, phi))
					{
						intersection curpoint;
						curpoint.point = iPoint[k];
						curpoint.sec_p = geom_sec;
						curpoint.sec_n = geom_sec;
						curpoint.lay_p = ilayer-1;
						curpoint.lay_n = ilayer;
						curpoint.is = is;
						curpoint.phi = getPhi(iPoint[k]);
						curpoint.R = calc_dist(iPoint[k],App::rayIntegration.start);
						CSolver::points[npoints] = curpoint;

						npoints++;
					}
					
				}
			}
		
			//delete[] iPoint;
		
		}
		
		vector3 iPointS = intersectRayWithPhi(ray,left);
		double is_phi = getPhi(iPointS);
		
		if(fabs(is_phi-left)<=1.e-12)
		{
			intersection curpoint;
			curpoint.point = iPointS;
			curpoint.sec_p = normalizeSector(is);
			curpoint.sec_n = normalizeSector(is-1);

			double pointR = module(iPointS);
			int clayer = -5;
			//printf("R: %le\n", pointR);
			for(int ilayer=0;ilayer<nlayers;ilayer++)
			{
				double Radius = CGeometry::outer_radius[geom_sec][ilayer];
				if(pointR<Radius)
				{
					clayer = ilayer-1;
					break;
				}
			}

			if(clayer>-5)
			{
				curpoint.lay_p = clayer;
				curpoint.lay_n = clayer;
				curpoint.R = calc_dist(iPointS, App::rayIntegration.start);
				CSolver::points[npoints] = curpoint;

				npoints++;
			}
		}
	}
	delete[] iPoint;
	CSolver::npoints = npoints-1;
	return npoints;
}

int CSolver::getPoints()
{
	if(App::geometryType == 0)
	{
		CSolver::getPointsSectors();
	}
	else
	{
		CSolver::getPointsCones();
	}
	//CDebugger::debug("intersections got: %d", CSolver::npoints);
	FILE *FP;

	//sorting
	int i,j;
	
	intersection temp;

	for(i=1;i<= CSolver::npoints + 1;i++)
	{
	 	for(j=0;j<CSolver::npoints+1-i;j++)
	 	{
	 		if(CSolver::points[j].R>CSolver::points[j+1].R)
	 		{
	 			temp=CSolver::points[j];
	 			CSolver::points[j]=CSolver::points[j+1];
	 			CSolver::points[j+1]=temp;
	 		}
	 	}
	}

	//CDebugger::debug("intersections sorted2: %d", CSolver::npoints);
	

	for(i=0; i<CSolver::npoints;i++)
	{
		TPath cpath;
		/*if(i > 3000 || i < 10) {
		    CDebugger::debug("start point %d/%d", i, CSolver::npoints);
		}*/
		if(i<CSolver::npoints-1 && (CSolver::points[i].lay_p<=CSolver::points[i+1].lay_p && CSolver::points[i].lay_p>-2))
		{
			cpath.layer = CSolver::points[i+1].lay_p;
			cpath.sector = CSolver::points[i+1].sec_p;
		}
		else
		{
			cpath.layer = CSolver::points[i].lay_p;
			cpath.sector = CSolver::points[i].sec_p;
		}
		/*
		if(i > CSolver::npoints - 3000) {
			CDebugger::debug("%d - (%d %d)", i, CSolver::points[i].sec_p, CSolver::points[i].lay_p);
		}*/
		cpath.path = calc_dist(CSolver::points[i].point, CSolver::points[i+1].point);
		
		CSolver::paths[i] = cpath;
	}

	//CDebugger::debug("Paths calculated: %d", CSolver::npoints);
	

	if(App::isStatMode)
	{
		char fname[255];

		//printvec(ray.start);
		//printvec(ray.angle);

		sprintf(fname, "%s/intersections.dat",App::output_dir);
		FP = fopen(fname,"w+");
		for(i=0;i<CSolver::npoints;i++)
		{
			fprintf(FP,"Sector: %d/%d (%d - %lf); Layer: %d/%d; Point: %Le;%Le;%Le; R=%le\n", CSolver::points[i].sec_p, CSolver::points[i].sec_n,CSolver::points[i].is,CSolver::points[i].phi, CSolver::points[i].lay_p, CSolver::points[i].lay_n, CSolver::points[i].point.x, CSolver::points[i].point.y, CSolver::points[i].point.z,CSolver::points[i].R);
			//printvec(CSolver::points[i].point);
		}
		fclose(FP);
	}
	return 1;
}