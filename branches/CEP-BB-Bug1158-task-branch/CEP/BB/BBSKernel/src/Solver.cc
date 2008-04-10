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


ostream &operator<<(ostream &out, const casa::LSQFit &eq)
{
    casa::uInt rank, nun, np, ncon, ner, *piv;
    casa::Double *nEq, *known, *constr, *er, *sEq, *sol, prec, nonlin;
    eq.debugIt(nun, np, ncon, ner, rank, nEq, known, constr, er, piv, sEq, sol,
        prec, nonlin);
    
    for(size_t i = 0; i < nun; ++i)
    {
        size_t idx = ((2*nun+1-i)*i)/2;
    	for(size_t j = 0; j < nun - i; ++j)
    	{
    		cout << nEq[idx+j] << "  ";
    	}
    	cout << endl;
    }
    
    return out;
}

//Solver::Solver()
//{
//    itsCoefficientIndex.reset(new CoefficientIndex());
//}

/*
void Solver::setParameterIndex(uint32 kernelId, const vector<string> &index)
{
    vector<uint32> &parameterMapping = itsParameterMapping[kernelId];
    parameterMapping.resize(index.size());

    for(uint32 i = 0; i < index.size(); ++i)
    {
        pair<map<string, uint32>::iterator, bool> result =
            itsParameters.insert(make_pair(index[i], itsParameters.size()));

        parameterMapping[i] = (result.first)->second;
    }

    cout << "Mapping (" << kernelId << "): " << parameterMapping << endl;
}


void Solver::getParameterIndex(vector<string> &index) const
{
    index.reserve(itsParameters.size());
    for(map<string, uint32>::const_iterator it = itsParameters.begin(),
        end = itsParameters.end();
        it != end;
        ++it)
    {
        index.push_back(it->first);
    }
}
*/

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


void setCoefficients(CoefficientMsg::Pointer msg)
{
}

void setEquations(EquationMsg::Pointer msg)
{
}

bool iterate(SolutionMsg::Pointer msg)
{
    return true;
}

/*
void Solver::getCoefficientIndices(vector<CoefficientIndex> &index) const
{
    index.clear();
    
    for(map<uint32, CoefficientIndex>::const_iterator it =
        itsCoefficientIndex.begin(),
        end = itsCoefficientIndex.end();
        it != end;
        ++it)
    {
        index.push_back(it->second);        
    }
}
*/

/*
void Solver::setCoefficientIndex(uint32 kernelId, uint32 cellId,
    const CoefficientIndex &index)
{
    // Update global CoefficientIndex.
    vector<uint32> &parameterMapping = itsParameterMapping[kernelId];
    vector<uint32> &coefficientMapping =
        itsCoefficientMapping[kernelId][cellId];
    coefficientMapping.resize(index.getCount());

    CoefficientIndex &globalIndex = itsCoefficientIndex[cellId];

    for(CoefficientIndex::const_iterator it = index.begin(), end = index.end();
        it != end;
        ++it)
    {
        const CoefficientInterval &result =
            globalIndex.setInterval(parameterMapping[it->first],
                (it->second).length);

        for(size_t i = 0; i < result.length; ++i)
        {
            coefficientMapping[(it->second).start + i] = result.start + i;
        }
    }
}
*/

/*
void Solver::setCoefficientIndex(size_t kernelId,
    const CoefficientIndex &index)
{
    NSTimer t1, t2, t3;
    
    // Merge the kernel's coefficient index with the solver's coefficient index.
    t1.start();
    itsCoefficientIndex->merge(index);
    t1.stop();
    
    // Construct a look-up table from parameter indices in index to
    // parameter indices in itsCoefficientIndex. Parameter indices are matched 
    // by matching the corresponding parameter names.
    t2.start();
    const std::map<string, uint> &kernelParameters = index.getParameters();

    vector<uint> lut;
    lut.resize(kernelParameters.size());

    for(map<string, uint>::const_iterator it = kernelParameters.begin(),
        end = kernelParameters.end();
        it != end;
        ++it)
    {
        lut[it->second] = itsCoefficientIndex->getParameterIndex(it->first);
    }
    t2.stop();
   
//    cout << lut << endl;
    
    // Build index look-up table for all solve domains of this kernel.
    t3.start();
    map<uint, vector<uint> > &kernelIndexLut = itsIndexLut[kernelId];

    const map<uint, uint> &kernelDomains = index.getDomains();
    for(map<uint, uint>::const_iterator itDomain = kernelDomains.begin(),
        endDomain = kernelDomains.end();
        itDomain != endDomain;
        ++itDomain)
    {
        // Get the global domain identifier.
        uint domainId = itDomain->first;
        
        // Get the index of this domain in the kernel's coefficient index.
        uint kernelDomainIndex = itDomain->second;
        
        // Get the index of this domain in the solver's coefficient index.
        uint domainIndex = itsCoefficientIndex->getDomainIndex(domainId);
        
        // Allocate the index lookup-table for this (kernel, domain).
        vector<uint> &indexLut = kernelIndexLut[domainId];
        indexLut.resize(index.getCoefficientCount(kernelDomainIndex));
        
        // Construct the index lookup-table.
        const map<uint, CoefficientInterval> &kernelIntervals =
            index.getCoefficientIntervals(kernelDomainIndex);
        for(map<uint, CoefficientInterval>::const_iterator itInterval = kernelIntervals.begin(),
            endParameter = kernelIntervals.end();
            itInterval != endParameter;
            ++itInterval)
        {
            DBGASSERT(itInterval->first < lut.size());
            
            CoefficientInterval interval =
                itsCoefficientIndex->getCoefficientInterval(domainIndex,
                    lut[itInterval->first]);
                
            for(uint i = 0; i < interval.length; ++i)
            {
                indexLut[itInterval->second.start + i] = interval.start + i;
            }
        }
    }
    t3.stop();
    
//    cout << itsCoefficientIndex << endl;
    cout << t1 << endl;
    cout << t2 << endl;
    cout << t3 << endl;
}
*/

