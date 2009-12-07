//# LocalSolveController.cc: Class that controls the execution of a local solve
//# command.
//#
//# Copyright (C) 2008
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
#include <BBSControl/LocalSolveController.h>
#include <Common/StreamUtil.h>

namespace LOFAR
{
namespace BBS
{
using LOFAR::operator<<;


LocalSolveController::LocalSolveController
    (const ExprSet<JonesMatrix>::Ptr &lhs,
    const ExprSet<JonesMatrix>::Ptr &rhs,
    const SolverOptions &options)
    :   itsLHS(lhs),
        itsRHS(rhs),
        itsInitFlag(false)
{
	ASSERT(itsLHS->size() == itsRHS->size() && itsLHS->size() > 0 );
    itsSolver.reset(options.maxIter, options.epsValue, options.epsDerivative,
        options.colFactor, options.lmFactor, options.balancedEqs,
        options.useSVD);
}

LocalSolveController::~LocalSolveController()
{
    itsLHS->clearSolvableParms();
    itsRHS->clearSolvableParms();
}

void LocalSolveController::init(const vector<string> &include,
    const vector<string> &exclude, const Grid &evalGrid, const Grid &solGrid,
    unsigned int cellChunkSize, bool propagate)
{
    ASSERTSTR(!itsInitFlag, "Controller already initialized");
    ASSERT(evalGrid.size() > 0);
    ASSERT(solGrid.size() > 0);
    ASSERT(cellChunkSize > 0);

    itsPropagateFlag = propagate;
    itsSolGrid = solGrid;
    itsCellChunkSize = cellChunkSize;

    // Find all parameters matching the specified inclusion and exclusion
    // criteria.
    ParmGroup solvablesLHS = ParmManager::instance().makeSubset(include,
        exclude, itsLHS->getParms());
    ParmGroup solvablesRHS = ParmManager::instance().makeSubset(include,
        exclude, itsRHS->getParms());
    // Merge the set of parameters found for the left and right hand side.
    std::set_union(solvablesLHS.begin(), solvablesLHS.end(),
        solvablesRHS.begin(), solvablesRHS.end(),
        std::inserter(itsSolvables, itsSolvables.begin()));

    if(itsSolvables.empty())
    {
        THROW(BBSControlException, "No parameters found matching the specified"
            " inclusion and exclusion criteria.");
    }

    // Assign solution grid to solvables.
    ParmManager::instance().setGrid(itsSolGrid, itsSolvables);

    // Instruct left and right hand side to generate partial derivatives for
    // the solvable parameters.
    itsLHS->setSolvableParms(itsSolvables);
    itsRHS->setSolvableParms(itsSolvables);

    // Create coefficient index.
    makeCoeffIndex(itsSolvables);

    // Initialize equator.
    itsEquator.reset(new Equator(itsLHS, itsRHS, evalGrid, itsSolGrid,
        itsCoeffIndex));
//    itsEquator->setSelection(baselines, products);

    itsInitFlag = true;
}

void LocalSolveController::run()
{
    ASSERTSTR(itsInitFlag, "Controller not initialized.");

	// Exchange coefficient index with solver.
    itsSolver.setCoeffIndex(0, itsCoeffIndex);

    // Construct look-up table from Solver's coefficient index.
    makeCoeffMapping(itsSolvables, itsSolver.getCoeffIndex());

    // Compute the number of cell chunks to process.
    const unsigned int nCellChunks =
        static_cast<unsigned int>(ceil(static_cast<double>(itsSolGrid[TIME]->size())
            / itsCellChunkSize));

    vector<CellCoeff> coeff;
    vector<CellEquation> equations;
    vector<CellSolution> solutions;

    Location chunkStart(0, 0);
    Location chunkEnd(itsSolGrid[FREQ]->size() - 1,
        itsSolGrid[TIME]->size() - 1);
    for(unsigned int cellChunk = 0; cellChunk < nCellChunks; ++cellChunk)
    {
        // Compute end cell of current cell chunk.
        chunkEnd.second = std::min(chunkStart.second + itsCellChunkSize - 1,
            itsSolGrid[TIME]->size() - 1);

        // Get initial coefficients.
        getInitialCoeff(coeff, chunkStart, chunkEnd);

        // Set initial coefficients.
        itsSolver.setCoeff(0, coeff);

        // Set cell selection.
        itsEquator->setCellSelection(chunkStart, chunkEnd);

        // Iterate.
        bool done = false;
        while(!done)
        {
            // Construct equations and pass to solver.
            itsEquator->process(equations);

            itsSolver.setEquations(0, equations);

            // Perform a non-linear LSQ iteration.
            done = itsSolver.iterate(solutions);

            // Update coefficients.
            setSolution(solutions, chunkStart, chunkEnd);
        }

        // Propagate coefficient values to the next cell chunk.
        // TODO: Find a better solution for this.
        if(itsPropagateFlag && cellChunk < nCellChunks - 1)
        {
            // Determine start and end time index of the next cell chunk.
            const size_t tStart = chunkStart.second + itsCellChunkSize;
            const size_t tEnd = std::min(tStart + itsCellChunkSize - 1,
                itsSolGrid[TIME]->size() - 1);

            Location cell;
            vector<double> values;
            for(cell.first = chunkStart.first; cell.first <= chunkEnd.first;
                ++cell.first)
            {
                // Get coefficient values for the upper border of the current
                // cell chunk.
                cell.second = chunkEnd.second;
                getCoeff(values, cell);

                // Assign coefficient values to the strip of cells in the next
                // cell chunk at the same frequency index.
                for(cell.second = tStart; cell.second <= tEnd; ++cell.second)
                {
                    setCoeff(values, cell);
                }
            }
        }

        // Move to the next cell chunk.
        chunkStart.second += itsCellChunkSize;
    }
}

void LocalSolveController::makeCoeffIndex(const ParmGroup &solvables)
{
    ASSERT(itsCoeffIndex.getCoeffCount() == 0);

    ParmGroup::const_iterator solIt = solvables.begin();
    while(solIt != solvables.end())
    {
        ParmProxy::Ptr parm = ParmManager::instance().get(*solIt);
        itsCoeffIndex.insert(parm->getName(), parm->getCoeffCount());
        ++solIt;
    }
}

void LocalSolveController::makeCoeffMapping(const ParmGroup &solvables,
    const CoeffIndex &index)
{
    ASSERT(itsSolCoeffMapping.empty());

    ParmGroup::const_iterator solIt = solvables.begin();
    while(solIt != solvables.end())
    {
        ParmProxy::Ptr parm = ParmManager::instance().get(*solIt);

        CoeffIndex::const_iterator intervalIt = index.find(parm->getName());
        ASSERT(intervalIt != index.end());

        const CoeffInterval &interval = intervalIt->second;
        ASSERT(parm->getCoeffCount() == interval.length);

        itsSolCoeffMapping.push_back(interval.start);
        ++solIt;
    }
}

void LocalSolveController::getInitialCoeff(vector<CellCoeff> &result,
    const Location &start, const Location &end) const
{
    const unsigned int nCells = (end.first - start.first + 1)
        * (end.second - start.second + 1);

    result.resize(nCells);
    vector<CellCoeff>::iterator resultIt = result.begin();

    CellIterator cellIt(start, end);
    while(!cellIt.atEnd())
    {
        resultIt->id = itsSolGrid.getCellId(*cellIt);
        getCoeff(resultIt->coeff, *cellIt);

        ++resultIt;
        ++cellIt;
    }
}

void LocalSolveController::setSolution(const vector<CellSolution> &solutions,
    const Location &start, const Location &end) const
{
    for(size_t i = 0; i < solutions.size(); ++i)
    {
        Location cell(itsSolGrid.getCellLocation(solutions[i].id));
        ASSERT(cell.first >= start.first && cell.first <= end.first
            && cell.second >= start.second && cell.second <= end.second);

        setCoeff(solutions[i].coeff, cell, itsSolCoeffMapping);
    }

#if defined(LOFAR_DEBUG) || defined(LOFAR_BBS_VERBOSE)
    ostringstream oss;
    oss << "Solver statistics:";
    for(size_t i = 0; i < solutions.size(); ++i)
    {
        oss << " [" << solutions[i].id << "] " << solutions[i].rank
        << " " << solutions[i].chiSqr;
    }
    LOG_DEBUG(oss.str());
#endif
}

void LocalSolveController::getCoeff(vector<double> &result,
    const Location &cell) const
{
    result.resize(itsCoeffIndex.getCoeffCount());
    vector<double>::iterator coeffIt = result.begin();

    ParmGroup::const_iterator solIt = itsSolvables.begin();
    while(solIt != itsSolvables.end())
    {
        ParmProxy::Ptr parm(ParmManager::instance().get(*solIt));
        vector<double> coeff(parm->getCoeff(cell));
        coeffIt = copy(coeff.begin(), coeff.end(), coeffIt);
        ++solIt;
    }
}

void LocalSolveController::setCoeff(const vector<double> &coeff,
    const Location &cell) const
{
    size_t i = 0;
    ParmGroup::const_iterator solIt = itsSolvables.begin();
    while(solIt != itsSolvables.end())
    {
        ParmProxy::Ptr parm = ParmManager::instance().get(*solIt);
        const size_t nCoeff = parm->getCoeffCount();
        ASSERT(i + nCoeff <= coeff.size());

        parm->setCoeff(cell, &(coeff[i]), nCoeff);
        i += nCoeff;

        ++solIt;
    }
}

void LocalSolveController::setCoeff(const vector<double> &coeff,
    const Location &cell, const vector<unsigned int> &mapping) const
{
    vector<unsigned int>::const_iterator mapIt = mapping.begin();
    ParmGroup::const_iterator solIt = itsSolvables.begin();
    while(solIt != itsSolvables.end())
    {
        ParmProxy::Ptr parm = ParmManager::instance().get(*solIt);
        ASSERT((*mapIt) + parm->getCoeffCount() <= coeff.size());

        parm->setCoeff(cell, &(coeff[*mapIt]), parm->getCoeffCount());

        ++mapIt;
        ++solIt;
    }
}

} //# namespace BBS
} //# namespace LOFAR
