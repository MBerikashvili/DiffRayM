#include "basics.h"
#include "geom3d.h"
#include "app.h"
#include "mathutl.h"
#include "const.h"

void printvec(vector3 a)
{
	printf("[%Le,%Le,%Le]\n", a.x, a.y, a.z);
}

double module(vector3 a)
{
	return sqrt(a.x*a.x+a.y*a.y+a.z*a.z);
}

double getPhi(vector3 point)
{
	long double dxy = sqrt(point.x*point.x+point.y*point.y);
	
	if(dxy<1.e-40*point.x || dxy == 0.0)
		return 0.0;
	//printf("dxy: %Le, arg: %Le\n", dxy, point.x/dxy);
	//printvec(point);
	if(point.y>0)
		return acos(point.x/dxy);
	else
		return 2.0*M_PI -  acos(point.x/dxy);
}

double getTheta(vector3 point)
{
	double dxy = module(point);
	if(dxy>1.e-40*fabs(point.z))
		return asin(point.z/dxy);
	else
		return M_PI*sign(point.z)/2.0;
}

//a = (dr, dphi, dtheta)

vector3 makeOrt(double phia, double theta)
{
	vector3 rsv;
	rsv.x = cos(phia)*cos(theta);
	rsv.y = sin(phia)*cos(theta);
	rsv.z = sin(theta);
	
	return rsv;
}

vector3 spherical2desc(vector3 sph)
{
	vector3 rsv;
	rsv.x = sph.x*cos(sph.y)*cos(sph.z);
	rsv.y = sph.x*sin(sph.y)*cos(sph.z);
	rsv.z = sph.x*sin(sph.z);
	return rsv;
}

vector3 rotate(vector3 resv, vector3 a)
{

	double r = module(resv);
	double phia = getPhi(resv);
	double theta = getTheta(resv);
	if(r < SMALL_NUMBER)
		return resv;
	//printvec(resv);
	//printf("R=%le; phi=%le\n",r,phia);
	//exit(1);
	vector3 rsv;
	rsv.x = (r+a.x)*cos(phia+a.y)*cos(theta+a.z);
	rsv.y = (r+a.x)*sin(phia+a.y)*cos(theta+a.z);
	rsv.z = (r+a.x)*sin(theta+a.z);
	
	return rsv;
}

vector3 normalize(vector3 p)
{
	double mod = module(p);
	return vector3(p.x/mod,p.y/mod,p.z/mod);
}

vector3 rotateOverAxis(vector3 p, vector3 axis, double angle)
{
	double co = cos(angle);
	double si = sin(angle);
	double m_co = 1-cos(angle);

	vector3 a = normalize(axis);
	vector3 result;
	result.x = p.x*(co+m_co*a.x*a.x) + p.y*(m_co*a.x*a.y - si*a.z) + p.z*(m_co*a.x*a.z + si*a.y);
	result.y = p.x*(m_co*a.x*a.y + si*a.z) + p.y*(co+m_co*a.y*a.y) + p.z*(m_co*a.z*a.y - si*a.x);
	result.z = p.x*(m_co*a.z*a.x - si*a.y) + p.y*(m_co*a.y*a.z + si*a.x) + p.z*(co+m_co*a.z*a.z);
	return result;
}

TRay createRay(vector3 start, vector3 angle)
{
	TRay result = TRay(start, angle);
	
	return result;
}

TRay rotateRay(TRay initial, vector3 angle)
{
	//startpoint rotation
	vector3 start2 = rotate(initial.start,angle);

	//get any 2 points of ray;
	angle.z = -angle.z;
	vector3 angle2 = initial.angle + angle;

	return TRay(start2,angle2);
}

