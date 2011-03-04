//# Solver.h:
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

#ifndef LOFAR_BBSKERNEL_SOLVER_H
#define LOFAR_BBSKERNEL_SOLVER_H

#include <BBSKernel/SolverInterfaceTypes.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_smartptr.h>
#include <Common/lofar_map.h>
#include <Common/LofarLogger.h>
//#include <ParmDB/ParmDBLog.h>
#include <ParmDB/Box.h>
#include <ParmDB/Grid.h>

#include <casa/Arrays/Array.h>
#include <scimath/Fitting/LSQFit.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

// Options for the solver.
class SolverOptions
{
public:
    SolverOptions();

    // Maximum number of iterations.
    size_t    maxIter;
    // Value convergence threshold.
    double    epsValue;
    // Derivative convergence threshold
    double    epsDerivative;
    // Colinearity factor.
    double    colFactor;
    // Levenberg-Marquardt factor.
    double    lmFactor;
    // Indicates well-balanced normal equations.
    bool      balancedEq;
    // Use singular value decomposition.
    bool      useSVD;
};

// Write a SolverOptions instance to an output stream in human readable form.
ostream& operator<<(ostream&, const SolverOptions&);

class Solver
{
//  friend class LocalSolveController;
  
public:
    typedef shared_ptr<Solver>          Ptr;
    typedef shared_ptr<const Solver>    ConstPtr;

    Solver();
    Solver(const SolverOptions &options);

    // (Re)set the solver's state. This allows a solver instance to be
    // re-used.
    void reset(const SolverOptions &options);

    // Set the (local) coefficient index of a kernel.
    void setCoeffIndex(size_t kernelId, const CoeffIndex &local);
    // Get the merged (global) coefficient index.
    CoeffIndex getCoeffIndex() const;

    // Get the the parm to coefficient index map
    map<size_t, vector<casa::uInt> > getCoeffMapping() const;
    
    // Set the initial coefficients of a kernel.
    template <typename T_ITER>
    void setCoeff(size_t kernelId, T_ITER first, T_ITER last);

    // Set the equations of a kernel.
    template <typename T_ITER>
    void setEquations(size_t kernelId, T_ITER first, T_ITER last);

    // Get the maximum number of iterations that are set
    size_t getMaxIter(void) const;

    // Get the current solver options from the solver
    SolverOptions getOptions() const;
    
    // Perform an iteration for all available cells.
    template <typename T_OUTPUT_ITER>
    bool iterate(T_OUTPUT_ITER out);
    
    // Solver covariance Matrix functions:
    // Read the covariance matrix from the solver
    
    // write covariance matrix in memory pointer provided; if corrMem=NULL, 
    // memory will be allocated, otherwise must be of sufficient size
    bool getCovarianceMatrix(uint32 id, double **corrMem=NULL);
    // write covariance to a casa::Array, will be resized if necessary
    bool getCovarianceMatrix(uint32 id, casa::Array<casa::Double> &);
    
    
    // Get the covariance matrix for a list of ids from itsCells
    void getCovarianceMatrices(vector<CellSolution> &Solutions, vector<CovarianceMatrix> &);
    
    // Get the covariance matrices for all solved solutions in itsCells
    //void getCovarianceMatrices(vector<CovarianceMatrix> &);
    
    // Remove solved solutions from itsCells
    bool removeSolvedSolutions();
    // Remove solved solutions from Solutions vector
    //void removeSolvedSolutions(vector<CellSolution> &Solutions);
    
    
private:
    //# TODO: Older versions of casacore do not define the symbols listed below,
    //# which is worked around by #define-ing the values explicitly. There must
    //# be a better way to do this, e.g. check for the casacore version in the
    //# build system and dealing with the problem there.
    static const unsigned int NONREADY = 0;  //# casa::LSQFit::NONREADY
    static const unsigned int NC = 0;        //# casa::LSQFit::NC
    static const unsigned int SUMLL = 2;     //# casa::LSQFit::SUMLL

    struct Cell
    {
        casa::LSQFit    solver;
        vector<double>  coeff;
    };

    map<size_t, Cell>                   itsCells;
    CoeffIndex                          itsCoeffIndex;
    map<size_t, vector<casa::uInt> >    itsCoeffMapping;

    double                              itsEpsValue;
    double                              itsEpsDerivative;
    size_t                              itsMaxIter;
    double                              itsColFactor;
    double                              itsLMFactor;
    bool                                itsBalancedEq;
    bool                                itsUseSVD;
};

