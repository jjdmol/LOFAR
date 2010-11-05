//# Solver.cc:
//#
//# Copyright (C) 2007
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>
#include <BBSKernel/Solver.h>

#include <Common/LofarTypes.h>
#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>
#include <Common/Timer.h>
#include <Common/lofar_iostream.h>

#include <ParmDB/ParmDBLog.h>
#include <BBSKernel/Exceptions.h>
#include <ParmDB/Grid.h>


namespace LOFAR
{
namespace BBS
{
using LOFAR::operator<<;

SolverOptions::SolverOptions()
    :   maxIter(0),
        epsValue(0.0),
        epsDerivative(0.0),
        colFactor(0.0),
        lmFactor(0.0),
        balancedEq(false),
        useSVD(false)
{
}

ostream& operator<<(ostream& os, const SolverOptions& obj)
{
    os << "Solver options:";
    Indent id;
    os << endl << indent << "Max nr. of iterations: " << obj.maxIter
        << endl << indent << "Epsilon value: " << obj.epsValue
        << endl << indent << "Epsilon derivative: " << obj.epsDerivative
        << endl << indent << "Colinearity factor: " << obj.colFactor
        << endl << indent << "LM factor: " << obj.lmFactor
        << boolalpha
        << endl << indent << "Balanced equations: " << obj.balancedEq
        << endl << indent << "Use SVD: " << obj.useSVD
        << noboolalpha;

    return os;
}

Solver::Solver()
{
    SolverOptions options;
    reset(options);
}

Solver::Solver(const SolverOptions &options)
{
    reset(options);
}

void Solver::reset(const SolverOptions &options)
{
    itsMaxIter = options.maxIter;
    itsEpsValue = options.epsValue;
    itsEpsDerivative = options.epsDerivative;
    itsColFactor = options.colFactor;
    itsLMFactor = options.lmFactor;
    itsBalancedEq = options.balancedEq;
    itsUseSVD = options.useSVD;

    itsCells.clear();
    itsCoeffIndex.clear();
    itsCoeffMapping.clear();
}

void Solver::setCoeffIndex(size_t kernelId, const CoeffIndex &local)
{
    vector<casa::uInt> &mapping = itsCoeffMapping[kernelId];
    mapping.resize(local.getCoeffCount());
    for(CoeffIndex::const_iterator it = local.begin(), end = local.end();
        it != end; ++it)
    {
        const CoeffInterval &interval =
            itsCoeffIndex.insert(it->first, it->second.length);

        for(size_t i = 0; i < interval.length; ++i)
        {
            mapping[it->second.start + i] = interval.start + i;
        }
    }
}

CoeffIndex Solver::getCoeffIndex() const
{
    return itsCoeffIndex;
}


map<size_t, vector<casa::uInt> > Solver::getCoeffMapping() const
{
	return itsCoeffMapping;    	 
}


size_t Solver::getMaxIter() const
{
	return itsMaxIter;	
}	


SolverOptions Solver::getOptions() const
{
	SolverOptions options;

	options.maxIter=itsMaxIter;
	options.epsValue=itsEpsValue;
	options.epsDerivative=itsEpsDerivative;
	options.colFactor=itsColFactor;
	options.lmFactor=itsLMFactor;
	options.balancedEq=itsBalancedEq;
	options.useSVD=itsUseSVD;
	
	return options;	
}



bool Solver::getCovarianceMatrix(uint32 id, double * corrMem)  // function cant be const because of LSQFIT.getCor() ???
{
	unsigned int nUnknowns=0;						// first get number of unknowns U, the matrix has size of U*U
	
	map<size_t, Cell>::iterator it;    			 // we need an iterator to access the map
	it=itsCells.find(id);							// find element with id in cells
	if(it==itsCells.end())							// check if we are past the end of cells
	{
		LOG_DEBUG_STR("Solver.cc::getCorrMatrix() id out of range");
		return false;
	}
		
	nUnknowns=it->second.solver.nUnknowns();  // use it to access LSQFit solver methods
	
	if(nUnknowns==0)									// if there are no unknowns (should not be the case)
	{
		LOG_DEBUG_STR("Solver::getCorrMatrix nUnknowns=0");
		return false;
	}
	
	size_t nelements=nUnknowns*nUnknowns;
	if(corrMem==NULL)									// if no memory was provided
		corrMem=(double *)calloc(nelements, sizeof(double));
	
	// Get (real, not complex) correlation matrix from the LSQFit object
	if(!(it->second.solver.getCovariance(corrMem)))
	{
		LOG_DEBUG_STR("Solver.cc::getCorrMatrix() could not get Correlation Matrix");
		return false;
	}
	else
	{
		return true;
	}
}


bool Solver::getCovarianceMatrix(uint32 id, casa::Array<casa::Double> &corrMatrix) // function cant be const because of LSQFIT.getCor() ???
{
	LOG_DEBUG_STR("Solver::getCorrMatrix()"); // DEBUG
	
	unsigned int nUnknowns=0;						// first get number of unknowns U, the matrix has size of U*U
	
	map<size_t, Cell>::iterator it; 		      // we need an iterator to access the map
	it=itsCells.find(id);							// find element with id in cells
	if(it==itsCells.end())							// check if we are past the end of cells
	{
		LOG_DEBUG_STR("Solver.cc::getCorrMatrix() id out of range");
		return false;
	}
		
	nUnknowns=it->second.solver.nUnknowns();  // use it to access LSQFit solver methods
	
	if(nUnknowns==0)									// if there are no unknowns (should not be the case)
	{
		LOG_DEBUG_STR("Solver::getCorrMatrix nUnknowns=0");
		return false;
	}
	
	
	size_t nelements=nUnknowns*nUnknowns;		// nelements in the correlation matrix is N*N
	casa::IPosition shape(nelements);
	
	corrMatrix.resize(shape); 						// resize casa array accordingly
	
	LOG_DEBUG_STR("Solver::getCorrMatrix() shape = " << shape); // DEBUG
	
	// Get (real, not complex) correlation matrix from the LSQFit object
	if(!(it->second.solver.getCovariance(corrMatrix.data())))
	{
		LOG_DEBUG_STR("Solver.cc::getCorrMatrix() could not get Correlation Matrix");
		return false;
	}
	else
	{
		return true;
	}
}



void Solver::getCovarianceMatrices(vector<uint32> &ids, vector<CovarianceMatrix> &covarMatrices)
{
	
	for(vector<uint32>::iterator it=ids.begin(); it!=ids.end(); ++it)
	{
	}
	
}



void Solver::getCovarianceMatrices(vector<CovarianceMatrix> &covarMatrices)
{
	
}


void Solver::removeSolvedSolutions()
{
	
}


void Solver::removeSolvedSolutions(vector<CellSolution> &Solutions)
{
	
}


} // namespace BBS
} // namespace LOFAR
