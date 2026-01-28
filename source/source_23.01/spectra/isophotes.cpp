#include "isophote.h"

int CIsophotes::nIsophotes = 0;
Isophote *CIsophotes::isophotes[25];

Isophote* CIsophotes::addIsophote(int type, double enC, double enW)
    {
	if(nIsophotes > 24)
	    return NULL;

	isophotes[nIsophotes] = new Isophote(type, enC, enW);
	nIsophotes++;
	return isophotes[nIsophotes-1];
    }

int CIsophotes::initAll()
    {
	for(int i=0; i<nIsophotes;i++)
	{
		isophotes[i]->initSpectra();
	}
	return nIsophotes;
    }

int CIsophotes::resetApp()
    {
	for(int i=0; i<nIsophotes;i++)
	{
		isophotes[i]->initAngles();
	}
	return nIsophotes;
    }

int CIsophotes::addFlux(double phi, double theta, double dphi, double dtheta, double *fluxes, double *lines)
{
	for(int i=0; i<nIsophotes;i++)
	{
		if(isophotes[i]->isophoteType == ISOPHOTE_CONT)
			isophotes[i]->addFrom(phi, theta, dphi, dtheta, fluxes);
		else
		if(isophotes[i]->isophoteType == ISOPHOTE_LINE)
			isophotes[i]->addFrom(phi, theta, dphi, dtheta, lines);
		else
		if(isophotes[i]->isophoteType == ISOPHOTE_RATIO)
			isophotes[i]->addFrom(phi, theta, dphi, dtheta, lines);
		
	}
	return nIsophotes;
}