// @}

template <typename T_ITER>
void Solver::setCoeff(size_t kernelId, T_ITER first, T_ITER last)
{
    const size_t nCoeff = itsCoeffIndex.getCoeffCount();

    map<size_t, vector<casa::uInt> >::const_iterator it =
        itsCoeffMapping.find(kernelId);
    ASSERT(it != itsCoeffMapping.end());
    const vector<casa::uInt> &mapping = it->second;

    for(; first != last; ++first)
    {
        pair<map<size_t, Cell>::iterator, bool> result =
            itsCells.insert(make_pair(first->id, Cell()));
        Cell &cell = (result.first)->second;

        if(result.second)
        {
            // Initialize cell if this is the first time it is referenced.
            ASSERT(cell.solver.nUnknowns() == 0);
            cell.coeff.resize(nCoeff);
            cell.solver.set(static_cast<casa::uInt>(nCoeff));
            cell.solver.setEpsValue(itsEpsValue);
            cell.solver.setEpsDerivative(itsEpsDerivative);
            cell.solver.setMaxIter(itsMaxIter);
            cell.solver.set(itsColFactor, itsLMFactor);
            cell.solver.setBalanced(itsBalancedEq);
        }

        // Merge local coefficients into the global coefficient vector for
        // this cell.
        const vector<double> &coeff = first->coeff;
        ASSERT(mapping.size() == coeff.size());
        for(size_t j = 0; j < coeff.size(); ++j)
        {
            cell.coeff[mapping[j]] = coeff[j];
        }
    }
}

template <typename T_ITER>
void Solver::setEquations(size_t kernelId, T_ITER first, T_ITER last)
{
    map<size_t, vector<casa::uInt> >::const_iterator it =
        itsCoeffMapping.find(kernelId);
    ASSERT(it != itsCoeffMapping.end());
    const vector<casa::uInt> &mapping = it->second;

    for(; first != last; ++first)
    {
        map<size_t, Cell>::iterator it = itsCells.find(first->id);
        if(it == itsCells.end())
        {
            continue;
        }

        Cell &cell = it->second;
        ASSERT(first->equation.nUnknowns() == mapping.size());
        const bool ok = cell.solver.merge(first->equation,
            mapping.size(), const_cast<casa::uInt*>(&mapping[0]));
        ASSERT(ok);
    }
}

template <typename T_OUTPUT_ITER>
bool Solver::iterate(T_OUTPUT_ITER out)
{
    bool done = true;
    map<size_t, Cell>::iterator it = itsCells.begin();
    while(it != itsCells.end())
    {
        const size_t cellId = it->first;
        Cell &cell = it->second;      
        
        if (cell.solver.isReady() == Solver::NONREADY)
        {
           // Get some statistics from the solver. Note that the chi squared is
           // valid for the _previous_ iteration. The solver cannot compute the
           // chi squared directly after an iteration, because it needs the new
           // condition equations for that and these are computed by the kernel.
           casa::uInt rank, nun, np, ncon, ner, *piv;
           casa::Double *nEq, *known, *constr, *er, *sEq, *sol, prec, nonlin;
           cell.solver.debugIt(nun, np, ncon, ner, rank, nEq, known, constr, er,
               piv, sEq, sol, prec, nonlin);
           ASSERT(er && ner > Solver::SUMLL);
   
           double lmFactor = nonlin;
           double chiSqr = er[Solver::SUMLL] / std::max(er[Solver::NC] + nun, 1.0);
                      
           // Perform an iteration. Only if the cell has not been solved for already
           // TODO: Fix this for Correlation Matrix logging
           cell.solver.solveLoop(rank, &(cell.coeff[0]), itsUseSVD);           
               
           // Record solution and statistics.
           CellSolution solution(static_cast<uint32>(cellId));
           solution.coeff = cell.coeff;
           solution.ready = (cell.solver.isReady() != Solver::NONREADY);
           solution.resultText = cell.solver.readyText();
           solution.rank = rank;
           solution.rankDeficiency = cell.solver.getDeficiency();
           solution.niter = cell.solver.nIterations();        
           solution.chiSqr = chiSqr;
           solution.lmFactor = lmFactor;
        
           // Temporary hack
           *out++ = solution;
           ++it;          
        }       

        if(cell.solver.isReady() == Solver::NONREADY)
        {
            done = false;
            //++it;
        }
    }

    return done;
}

} //# namespace BBS
} //# namespace LOFAR

#endif