vector3* intersectSphere(TRay ray, long double radius, vector3 res[2])
{
	//vector3 *res;
	//res = new vector3[2];
	//square root solver
	long double b = 2*(ray.start.x*cos(ray.angle.y)*cos(ray.angle.z)+ray.start.y*sin(ray.angle.y)*cos(ray.angle.z)+ray.start.z*sin(ray.angle.z));
	long double c = pow(ray.start.x,2)+pow(ray.start.y,2)+pow(ray.start.z,2)-radius*radius;
	long double D = b*b-4*c;

	//printvec(ray.start);
	//printvec(ray.angle);
	//printf("D=%Le; R=%Le; b=%Le; c=%Le;\n",D,radius, b, c);
	//if(D < 0)
	//    exit(1);
	if(App::isStatMode)
	{
		char fname2[255];

		sprintf(fname2, "%s/points_stat.dat",App::output_dir);

		FILE *FST;
		
		FST = fopen(fname2,"a+");
		
		fprintf(FST, "Radius: %le; Ray: start [%Le;%Le;%Le];[%Le;%Le] b=%Le;c=%Le; D=%Le\n",radius, ray.start.x, ray.start.y, ray.start.z, ray.angle.y, ray.angle.z, b, c, D);

		fclose(FST);

	}
/*
	if(fabs(ray.angle.z + 8.88e-7)<5.e-8)
	{
		if(radius < 1.75e+19)
		{
			printf("INTERSECT %Le with D = %Le; (b=%Le;c=%Le)\n",radius,D,b,c);
		}
	}*/
	res[0] = vector3(1.e+80,1.e+80,1.e+80);
	res[1] = vector3(1.e+80,1.e+80,1.e+80);
	
	//exit(1);
	//printf("DISQ: %le\n", D);
	if(D>=0.0)
	{
		long double r1 = (-b + sqrt(D))/(2.0);
		long double r2 = (-b - sqrt(D))/(2.0);
		
		//printf("R1: %le\n", r1);
		//printf("R2: %le\n", r2);
		
		if(r1>0)
		{
			res[0] = vector3(ray.start.x+r1*cos(ray.angle.y)*cos(ray.angle.z),ray.start.y+r1*sin(ray.angle.y)*cos(ray.angle.z),ray.start.z+r1*sin(ray.angle.z));
		}

		if(r2>0)
		{
			res[1] = vector3(ray.start.x+r2*cos(ray.angle.y)*cos(ray.angle.z),ray.start.y+r2*sin(ray.angle.y)*cos(ray.angle.z),ray.start.z+r2*sin(ray.angle.z));
		}
	}
	/*printf("VEC1: ");
	printvec(res[0]);
	printvec(res[1]);*/
	return res;
}


vector3* intersectRayWithCone(TRay ray, double theta, vector3 res[2])
{
	//vector3 *res;
	//res = new vector3[2];
	//square root solver

	long double b = 2*(ray.start.x*cos(ray.angle.y)*cos(ray.angle.z)+ray.start.y*sin(ray.angle.y)*cos(ray.angle.z))*pow(sin(theta),2.0)-2*ray.start.z*sin(ray.angle.z)*pow(cos(theta),2.0);
	long double c = (pow(ray.start.x,2.0)+pow(ray.start.y,2.0))*pow(sin(theta),2.0)-pow(ray.start.z,2.0)*pow(cos(theta),2.0);	
	long double a = (pow(cos(ray.angle.y)*cos(ray.angle.z),2.0)+pow(sin(ray.angle.y)*cos(ray.angle.z),2.0))*pow(sin(theta),2.0)-pow(sin(ray.angle.z),2.0)*pow(cos(theta),2.0);
	
	res[0] = vector3(1.e+80,1.e+80,1.e+80);
	res[1] = vector3(1.e+80,1.e+80,1.e+80);
	
	if(fabs(a) < 1.e-50)
	{
		return res;
	}
	b = b/a;
	c = c/a;
	long double D = b*b-4*c;

	if(App::isStatMode)
	{
		char fname2[255];

		sprintf(fname2, "%s/points_stat.dat",App::output_dir);

		FILE *FST;
		
		FST = fopen(fname2,"a+");
		
		fprintf(FST, "Theta: %le; Ray: [%Le;%Le;%Le]; b=%Le;c=%Le; %Le - %Le = D=%Le\n",theta, ray.angle.x, ray.angle.y, ray.angle.z, b, c, b*b, 4*c, D);
		
		fclose(FST);

	}
	//printf("Theta: %le; Ray: [%Le;%Le;%Le]; b=%Le;c=%Le; %Le - %Le = D=%Le\n",theta, ray.angle.x, ray.angle.y, ray.angle.z, b, c, b*b, 4*c, D);

	//exit(1);
	//printf("DISQ: %le\n", D);
	if(D>=0.0)
	{
		double r1 = (-b + sqrt(D))/(2.0);
		double r2 = (-b - sqrt(D))/(2.0);
		
		//printf("R1: %le\n", r1);
		//printf("R2: %le\n", r2);
		
		if(r1>0)
		{
			res[0] = vector3(ray.start.x+r1*cos(ray.angle.y)*cos(ray.angle.z),ray.start.y+r1*sin(ray.angle.y)*cos(ray.angle.z),ray.start.z+r1*sin(ray.angle.z));
		}

		if(r2>0)
		{
			res[1] = vector3(ray.start.x+r2*cos(ray.angle.y)*cos(ray.angle.z),ray.start.y+r2*sin(ray.angle.y)*cos(ray.angle.z),ray.start.z+r2*sin(ray.angle.z));
		}
	}
	/*printf("VEC1: ");
	printvec(res[0]);
	printvec(res[1]);*/
	return res;
}

vector3 multiply(double M, vector3 v)
{
    return vector3(M*v.x, M*v.y, M*v.z);
}


