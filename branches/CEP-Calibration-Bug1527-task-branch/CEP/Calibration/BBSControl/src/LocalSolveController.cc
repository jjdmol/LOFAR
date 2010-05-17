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

LocalSolveController::LocalSolveController(const VisEquator::Ptr &equator,
    const Solver::Ptr &solver)
    :   itsEquator(equator),
        itsSolver(solver),
        itsPropagateFlag(false),
        itsCellChunkSize(1)
{
}

void LocalSolveController::setSolutionGrid(const Grid &grid)
{
    itsEquator->setSolutionGrid(grid);
    itsSolGrid = grid;
}

void LocalSolveController::setSolvables(const vector<string> &include,
    const vector<string> &exclude)
{
    itsEquator->setSolvables(ParmManager::instance().makeSubset(include,
        exclude, itsEquator->parms()));

    itsSolvables = itsEquator->solvables();
    if(itsSolvables.empty()) {
      LOG_WARN_STR("No parameters found matching the specified inclusion and"
        " exclusion criteria.");
    }

    // Regenerate coefficient index.
    makeCoeffIndex();
}

void LocalSolveController::setPropagateSolutions(bool propagate)
{
    itsPropagateFlag = propagate;
}

void LocalSolveController::setCellChunkSize(size_t size)
{
    if(size > 0)
    {
        itsCellChunkSize = size;
    }
}

void LocalSolveController::run()
{
    if(itsSolvables.empty())
    {
        LOG_WARN_STR("No parameters selected for solving; nothing to be done.");
        return;
    }

    // Assign solution grid to solvables. This is done here instead of in
    // setSolutionGrid because the solution grid can only be set once for any
    // given parameter (after setting it once, it can only be set to exactly
    // the same grid).
    ParmManager::instance().setGrid(itsSolGrid, itsSolvables);

	// Inform solver of local coefficient index.
    itsSolver->setCoeffIndex(0, itsCoeffIndex);

    // Query solver for the global coefficient index and contruct a look-up
    // table that maps global to local indices.
    makeCoeffMapping(itsSolver->getCoeffIndex());

    // Compute the number of cell chunks to process.
    const size_t nCellChunks =
        static_cast<size_t>(ceil(static_cast<double>(itsSolGrid[TIME]->size())
            / itsCellChunkSize));

    // Compute the nominal number of solution cells in a cell chunk.
    const size_t nCellsPerChunk = std::min(itsCellChunkSize,
        itsSolGrid[TIME]->size()) * itsSolGrid[FREQ]->size();

    vector<CellCoeff> coeff;
    vector<CellEquation> equations(nCellsPerChunk);
    vector<CellSolution> solutions;

    Location chunkStart(0, 0);
    Location chunkEnd(itsSolGrid[FREQ]->size() - 1,
        itsSolGrid[TIME]->size() - 1);
    for(size_t cellChunk = 0; cellChunk < nCellChunks; ++cellChunk)
    {
        // Compute end cell of current cell chunk.
        chunkEnd.second = std::min(chunkStart.second + itsCellChunkSize - 1,
            itsSolGrid[TIME]->size() - 1);

        // Get initial coefficients.
        getInitialCoeff(coeff, chunkStart, chunkEnd);

        // Set initial coefficients.
        itsSolver->setCoeff(0, coeff.begin(), coeff.end());

        // Set cell selection.
        itsEquator->setCellSelection(chunkStart, chunkEnd);

        // Iterate.
        bool done = false;
        while(!done)
        {
            // Construct equations and pass to solver.
            vector<CellEquation>::iterator it =
                itsEquator->process(equations.begin(), equations.end());
            itsSolver->setEquations(0, equations.begin(), it);

            // Perform a non-linear LSQ iteration.
            solutions.clear();
            done = itsSolver->iterate(back_inserter(solutions));

            // Update coefficients.
            setSolution(solutions, chunkStart, chunkEnd);

            // Notify itsEquator of coefficient update.
            itsEquator->solvablesChanged();
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

void LocalSolveController::makeCoeffIndex()
{
    itsCoeffIndex.clear();

    for(ParmGroup::const_iterator sol_it = itsSolvables.begin(),
        sol_end = itsSolvables.end(); sol_it != sol_end; ++sol_it)
    {
        ParmProxy::Ptr parm = ParmManager::instance().get(*sol_it);
        itsCoeffIndex.insert(parm->getName(), parm->getCoeffCount());
    }
}

void LocalSolveController::makeCoeffMapping(const CoeffIndex &index)
{
    itsSolCoeffMapping.clear();

    for(ParmGroup::const_iterator sol_it = itsSolvables.begin(),
        sol_end = itsSolvables.end(); sol_it != sol_end; ++sol_it)
    {
        ParmProxy::Ptr parm = ParmManager::instance().get(*sol_it);

        CoeffIndex::const_iterator interval_it = index.find(parm->getName());
        ASSERT(interval_it != index.end());

        const CoeffInterval &interval = interval_it->second;
        ASSERT(parm->getCoeffCount() == interval.length);

        itsSolCoeffMapping.push_back(interval.start);
    }
}

void LocalSolveController::getInitialCoeff(vector<CellCoeff> &result,
    const Location &start, const Location &end) const
{
    const size_t nCells = (end.first - start.first + 1) * (end.second
        - start.second + 1);
    result.resize(nCells);

    CellIterator cell_it(start, end);
    vector<CellCoeff>::iterator result_it = result.begin();
    for(; !cell_it.atEnd(); ++cell_it, ++result_it)
    {
        result_it->id = itsSolGrid.getCellId(*cell_it);
        getCoeff(result_it->coeff, *cell_it);
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
    vector<double>::iterator coeff_it = result.begin();

    for(ParmGroup::const_iterator sol_it = itsSolvables.begin(),
        sol_end = itsSolvables.end(); sol_it != sol_end; ++sol_it)
    {
        ParmProxy::Ptr parm(ParmManager::instance().get(*sol_it));
        vector<double> coeff(parm->getCoeff(cell));
        coeff_it = copy(coeff.begin(), coeff.end(), coeff_it);
    }
}

void LocalSolveController::setCoeff(const vector<double> &coeff,
    const Location &cell) const
{
    size_t i = 0;
    for(ParmGroup::const_iterator sol_it = itsSolvables.begin(),
        sol_end = itsSolvables.end(); sol_it != sol_end; ++sol_it)
    {
        ParmProxy::Ptr parm = ParmManager::instance().get(*sol_it);
        const size_t nCoeff = parm->getCoeffCount();
        ASSERT(i + nCoeff <= coeff.size());

        parm->setCoeff(cell, &(coeff[i]), nCoeff);
        i += nCoeff;
    }
}

void LocalSolveController::setCoeff(const vector<double> &coeff,
    const Location &cell, const vector<unsigned int> &mapping) const
{
    vector<unsigned int>::const_iterator map_it = mapping.begin();
    for(ParmGroup::const_iterator sol_it = itsSolvables.begin(),
        sol_end = itsSolvables.end(); sol_it != sol_end; ++sol_it, ++map_it)
    {
        ParmProxy::Ptr parm = ParmManager::instance().get(*sol_it);
        ASSERT((*map_it) + parm->getCoeffCount() <= coeff.size());
        parm->setCoeff(cell, &(coeff[*map_it]), parm->getCoeffCount());
    }
}

} //# namespace BBS
} //# namespace LOFAR
