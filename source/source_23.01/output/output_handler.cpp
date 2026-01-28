#include "output_handler.h"
#include "prt_lines.h"
#include "prt_continuum.h"
#include "prt_isophote.h"
#include "../spectra/isophote.h"


bool OutputHandler::lifr()
{
	PrtLifr *lifr = new PrtLifr();
	lifr->print();
	delete lifr;
	return true;
}

bool OutputHandler::llum()
{
	PrtLlum *llum = new PrtLlum();
	llum->print();
	delete llum;
	return true;
}

bool OutputHandler::linesarr()
{
	PrtLinesarr *lins = new PrtLinesarr();
	lins->print();
	delete lins;
	return true;
}

//!!BM bool OutputHandler::contarr(char *fname)
bool OutputHandler::contarr()
{
//!!BM UPD:
//!!	char fname[255];
//	sprintf(fname,"contarr_Sector%d.dat",(App::onlySectorNo+1));
//	PrtContarr *c = new PrtContarr(fname);
//!!!END OF BM UPD
	PrtContarr *c = new PrtContarr();
	c->print();
	delete c;
	return true;
}

bool OutputHandler::spectra()
{
	PrtSpectra *c = new PrtSpectra();
	c->print();
	delete c;
	return true;
}

bool OutputHandler::isophotes()
{
	for(int i=0; i<CIsophotes::nIsophotes;i++)
	{
	    char *fnm;
	    fnm = (char*)malloc(255);
	    sprintf(fnm, "isophote_%d.dat",i);
	    PrtIsophote *c = new PrtIsophote(fnm, i);
	    c->print();
	    delete c;
	}
	PrtIsophote::plot();
	return true;
}