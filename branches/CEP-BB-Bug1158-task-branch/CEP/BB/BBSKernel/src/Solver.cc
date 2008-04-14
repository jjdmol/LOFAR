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

#include <Common/lofar_iostream.h>
#include <Common/LofarTypes.h>
#include <Common/StreamUtil.h>
#include <Common/Timer.h>

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
    itsCoefficientIndex = CoefficientIndex();
    itsCoeffMapping.clear();
}

    
CoeffIndexMsg::Pointer Solver::getCoefficientIndex() const
{
    CoeffIndexMsg::Pointer msg(new CoeffIndexMsg(0));
    msg->getContents() = itsCoefficientIndex;

    return msg;
}


void Solver::setCoefficientIndex(CoeffIndexMsg::Pointer msg)
{
    const uint32 kernelId = msg->getKernelId();
    const CoefficientIndex &index = msg->getContents();

    // Construct a mapping from local parameter id to global parameter id
    // based on name.
    vector<uint32> pMapping(index.getParmCount());
    for(CoefficientIndex::ParmIndexType::const_iterator it = index.beginParm(),
        end = index.endParm();
        it != end;
        ++it)
    {
        const pair<uint32, bool> result =
            itsCoefficientIndex.insertParm(it->first);
            
        pMapping[it->second] = result.first;
    }
    cout << "Parameter mapping (" << kernelId << "): " << pMapping << endl;
    
    for(CoefficientIndex::CoeffIndexType::const_iterator it =
        index.begin(), end = index.end();
        it != end;
        ++it)
    {
        const uint32 cellId = it->first;
        const CellCoeffIndex &local = it->second;
        CellCoeffIndex &global = itsCoefficientIndex[cellId];
        
        vector<uint32> &cMapping = itsCoeffMapping[kernelId][cellId];
        cMapping.resize(local.getCount());
        for(CellCoeffIndex::const_iterator intIt = local.begin(),
            intEnd = local.end();
            intIt != intEnd;
            ++intIt)
        {
            const CoeffInterval &interval =
                global.setInterval(pMapping[intIt->first],
                    (intIt->second).length);

            for(size_t i = 0; i < interval.length; ++i)
            {
                cMapping[(intIt->second).start + i] = interval.start + i;
            }
        }
    }
}    


void Solver::setCoefficients(CoefficientMsg::Pointer msg)
{
    map<uint32, vector<uint32> > &cMapping =
        itsCoeffMapping[msg->getKernelId()];

    const vector<CellCoeff> &cellCoeff = msg->getContents();

    for(size_t i = 0; i < cellCoeff.size(); ++i)
    {
        const uint32 cellId = cellCoeff[i].id;
        const size_t coeffCount = itsCoefficientIndex[cellId].getCount();

        pair<map<uint32, Cell>::iterator, bool> result =
            itsCells.insert(make_pair(cellId, Cell()));
        Cell &cell = (result.first)->second;

        ASSERT(cell.iteration == 0);

        if(cell.solver.nUnknowns() == 0)
        {
            cout << "Initializing cell: " << cellId << endl;
            cout << "Coefficient count: " << coeffCount << endl;

            cell.coeff.resize(coeffCount);
            cell.solver.set(coeffCount);
            cell.solver.setEpsValue(itsEpsValue);
            cell.solver.setEpsDerivative(itsEpsDerivative);
            cell.solver.setMaxIter(itsMaxIter);
            cell.solver.set(itsColFactor, itsLmFactor);
            cell.solver.setBalanced(itsBalanced);
        }

        const vector<uint32> &mapping = cMapping[cellId];            
        cout << "Look-up table: " << mapping << endl;
        
        ASSERT(mapping.size() == cellCoeff[i].coeff.size());
    
        for(size_t j = 0; j < cellCoeff[i].coeff.size(); ++j)
        {
            cell.coeff[mapping[j]] = cellCoeff[i].coeff[j];
        }
        cout << "Coefficients: " << cell.coeff << endl;
    }
}


void Solver::setEquations(EquationMsg::Pointer msg)
{
    map<uint32, vector<uint32> > &cMapping =
        itsCoeffMapping[msg->getKernelId()];

    const vector<CellEquation> &equations = msg->getContents();

    for(size_t i = 0; i < equations.size(); ++i)
    {
        const uint32 cellId = equations[i].id;
        map<uint32, Cell>::iterator it = itsCells.find(cellId);
        if(it == itsCells.end())
        {
            continue;
        }

        Cell &cell = it->second;
        const vector<uint32> &mapping = cMapping[cellId];
        ASSERT(equations[i].equation.nUnknowns() == mapping.size());
        const bool ok = cell.solver.merge(equations[i].equation,
            mapping.size(), const_cast<uint32*>(&mapping[0]));
        ASSERT(ok);
    }
}


bool Solver::iterate(SolutionMsg::Pointer msg)
{
    vector<CellSolution> &solutions = msg->getContents();

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
        cout << "Coefficients: " << cell.coeff << endl;

        // Increase iteration counter.
        ++cell.iteration;

        CellSolution solution(cellId);
        solution.coeff = cell.coeff;
        solution.result = cell.solver.isReady();
        solution.resultText = cell.solver.readyText();
        solution.rank = rank;
        solution.chiSqr = chiSqr;
        solution.lmFactor = lmFactor;

        solutions.push_back(solution);

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