TPlane createPlaneFromPoints(vector3 M1, vector3 M2, vector3 M3)
{
	TPlane result;

	result.A = (M2.y-M1.y)*(M3.z-M1.z) - (M3.y-M1.y)*(M2.z-M1.z);
	result.B = (M2.z-M1.z)*(M3.x-M1.x) - (M3.z-M1.z)*(M2.x-M1.x);
	result.C = (M2.x-M1.x)*(M3.y-M1.y) - (M3.x-M1.x)*(M2.y-M1.y);
	result.D = -1*(M1.x*result.A + M1.y*result.B+M1.z*result.C);

	return result;
}

TPlane getPlaneByAxisAndPhi(vector3 axis, double phi)
{
	vector3 point1 = vector3(0,0,0);
	vector3 point2 = axis;
	double theta = getTheta(axis);
	vector3 point3;

	if(theta<M_PI/4.0)
		point3 = rotate(axis,vector3(0,0,M_PI/4.0));
	else
		point3 = rotate(axis,vector3(0,0,-M_PI/4.0));

	//printvec(point3);

	point3 = rotateOverAxis(point3,axis,phi);

	//printf("rotated:\n");
	//printvec(point3);

	return createPlaneFromPoints(point1,point2,point3);

}

void printplane(TPlane P)
{
	printf("Plane: %le*x+%le*y+%le*z+%le = 0\n", P.A,P.B,P.C,P.D);
}


THalfplane create(TPlane P, vector3 point1, int side_des)
{
	//side_des is a
	THalfplane result;
	result.plane = P;
	result.lpoint = point1;
	result.side_des = side_des;
	return result;
}


THalfplane createFromAngles(double axis_phi, double axis_theta, double rot_phi)
{
	THalfplane result;
	vector3 axis = makeOrt(axis_phi,axis_theta);
	result.plane = getPlaneByAxisAndPhi(axis,rot_phi);
	result.lpoint = axis;
	result.side_des = sign(acos(rot_phi));
	return result;
}

vector3 intersectHalfplane(TRay ray, THalfplane sector)
{

}

//Closer to real geometry of object:
//Center of object always at zero
//So in fact half-plane is only defined by phi

TPlane makeCenteredByPhi(double phi)
{
	return TPlane(sin(phi),-cos(phi),0.0,0.0);
}

vector3 intersectRayWithPhi(TRay ray, double phi)
{
	vector3 axis = makeOrt(0.0, M_PI/2.0);
	TPlane sector = makeCenteredByPhi(phi);
	double kx = cos(ray.angle.y)*cos(ray.angle.z);
	double ky = sin(ray.angle.y)*cos(ray.angle.z);
	double kz = sin(ray.angle.z);

	double zn = (sector.A*kx+sector.B*ky);
	
	vector3 point;

	if(fabs(zn)<=1.e-80)
	{
		point = vector3(1.e+80,1.e+80,1.e+80);
	}
	else
	{
		double t = -(sector.A*ray.start.x+sector.B*ray.start.y)/zn;

		if(t>0)
		{
			//printf("t=%le\n", t);
			point = vector3(ray.start.x+kx*t,ray.start.y+ky*t,ray.start.z+kz*t);
			double oPhi = getPhi(point);

			if(sign(acos(phi)) != sign(acos(oPhi)) || sign(asin(phi)) != sign(asin(oPhi)))
			{
				point = vector3(1.e+80,1.e+80,1.e+80);
			}
		}
		else
		{
			point = vector3(1.e+80,1.e+80,1.e+80);
		}
	}

	//printvec(point);
	return point;
}

double normalize_phi(double phi)
{

	while(phi<0)
	{
		phi += 2*M_PI;
	}

	while(phi>2*M_PI)
	{
		phi -= 2*M_PI;
	}

	return phi;

}

long double calc_dist(vector3 a, vector3 b)
{
	return sqrt(pow(a.x-b.x,2.0)+pow(a.y-b.y,2.0)+pow(a.z-b.z,2.0));
}


bool checkAngleWithin(double left, double right, double angle)
{
	if(right>left)
	{
		return (angle >= left && angle <= right);
	}
	else
	{
		//right += 2*M_PI;
		 if(angle >= left && angle <= right + 2*M_PI)
		 {
		 	return true;
		 }
		 else
		 {
		 	return (angle >= left - 2*M_PI && angle <= right);
		 }
	}
}

bool checkTopAngleWithin(double bottom, double top, double angle)
{
	if(top>bottom)
	{
		return (angle >= bottom && angle < top);
	}
	else
	{
		//right += 2*M_PI;
		 if(angle >= bottom && angle < top + 2*M_PI)
		 {
		 	return true;
		 }
		 else
		 {
		 	return (angle >= bottom - 2*M_PI && angle < top);
		 }
	}
}