/*
void Solver::setCoefficients(uint32 kernelId, const vector<Coefficients> &src)
{
    map<uint32, vector<uint32> > &cMapping = itsCoefficientMapping[kernelId];

    for(size_t i = 0; i < src.size(); ++i)
    {
        const uint32 cellId = src[i].itsCellId;

        Cell &cell = itsCells[cellId];
        const CellCoefficientIndex &index = itsCoefficientIndex[cellId];

        if(cell.solver.nUnknowns() == 0)
        {
            cout << "Initializing cell: " << cellId << endl;
            cout << "Coefficient count: " << index.getCount() << endl;

            cell.coefficients.resize(index.getCount());
            cell.solver.set(index.getCount());
            // Set new value solution test
            cell.solver.setEpsValue(1e-9);
            // Set new 'derivative' test (actually, the inf norm of the known
            // vector is checked).
            cell.solver.setEpsDerivative(1e-9);
            // Set maximum number of iterations
            cell.solver.setMaxIter(20);
            // Set new factors (collinearity factor, and Levenberg-Marquardt LMFactor)
            cell.solver.set(1e-9, 1.0);
            cell.solver.setBalanced(true);
        }

        const vector<uint32> &mapping = cMapping[cellId];            
        cout << "Look-up table: " << mapping << endl;
        
        ASSERT(mapping.size() == src[i].itsCoefficients.size());
    
        for(size_t j = 0; j < src[i].itsCoefficients.size(); ++j)
        {
            cell.coefficients[mapping[j]] = src[i].itsCoefficients[j];
        }
        cout << "Coefficients: " << cell.coefficients << endl;
    }
}
*/

/*
void Solver::setEquations(uint32 kernelId, const vector<Equation> &equations)
{
//    map<uint, map<uint, vector<uint> > >::const_iterator kernelIt =
//        itsIndexLut.find(kernelId);        
//    ASSERT(kernelIt != itsIndexLut.end());

//    map<uint, vector<uint> >::const_iterator indexIt =
//        kernelIt->second.find(domainId);
//    ASSERT(indexIt != kernelIt->second.end());
        
//    const vector<uint> &lut = indexIt->second;
    map<uint32, vector<uint32> > &cmap = itsCoefficientMapping[kernelId];

    for(size_t i = 0; i < equations.size(); ++i)
    {
        const uint32 cellId = equations[i].itsCellId;
        const vector<uint> &mapping = cmap[cellId];

        Cell &cell = itsCells[cellId];
        cout << "Check: " <<  equations[i].itsEquations.nUnknowns() << " "
            << mapping.size() << endl;
            
        ASSERT(equations[i].itsEquations.nUnknowns() == mapping.size());
        bool result = cell.solver.merge(equations[i].itsEquations,
            mapping.size(), const_cast<uint*>(&mapping[0]));
        ASSERT(result);

        ASSERT(cell.count >= 1);
        cell.count--;

        if(cell.count == 0)
        {
            cout << "Cell " << cellId << " ready for iteration." << endl;
    }
//        cout << cell.solver << endl;
  }
}


void Solver::iterate(vector<Solution> &solutions)
{
    for(map<uint, Cell>::iterator it = itsCells.begin(), end = itsCells.end();
        it != end;
        ++it)
    {
        Cell &cell = it->second;

        if(cell.count != 0)
        {
            continue;
        }
        
        cell.count = itsGroupSize;
        
        // Get some statistics from the solver. Note that the chi squared is
        // valid for the _previous_ iteration. The solver cannot compute the
        // chi squared directly after an iteration, because it needs the new
        // condition equations for that and these are computed by the kernel.
        casa::uInt rank, nun, np, ncon, ner, *piv;
        casa::Double *nEq, *known, *constr, *er, *sEq, *sol, prec, nonlin;
        cell.solver.debugIt(nun, np, ncon, ner, rank, nEq, known, constr, er,
            piv, sEq, sol, prec, nonlin);
        DBGASSERT(er && ner > SUMLL);

        double lmFactor = nonlin;
        double chiSquared = er[SUMLL] / std::max(er[NC] + nun, 1.0);

        // Solve the normal equations.
        cell.solver.solveLoop(rank, &(cell.coefficients[0]), true);

    //    cout << "Coefficients: " << cell.coefficients << endl;

        // Construct a result.
        Solution solution;
        solution.itsCellId = it->first;
        solution.itsCoefficients = cell.coefficients;
        solution.itsResultCode = cell.solver.isReady();
        solution.itsResultText = cell.solver.readyText();
        solution.itsRank = rank;
        solution.itsChiSquared = chiSquared;
        solution.itsLMFactor = lmFactor;

        solutions.push_back(solution);

        // If we're done with this domain, unregister it.
        if(cell.solver.isReady() != NONREADY)
        {
            // remove solver
            cout << "Done with cell " << it->first << endl;
        }
    }
}
*/

