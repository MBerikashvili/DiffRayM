#pragma once
#include <algorithm>

struct AngleStep {
	double phi, theta, dphi, dtheta, Fi, dS;
	bool isPredicted;
	AngleStep(double cphi, double ctheta, double cdphi, double cdtheta)
	{
		phi = cphi;
		theta = ctheta;
		dphi = cdphi;
		dtheta = cdtheta;
		dS = cdphi * cdtheta;
		Fi = 0.;
		isPredicted = false;
	}

	AngleStep& operator=(const AngleStep& a)
	{
	    phi=a.phi;
	    theta=a.theta;
	    dphi=a.dphi;
	    dtheta=a.dtheta;
	    Fi = a.Fi;
	    isPredicted = a.isPredicted;
	    return *this;
	}
	
	double left()
	{
		return phi - dphi/2;
	}

	double right()
	{
		return phi + dphi/2;
	}

	double top()
	{
		return theta + dtheta/2;
	}

	double bottom()
	{
		return theta - dtheta/2;
	}

	bool contains(double phi, double theta)
	{
		if(phi < this->left()) return false;
		if(phi > this->right()) return false;
		if(theta > this->top()) return false;
		if(theta < this->bottom()) return false;
		return false;
	}
	double intersectionSquare(double phi2, double theta2, double dphi2, double dtheta2)
	{
		double left = std::max(this->left(), phi2 - dphi2/2);
		double bottom = std::max(this->bottom(), theta2 - dtheta2/2);
		double right = std::min(this->right(), phi2 + dphi2/2);
		double top = std::min(this->top(), theta2 + dtheta2/2);
		
		double width = right - left;
		double height = top - bottom;

		if(width < 0)
		    return 0;

		if(height < 0)
		    return 0;

		return width * height;
	} 
};