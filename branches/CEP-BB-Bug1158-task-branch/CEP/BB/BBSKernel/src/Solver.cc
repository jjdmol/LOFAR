//# Solver.cc: 
//#
//# Copyright (C) 2007
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#include <lofar_config.h>
#include <BBSKernel/Solver.h>

#include <Common/LofarTypes.h>
#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>
#include <Common/Timer.h>
#include <Common/lofar_iostream.h>

#if 0
    #define NONREADY        casa::LSQFit::NONREADY
    #define N_ReadyCode     casa::LSQFit::N_ReadyCode
    #define SUMLL           casa::LSQFit::SUMLL
    #define NC              casa::LSQFit::NC
#else
    #define NONREADY        0
    #define N_ReadyCode     999
    #define NC              0
    #define SUMLL           2
#endif


namespace LOFAR
{
namespace BBS 
{
using LOFAR::operator<<;

Solver::Solver()
{
    reset();
}


void Solver::reset(double epsValue, double epsDerivative, size_t maxIter,
    double colFactor, double lmFactor, bool balanced, bool useSvd)
{
    itsEpsValue = epsValue;
    itsEpsDerivative = epsDerivative;
    itsMaxIter = maxIter;
    itsColFactor = colFactor;
    itsLmFactor = lmFactor;
    itsBalanced = balanced;
    itsUseSvd = useSvd;
    
    itsCells.clear();
    itsCoeffIndex.clear();
    itsCoeffMapping.clear();
}

    
void Solver::setCoeffIndex(uint32 kernelId, const CoeffIndex &local)
{
    vector<uint32> &mapping = itsCoeffMapping[kernelId];
    mapping.resize(local.getCoeffCount());
    for(CoeffIndex::const_iterator it = local.begin(), end = local.end();
        it != end;
        ++it)
    {
        const CoeffInterval &interval =
            itsCoeffIndex.insert(it->first, it->second.length);

        for(size_t i = 0; i < interval.length; ++i)
        {
            mapping[it->second.start + i] = interval.start + i;
        }
    }        
}    


const CoeffIndex &Solver::getCoeffIndex() const
{
    return itsCoeffIndex;
}


void Solver::setCoeff(uint32 kernelId, const vector<CellCoeff> &local)
{
    const vector<uint32> &mapping = itsCoeffMapping[kernelId];
    LOG_DEBUG_STR("Look-up table: " << mapping);

    const uint32 nCoeff = itsCoeffIndex.getCoeffCount();
    LOG_DEBUG_STR("Global coefficient count: " << nCoeff);

    for(size_t i = 0; i < local.size(); ++i)
    {
        pair<map<uint32, Cell>::iterator, bool> result =
            itsCells.insert(make_pair(local[i].id, Cell()));
        Cell &cell = (result.first)->second;
        
        if(result.second)
        {
            ASSERT(cell.solver.nUnknowns() == 0);
            LOG_DEBUG_STR("Initializing cell: " << local[i].id);

            cell.coeff.resize(nCoeff);
            cell.solver.set(nCoeff);
            cell.solver.setEpsValue(itsEpsValue);
            cell.solver.setEpsDerivative(itsEpsDerivative);
            cell.solver.setMaxIter(itsMaxIter);
            cell.solver.set(itsColFactor, itsLmFactor);
            cell.solver.setBalanced(itsBalanced);
        }

        const vector<double> &localCoeff = local[i].coeff;
        ASSERT(mapping.size() == localCoeff.size());

        for(size_t j = 0; j < localCoeff.size(); ++j)
        {
            cell.coeff[mapping[j]] = localCoeff[j];
        }

        LOG_DEBUG_STR("Global coefficients: " << cell.coeff);
    }
}


void Solver::setEquations(uint32 kernelId, const vector<CellEquation> &local)
{
    const vector<uint32> &mapping = itsCoeffMapping[kernelId];
    for(size_t i = 0; i < local.size(); ++i)
    {
        map<uint32, Cell>::iterator it = itsCells.find(local[i].id);
        if(it == itsCells.end())
        {
            continue;
        }

        Cell &cell = it->second;
        ASSERT(local[i].equation.nUnknowns() == mapping.size());
        const bool ok = cell.solver.merge(local[i].equation,
            mapping.size(), const_cast<uint32*>(&mapping[0]));
        ASSERT(ok);
    }
}


void Solver::getEquations(vector<CellEquation> &global)
{
    global.resize(itsCells.size());

    size_t i = 0;
    for(map<uint32, Cell>::const_iterator it = itsCells.begin(),
        end = itsCells.end();
        it != end;
        ++it)
    {
        global[i].id = it->first;
        global[i].equation = it->second.solver;
        ++i;
    }
}


bool Solver::iterate(vector<CellSolution> &global)
{
    // Empty the solution vector.
    global.clear();

    bool done = true;
    map<uint32, Cell>::iterator it = itsCells.begin();
    while(it != itsCells.end())
    {
        const uint32 cellId = it->first;
        Cell &cell = it->second;
        
        ASSERT(cell.solver.nUnknowns() > 0);

        // Get some statistics from the solver. Note that the chi squared is
        // valid for the _previous_ iteration. The solver cannot compute the
        // chi squared directly after an iteration, because it needs the new
        // condition equations for that and these are computed by the kernel.
        casa::uInt rank, nun, np, ncon, ner, *piv;
        casa::Double *nEq, *known, *constr, *er, *sEq, *sol, prec, nonlin;
        cell.solver.debugIt(nun, np, ncon, ner, rank, nEq, known, constr, er,
            piv, sEq, sol, prec, nonlin);
        ASSERT(er && ner > SUMLL);

        double lmFactor = nonlin;
        double chiSqr = er[SUMLL] / std::max(er[NC] + nun, 1.0);

        // Solve the normal equations.
        cell.solver.solveLoop(rank, &(cell.coeff[0]), itsUseSvd);
//        cout << "Coefficients: " << cell.coeff << endl;

        // Record solution and statistics.
        CellSolution solution(cellId);
        solution.coeff = cell.coeff;
        solution.result = cell.solver.isReady();
        solution.resultText = cell.solver.readyText();
        solution.rank = rank;
        solution.chiSqr = chiSqr;
        solution.lmFactor = lmFactor;

        global.push_back(solution);

        if(cell.solver.isReady() == NONREADY)
        {
            done = false;
            ++it;
        }
        else
        {
            itsCells.erase(it++);
        }
    }
    
    return done;
}


} // namespace BBS
} // namespace LOFAR
