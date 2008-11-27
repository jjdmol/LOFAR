//# GlobalSolveController.cc: Class that controls the execution of a global
//# solve command.
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
#include <BBSControl/GlobalSolveController.h>
#include <BBSControl/Messages.h>

#if 0
#define NONREADY        casa::LSQFit::NONREADY
#define SOLINCREMENT    casa::LSQFit::SOLINCREMENT
#define DERIVLEVEL      casa::LSQFit::DERIVLEVEL
#else
#define NONREADY        0
#define SOLINCREMENT    1
#define DERIVLEVEL      2
#endif

namespace LOFAR
{
namespace BBS 
{

GlobalSolveController::GlobalSolveController(const KernelId &id,
    const VisData::Pointer &chunk, const Model::Pointer &model,
    const shared_ptr<BlobStreamableConnection> &solver)
    :   itsKernelId(id),
        itsChunk(chunk),
        itsModel(model),
        itsSolver(solver),
        itsInitFlag(false)
{
    ASSERT(itsChunk);
    ASSERT(itsModel);
    ASSERT(itsSolver);
}
        
GlobalSolveController::~GlobalSolveController()
{
    itsModel->clearPerturbedParms();
}

void GlobalSolveController::init(const vector<string> &include,
    const vector<string> &exclude, const Grid &solGrid,
    const vector<baseline_t> &baselines, const vector<string> &products,
    uint cellChunkSize, bool propagate)
{
    ASSERTSTR(!itsInitFlag, "Controller already initialized");
    ASSERT(solGrid.size() > 0);
    ASSERT(baselines.size() > 0);
    ASSERT(products.size() > 0);
    ASSERT(cellChunkSize > 0);

    itsPropagateFlag = propagate;
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
        itsSolGrid.nx() * itsCellChunkSize));
    itsEquator->setSelection(baselines, products);
    
    itsInitFlag = true;
}

void GlobalSolveController::run()
{
    ASSERTSTR(itsInitFlag, "Controller not initialized.");

    // Exchange coefficient index with solver.
    CoeffIndexMsg localIndexMsg(itsKernelId);
    localIndexMsg.getContents() = itsCoeffIndex;
    itsSolver->sendObject(localIndexMsg);

    shared_ptr<BlobStreamable> remoteMsg(itsSolver->recvObject());
    shared_ptr<MergedCoeffIndexMsg> remoteIndexMsg
        (dynamic_pointer_cast<MergedCoeffIndexMsg>(remoteMsg));
    ASSERTSTR(remoteIndexMsg, "Protocol error: expected MergedCoeffIndexMsg.");
    
    // Construct look-up table from Solver's coefficient index.
    makeCoeffMapping(itsSolvables, remoteIndexMsg->getContents());

    // Compute the number of cell chunks to process.
    const uint nCellChunks =
        static_cast<uint>(ceil(static_cast<double>(itsSolGrid[TIME]->size())
            / itsCellChunkSize));

    CoeffMsg localCoeffMsg(itsKernelId);
    EquationMsg localEqMsg(itsKernelId);

    Location chunkStart(0, 0);
    Location chunkEnd(itsSolGrid[FREQ]->size() - 1,
        itsSolGrid[TIME]->size() - 1);
    for(uint cellChunk = 0; cellChunk < nCellChunks; ++cellChunk)
    {
        // Compute end cell of current cell chunk.
        chunkEnd.second = std::min(chunkStart.second + itsCellChunkSize - 1,
            itsSolGrid[TIME]->size() - 1);
            
        // Send initial coefficient values.
        getInitialCoeff(localCoeffMsg.getContents(), chunkStart, chunkEnd);
        itsSolver->sendObject(localCoeffMsg);
        
        // Iterate.
        bool done = false;
        while(!done)
        {
            // Construct equations and send to the solver.
            itsEquator->process(localEqMsg.getContents(), chunkStart, chunkEnd);
            itsSolver->sendObject(localEqMsg);

            // Receive solutions from the solver.
            remoteMsg.reset(itsSolver->recvObject());
            shared_ptr<SolutionMsg> remoteSolutionMsg
                (dynamic_pointer_cast<SolutionMsg>(remoteMsg));
            ASSERTSTR(remoteSolutionMsg, "Protocol error: expected"
                " SolutionMsg.");

            const vector<CellSolution> &solutions =
                remoteSolutionMsg->getContents();

            // Update coefficients.
            setSolution(solutions, chunkStart, chunkEnd);

            // Check if iteration should be terminated.
            done = true;
            for(size_t i = 0; i < solutions.size(); ++i)
            {
                done = done && (solutions[i].result != NONREADY);
            }                
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

    itsSolver->sendObject(ChunkDoneMsg(itsKernelId));
}

void GlobalSolveController::makeCoeffIndex(const ParmGroup &solvables)
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

void GlobalSolveController::makeCoeffMapping(const ParmGroup &solvables,
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
	    
void GlobalSolveController::getInitialCoeff(vector<CellCoeff> &result,
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
        getCoeff(resultIt->coeff, *cellIt);

        ++resultIt;
        ++cellIt;
    }
}
        
void GlobalSolveController::setSolution(const vector<CellSolution> &solutions,
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

void GlobalSolveController::getCoeff(vector<double> &result,
    const Location &cell) const
{
    result.resize(itsCoeffIndex.getCoeffCount());
    vector<double>::iterator coeffIt = result.begin();

    ParmGroup::const_iterator solIt = itsSolvables.begin();
    while(solIt != itsSolvables.end())
    {
        ParmProxy::Pointer parm(ParmManager::instance().get(*solIt));
        vector<double> coeff(parm->getCoeff(cell));
        coeffIt = copy(coeff.begin(), coeff.end(), coeffIt);
        ++solIt;
    }
}

void GlobalSolveController::setCoeff(const vector<double> &coeff,
    const Location &cell) const
{
    size_t i = 0;
    ParmGroup::const_iterator solIt = itsSolvables.begin();
    while(solIt != itsSolvables.end())
    {
        ParmProxy::Pointer parm = ParmManager::instance().get(*solIt);
        const size_t nCoeff = parm->getCoeffCount();
        ASSERT(i + nCoeff <= coeff.size());

        parm->setCoeff(cell, &(coeff[i]), nCoeff);
        i += nCoeff;
        
        ++solIt;
    }        
}

void GlobalSolveController::setCoeff(const vector<double> &coeff,
    const Location &cell, const vector<uint> &mapping) const
{
    vector<uint>::const_iterator mapIt = mapping.begin();
    ParmGroup::const_iterator solIt = itsSolvables.begin();
    while(solIt != itsSolvables.end())
    {
        ParmProxy::Pointer parm = ParmManager::instance().get(*solIt);
        ASSERT((*mapIt) + parm->getCoeffCount() <= coeff.size());

        parm->setCoeff(cell, &(coeff[*mapIt]), parm->getCoeffCount());

        ++mapIt;
        ++solIt;
    }        
}

} //# namespace BBS
} //# namespace LOFAR