/*
void Solver::setCoefficients(uint kernelId, uint domainId, const vector<double> &coefficients)
{
    Domain &domain = itsDomains[domainId];

    if(domain.solver.nUnknowns() == 0)
    {
        uint domainIndex = itsCoefficientIndex->getDomainIndex(domainId);
        
        cout << "Initializing domain" << endl;
        cout << "Coefficient count: " << itsCoefficientIndex->getCoefficientCount(domainIndex) << endl;

        domain.coefficients.resize(itsCoefficientIndex->getCoefficientCount(domainIndex));
        domain.solver.set(itsCoefficientIndex->getCoefficientCount(domainIndex));
        // Set new value solution test
        domain.solver.setEpsValue(1e-9);
        // Set new 'derivative' test (actually, the inf norm of the known
        // vector is checked).
        domain.solver.setEpsDerivative(1e-9);
        // Set maximum number of iterations
        domain.solver.setMaxIter(20);
        // Set new factors (collinearity factor, and Levenberg-Marquardt LMFactor)
        domain.solver.set(1e-9, 1.0);
        domain.solver.setBalanced(true);
    }
            
    const map<uint, map<uint, vector<uint> > >::const_iterator kernelIt =
        itsIndexLut.find(kernelId);        
    ASSERT(kernelIt != itsIndexLut.end());

    const map<uint, vector<uint> >::const_iterator indexIt =
        kernelIt->second.find(domainId);
    ASSERT(indexIt != kernelIt->second.end());
        
    const vector<uint> &lut = indexIt->second;
//    cout << "Look-up table: " << lut << endl;
    ASSERT(lut.size() == coefficients.size());
    
    for(size_t i = 0; i < coefficients.size(); ++i)
    {
        domain.coefficients[lut[i]] = coefficients[i];
    }
    
//    cout << "Coefficients: " << domain.coefficients << endl;
}


void Solver::setEquations(uint kernelId, uint domainId,
    const casa::LSQFit &equations)
{
    map<uint, map<uint, vector<uint> > >::const_iterator kernelIt =
        itsIndexLut.find(kernelId);        
    ASSERT(kernelIt != itsIndexLut.end());

    map<uint, vector<uint> >::const_iterator indexIt =
        kernelIt->second.find(domainId);
    ASSERT(indexIt != kernelIt->second.end());
        
    const vector<uint> &lut = indexIt->second;

    ASSERT(equations.nUnknowns() == lut.size());
    ASSERT(itsDomains[domainId].solver.nUnknowns() == lut.size());
    bool result = itsDomains[domainId].solver.merge(equations, lut.size(),
        const_cast<uint*>(&lut[0]));
    ASSERT(result);

    cout << itsDomains[domainId].solver << endl;
}


IterationResult Solver::iterate(uint domainId)
{
    Domain &domain = itsDomains[domainId];
    ASSERT(domain.solver.nUnknowns() > 0);

    // Get some statistics from the solver. Note that the chi squared is
    // valid for the _previous_ iteration. The solver cannot compute the
    // chi squared directly after an iteration, because it needs the new
    // condition equations for that and these are computed by the kernel.
    casa::uInt rank, nun, np, ncon, ner, *piv;
    casa::Double *nEq, *known, *constr, *er, *sEq, *sol, prec, nonlin;
    domain.solver.debugIt(nun, np, ncon, ner, rank, nEq, known, constr, er,
        piv, sEq, sol, prec, nonlin);
    ASSERT(er && ner > SUMLL);

    double lmFactor = nonlin;
    double chiSquared = er[SUMLL] / std::max(er[NC] + nun, 1.0);

    // Solve the normal equations.
    domain.solver.solveLoop(rank, &(domain.coefficients[0]), true);

//    cout << "Coefficients: " << domain.coefficients << endl;

    // Construct a result.
    IterationResult result(domainId,
        domain.solver.isReady(),
        domain.solver.readyText(),
        domain.coefficients,
        rank,
        chiSquared,
        lmFactor);

    // If we're done with this domain, unregister it.
    if(domain.solver.isReady() != NONREADY)
    {
        // remove solver
    }

    return result;
}
*/

} // namespace BBS
} // namespace LOFAR
