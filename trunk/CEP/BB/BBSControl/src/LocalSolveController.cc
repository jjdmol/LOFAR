//# LocalSolveController.cc: Class that controls the execution of a local solve
//# command.
//#
//# Copyright (C) 2008
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
#include <BBSControl/LocalSolveController.h>

namespace LOFAR
{
namespace BBS 
{

LocalSolveController::LocalSolveController(const VisData::Pointer &chunk,
    const Model::Pointer &model, const SolverOptions &options)
    :   itsChunk(chunk),
        itsModel(model),
        itsInitFlag(false)
{
    ASSERT(itsChunk);
    ASSERT(itsModel);
    
    itsSolver.reset(options.maxIter, options.epsValue, options.epsDerivative,
        options.colFactor, options.lmFactor, options.balancedEqs,
        options.useSVD);
}
        
LocalSolveController::~LocalSolveController()
{
    itsModel->clearPerturbedParms();
}

void LocalSolveController::init(const vector<string> &include,
    const vector<string> &exclude, const Grid &solGrid,
    const vector<baseline_t> &baselines, const vector<string> &products,
    uint cellChunkSize)
{
    ASSERTSTR(!itsInitFlag, "Controller already initialized");
    ASSERT(solGrid.size() > 0);
    ASSERT(baselines.size() > 0);
    ASSERT(products.size() > 0);
    ASSERT(cellChunkSize > 0);

    itsSolGrid = solGrid;
    itsCellChunkSize = cellChunkSize;
    
    // Parse solvable selection.
    itsSolvables = ParmManager::instance().makeSubset(include, exclude,
        itsModel->getParms());
    ASSERT(!itsSolvables.empty());

    // Assign solution grid to solvables.
    ParmManager::instance().setGrid(itsSolGrid, itsSolvables);

    // Instruct model to generate perturbed values for solvables.
    itsModel->setPerturbedParms(itsSolvables);

    // Create coefficient index.
    makeCoeffIndex(itsSolvables);

    // Initialize equator.
    itsEquator.reset(new Equator(itsChunk, itsModel, itsCoeffIndex, itsSolGrid,
        itsCellChunkSize));
    itsEquator->setSelection(baselines, products);
    
    itsInitFlag = true;
}

void LocalSolveController::run()
{
    ASSERTSTR(itsInitFlag, "Controller not initialized.");

    // Compute overlap between the solution grid and the chunk.
    const Grid &visGrid = itsChunk->getDimensions().getGrid();
    Box overlap = itsSolGrid.getBoundingBox() & visGrid.getBoundingBox();
    ASSERTSTR(!overlap.empty(), "No overlap between the solution grid and the"
        " current chunk.");

    // Find the first and last cell that intersects the chunk.
    Location startCell(itsSolGrid.locate(overlap.lower()));
    Location endCell(itsSolGrid.locate(overlap.upper(), false));
    
    // Initialize solver.
    itsSolver.setCoeffIndex(0, itsCoeffIndex);

    // Construct look-up table from Solver's coefficient index.
    makeCoeffMapping(itsSolvables, itsSolver.getCoeffIndex());

    // Compute the number of cell chunks to process.
    const uint nCellChunks =
        static_cast<uint>(ceil(static_cast<double>(endCell.second
            - startCell.second + 1) / itsCellChunkSize));

    vector<CellCoeff> coeff;
    vector<CellEquation> equations;
    vector<CellSolution> solutions;

    Location chunkStart(startCell), chunkEnd(endCell);
    for(uint cellChunk = 0; cellChunk < nCellChunks; ++cellChunk)
    {
        // Compute end cell of current cell chunk.
        chunkEnd.second = std::min(chunkStart.second + itsCellChunkSize - 1,
            endCell.second);
            
        // Get initial coefficients.
        getCoeff(coeff, chunkStart, chunkEnd);

        // Set initial coefficients.
        itsSolver.setCoeff(0, coeff);

        // Iterate.
        bool done = false;
        while(!done)
        {
            // Construct equations and pass to solver.
            itsEquator->process(equations, chunkStart, chunkEnd);
            itsSolver.setEquations(0, equations);
            
            // Perform a non-linear LSQ iteration.
            done = itsSolver.iterate(solutions);
            
            // Update coefficients.
            setCoeff(solutions, chunkStart, chunkEnd);
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
        ParmProxy::Pointer parm = ParmManager::instance().get(*solIt);
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
        ParmProxy::Pointer parm = ParmManager::instance().get(*solIt);

        CoeffIndex::const_iterator intervalIt = index.find(parm->getName());
        ASSERT(intervalIt != index.end());        

        const CoeffInterval &interval = intervalIt->second;
        ASSERT(parm->getCoeffCount() == interval.length);

        itsSolCoeffMapping.push_back(interval.start);
        ++solIt;
    }
}
	    
void LocalSolveController::getCoeff(vector<CellCoeff> &result,
    const Location &start, const Location &end) const
{
    const uint nCells = (end.first - start.first + 1)
        * (end.second - start.second + 1);
        
    result.resize(nCells);
    vector<CellCoeff>::iterator resultIt = result.begin();

    CellIterator cellIt(start, end);
    while(!cellIt.atEnd())
    {
        resultIt->id = itsSolGrid.getCellId(*cellIt);
        resultIt->coeff.resize(itsCoeffIndex.getCoeffCount());
        vector<double>::iterator coeffIt = resultIt->coeff.begin();

        ParmGroup::const_iterator solIt = itsSolvables.begin();
        while(solIt != itsSolvables.end())
        {
            ParmProxy::Pointer parm = ParmManager::instance().get(*solIt);
            vector<double> coeff(parm->getCoeff(*cellIt));
            coeffIt = copy(coeff.begin(), coeff.end(), coeffIt);
            ++solIt;
        }

        ++resultIt;
        ++cellIt;
    }
}
        
void LocalSolveController::setCoeff(const vector<CellSolution> &solutions,
    const Location &start, const Location &end) const
{
    for(size_t i = 0; i < solutions.size(); ++i)
    {
        Location cell(itsSolGrid.getCellLocation(solutions[i].id));
        ASSERT(cell.first >= start.first && cell.first <= end.first
            && cell.second >= start.second && cell.second <= end.second);

        vector<uint>::const_iterator mapIt = itsSolCoeffMapping.begin();
        ParmGroup::const_iterator solIt = itsSolvables.begin();
        while(solIt != itsSolvables.end())
        {
            ParmProxy::Pointer parm = ParmManager::instance().get(*solIt);
            ASSERT((*mapIt) + parm->getCoeffCount()
                <= solutions[i].coeff.size());

            parm->setCoeff(cell, &(solutions[i].coeff[*mapIt]),
                parm->getCoeffCount());

            ++mapIt;
            ++solIt;
        }        
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

} //# namespace BBS
} //# namespace LOFAR
