#pragma once
#include "basics.h"

struct vector3
{
	long double x;
	long double y;
	long double z;

	vector3(long double x=0., long double y=0., long double z=0.) 
        : x(x), y(y), z(z)
    {
    }

	vector3& operator=(const vector3& a)
    {
        x=a.x;
        y=a.y;
        z=a.z;
        return *this;
    }

    // addop. doesn't modify object. therefore const.
    vector3 operator+(const vector3& a) const
    {
        return vector3(a.x+x, a.y+y, a.z+z);
    }

    vector3 operator-(const vector3& a) const
    {
        return vector3(x-a.x, y-a.y, z-a.z);
    }

    vector3 operator-() const
    {
        return vector3(-x, -y, -z);
    }

    double operator*(const vector3 a)
    {
    	return a.x*x+a.y*y+a.z*z;
    }

    // equality comparison. doesn't modify object. therefore const.
    bool operator==(const vector3& a) const
    {
        return (x == a.x && y == a.y && z == a.z);
    }

    

};

vector3 multiply(double M, vector3 v);

void printvec(vector3 a);

double module(vector3 a);

double getPhi(vector3 point);

double getTheta(vector3 point);

//a = (dr, dphi, dtheta);

vector3 spherical2desc(vector3 sph);

vector3 rotate(vector3 resv, vector3 a);

vector3 normalize(vector3 p);

vector3 rotateOverAxis(vector3 p, vector3 axis, double angle);


struct TRay
{
	vector3 start;
	vector3 angle;

	TRay(vector3 start = vector3(0,0,0), vector3 angle = vector3())
		: start(start), angle(angle)
		{

		}
};

TRay createRay(vector3 start, vector3 angle);

TRay rotateRay(TRay initial, vector3 angle);

vector3* intersectSphere(TRay ray, long double radius, vector3 res[2]);

vector3* intersectRayWithCone(TRay ray, double theta, vector3 res[2]);


struct TPlane
{
	double A,B,C,D;

	TPlane(double A=0., double B=0., double C=0., double D=0.) 
        : A(A), B(B), C(C), D(D)
    {
    }
};

TPlane createPlaneFromPoints(vector3 M1, vector3 M2, vector3 M3);

TPlane getPlaneByAxisAndPhi(vector3 axis, double phi);

void printplane(TPlane P);

struct THalfplane
{
	TPlane    plane;
	vector3   lpoint;
	int side_des;
};

THalfplane create(TPlane P, vector3 point1, int side_des = 0);


THalfplane createFromAngles(double axis_phi, double axis_theta, double rot_phi);

vector3 intersectHalfplane(TRay ray, THalfplane sector);

//Closer to real geometry of object:
//Center of object always at zero
//So in fact half-plane is only defined by phi

TPlane makeCenteredByPhi(double phi);

vector3 intersectRayWithPhi(TRay ray, double phi);

double normalize_phi(double phi);

long double calc_dist(vector3 a, vector3 b);

bool checkAngleWithin(double left, double right, double angle);

bool checkTopAngleWithin(double bottom, double top, double angle);

struct Apperture
{
    double phi, theta, phi_width, theta_width;

    Apperture(double phi = 0, double theta = 0, double phi_width = 0, double theta_width = 0)
    : phi(phi),theta(theta),phi_width(phi_width),theta_width(theta_width)
    {

    }

    bool reset(double sphi = 0, double stheta = 0, double sphi_width = 0, double stheta_width = 0)
    {
        phi = sphi;
        theta = stheta;
        phi_width = sphi_width;
        theta_width = stheta_width;
	return true;
    }
};