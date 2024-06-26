/* ****************************************************************** **
**    OpenSees - Open System for Earthquake Engineering Simulation    **
**          Pacific Earthquake Engineering Research Center            **
**                                                                    **
**                                                                    **
** (C) Copyright 1999, The Regents of the University of California    **
** All Rights Reserved.                                               **
**                                                                    **
** Commercial use of this program without express permission of the   **
** University of California, Berkeley, is strictly prohibited.  See   **
** file 'COPYRIGHT'  in main directory for information on usage and   **
** redistribution,  and for a DISCLAIMER OF ALL WARRANTIES.           **
**                                                                    **
** Developed by:                                                      **
**   Frank McKenna (fmckenna@ce.berkeley.edu)                         **
**   Gregory L. Fenves (fenves@ce.berkeley.edu)                       **
**   Filip C. Filippou (filippou@ce.berkeley.edu)                     **
**                                                                    **
** ****************************************************************** */
//
// Written: MHS
// Created: 10/99
// Revision: A
//
// Description: This file contains the class definition for 
// TimeSeriesIntegrator.
//
#include <TimeSeriesIntegrator.h>
#if 0
#include <elementAPI.h>


void* OPS_ADD_RUNTIME_VPV(OPS_TrapezoidalTimeSeriesIntegrator);
void* OPS_ADD_RUNTIME_VPV(OPS_SimpsonTimeSeriesIntegrator);

void *
OPS_ADD_RUNTIME_VPV(OPS_TimeSeriesIntegrator)
{
    void *seriesIntegrator = 0;

    if (OPS_GetNumRemainingInputArgs() < 1) {
	opserr << "WARNING TimeSeriesIntegrator type is required\n";
	return 0;
    }

    const char* type = OPS_GetString();
    if (strcmp(type,"Trapezoidal") == 0) {
	seriesIntegrator = (TimeSeriesIntegrator*)OPS_CALL_RUNTIME_VPV(OPS_TrapezoidalTimeSeriesIntegrator);
		
    } else if (strcmp(type,"Simpson") == 0) {
	seriesIntegrator = (TimeSeriesIntegrator*)OPS_CALL_RUNTIME_VPV(OPS_SimpsonTimeSeriesIntegrator);
		
    } else {
	// type of load pattern type unknown
	opserr << "WARNING unknown TimeSeriesIntegrator type " << type << " - ";
	opserr << " SeriesIntegratorType <type args>\n\tvalid types: Trapezoidal or Simpson\n";
	return 0;
    }
    if (seriesIntegrator == 0) {
	opserr << "WARNING invalid series integrator: " << type;
	opserr << " - pattern UniformExcitation -int {Series Integrator}\n";
	return 0;
    }

    return seriesIntegrator;
}
#endif


TimeSeriesIntegrator::TimeSeriesIntegrator (int classTag)
:MovableObject(classTag)
{

}

TimeSeriesIntegrator::~TimeSeriesIntegrator()
{

}
