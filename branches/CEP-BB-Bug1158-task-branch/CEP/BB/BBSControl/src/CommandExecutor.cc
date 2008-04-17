//# CommandExecutor.cc: 
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
#include <BBSControl/CommandExecutor.h>

#include <BBSControl/CommandQueue.h>
#include <BBSControl/Structs.h>

#include <BBSControl/BlobStreamableVector.h>
#include <BBSControl/BlobStreamableConnection.h>

#include <BBSControl/InitializeCommand.h>
#include <BBSControl/FinalizeCommand.h>
#include <BBSControl/NextChunkCommand.h>
#include <BBSControl/RecoverCommand.h>
#include <BBSControl/SynchronizeCommand.h>

#include <BBSControl/Strategy.h>
#include <BBSControl/MultiStep.h>
#include <BBSControl/PredictStep.h>
#include <BBSControl/SubtractStep.h>
#include <BBSControl/CorrectStep.h>
#include <BBSControl/SolveStep.h>
#include <BBSControl/ShiftStep.h>
#include <BBSControl/RefitStep.h>

#include <BBSKernel/Grid.h>
#include <BBSKernel/MeasurementAIPS.h>
#include <BBSKernel/Prediffer.h>

#include <Common/LofarLogger.h>
#include <BBSControl/StreamUtil.h>
#include <Common/lofar_iomanip.h>
#include <Common/Timer.h>

#include <ParmDB/ParmDBMeta.h>

#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/MVTime.h>

#if 0
#define NONREADY        casa::LSQFit::NONREADY
#define SOLINCREMENT    casa::LSQFit::SOLINCREMENT
#define DERIVLEVEL      casa::LSQFit::DERIVLEVEL
#define N_ReadyCode     casa::LSQFit::N_ReadyCode
#else
#define NONREADY        0
#define SOLINCREMENT    1
#define DERIVLEVEL      2
#define N_ReadyCode     999
#endif


namespace LOFAR
{
//using ParmDB::ParmDB;
//using ParmDB::ParmDBMeta;

namespace BBS 
{
using LOFAR::operator<<;


//# Ensure classes are registered with the ObjectFactory.
//template class BlobStreamableVector<DomainRegistrationRequest>;
//template class BlobStreamableVector<IterationRequest>;
//template class BlobStreamableVector<IterationResult>;


CommandExecutor::~CommandExecutor()
{
}


void CommandExecutor::visit(const InitializeCommand &/*command*/)
{
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

    LOG_DEBUG("Handling an InitializeCommand");

    shared_ptr<const Strategy> strategy(itsCommandQueue->getStrategy());
    ASSERT(strategy);

    try
    {
        // Open measurement.
        LOG_INFO_STR("Input: " << strategy->dataSet() << "::"
            << strategy->inputData());
        itsInputColumn = strategy->inputData();
        itsMeasurement.reset(new MeasurementAIPS(strategy->dataSet()));
    }
    catch(Exception &ex)
    {
        itsResult = CommandResult(CommandResult::ERROR,
            "Failed to open observation.");
        return;
    }        

    try
    {
        // Open sky model parmdb.
        LOFAR::ParmDB::ParmDBMeta skyDbMeta("aips", strategy->parmDB().localSky);
        LOG_INFO_STR("Sky model database: " << skyDbMeta.getTableName());
        itsSkyDb.reset(new LOFAR::ParmDB::ParmDB(skyDbMeta));
    }
    catch(Exception &ex)
    {
        itsResult = CommandResult(CommandResult::ERROR,
            "Failed to open sky model parameter database.");
        return;
    }        


    try
    {
        // Open instrument model parmdb.
        LOFAR::ParmDB::ParmDBMeta instrumentDbMeta("aips",
            strategy->parmDB().instrument);
        LOG_INFO_STR("Instrument model database: "
            << instrumentDbMeta.getTableName());
        itsInstrumentDb.reset(new LOFAR::ParmDB::ParmDB(instrumentDbMeta));
    }
    catch(Exception &ex)
    {
        itsResult = CommandResult(CommandResult::ERROR,
            "Failed to open instrument model parameter database.");
        return;
    }        

    // TODO: Determine kernel id.
    const uint32 kernelId = 0; 

    // Create kernel.
    itsKernel.reset(new Prediffer(kernelId, itsMeasurement, *itsSkyDb.get(),
        *itsInstrumentDb.get()));

    // Initialize the Region Of Interest selection from information in strategy.
    if(!strategy->stations().empty())
        itsRoiSelection.setStations(strategy->stations());

    const Correlation &correlation = strategy->correlation();
    if(!correlation.type.empty())
    {
        itsRoiSelection.setPolarizations(correlation.type);
    }
    
    if(correlation.selection == "AUTO")
    {
        itsRoiSelection.setBaselineFilter(VisSelection::AUTO);
    }
    else if(correlation.selection == "CROSS")
    {
        itsRoiSelection.setBaselineFilter(VisSelection::CROSS);
    }

    const RegionOfInterest &roi = strategy->regionOfInterest();
    if(!roi.frequency.empty())
    {
        itsRoiSelection.setStartChannel(roi.frequency[0]);
    }
    if(roi.frequency.size() > 1)
    {
        itsRoiSelection.setEndChannel(roi.frequency[1]);
    }
    if(!roi.time.empty())
    {
        itsRoiSelection.setStartTime(roi.time[0]);
    }
    if(roi.time.size() > 1)
    {
        itsRoiSelection.setEndTime(roi.time[1]);
    }

    itsRoiDimensions = itsMeasurement->getDimensions(itsRoiSelection);

    itsChunkSize = static_cast<size_t>(strategy->domainSize().timeInterval);
    if(itsChunkSize > 0)
    {
        LOG_DEBUG_STR("Chunk size: " << itsChunkSize << " time slot(s)");
    }
    else
    {
        LOG_DEBUG_STR("Chunk size: all time slot(s)");
        itsChunkSize = itsRoiDimensions.getTimeSlotCount();
    }

    itsResult = CommandResult(CommandResult::OK, "Ok.");
}


void CommandExecutor::visit(const FinalizeCommand &/*command*/)
{
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

    LOG_DEBUG("Handling a FinalizeCommand");

    //# How to notify KernelProcessControl of this?
    itsResult = CommandResult(CommandResult::OK, "Ok.");
}


void CommandExecutor::visit(const NextChunkCommand &/*command*/)
{
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

    LOG_DEBUG("Handling a NextChunkCommand");

    if(itsChunkPosition == itsRoiDimensions.getTimeSlotCount())
    {
        LOG_DEBUG_STR("NextChunk: OUT_OF_DATA");
        itsResult = CommandResult(CommandResult::OUT_OF_DATA);
        return;
    }

    // Create chunk selection.
    itsChunkSelection = itsRoiSelection;
    itsChunkSelection.clear(VisSelection::TIME_START);
    itsChunkSelection.clear(VisSelection::TIME_END);

    const Grid<double> &grid = itsRoiDimensions.getGrid();

    double start = grid[TIME]->lower(itsChunkPosition);
    itsChunkPosition += itsChunkSize;
    if(itsChunkPosition > itsRoiDimensions.getTimeSlotCount())
    {
        itsChunkPosition = itsRoiDimensions.getTimeSlotCount();
    }
    double end = grid[TIME]->upper(itsChunkPosition - 1);
    itsChunkSelection.setTimeRange(start, end);
    
    // Deallocate chunk.
    itsKernel->detachChunk();
    ASSERTSTR(itsChunk.use_count() == 0 || itsChunk.use_count() == 1,
        "itsChunk shoud be unique (or uninitialized) by now.");
    itsChunk.reset();

    LOG_DEBUG("Reading chunk...");
    try
    {
        itsChunk = itsMeasurement->read(itsChunkSelection, itsInputColumn,
            true);
    }
    catch(Exception &ex)
    {
        itsResult = CommandResult(CommandResult::ERROR, "Failed to read"
            " chunk.");
        return;                
    }

    itsKernel->attachChunk(itsChunk);

    // Display information about chunk.
    const Grid<double> &chunkGrid = itsChunk->dims.getGrid();
    pair<double, double> freqRange = chunkGrid[FREQ]->range();
    pair<double, double> timeRange = chunkGrid[TIME]->range();

    LOG_INFO_STR("Chunk: ");
    LOG_INFO_STR("  Frequency: "
        << setprecision(3) << freqRange.first / 1e6 << " MHz"
        << " - "
        << setprecision(3) << freqRange.second / 1e6 << " MHz");
    LOG_INFO_STR("  Bandwidth: "
        << setprecision(3) << (freqRange.second - freqRange.first) / 1e3
        << "kHz (" << chunkGrid[FREQ]->size() << " channels of "
        << setprecision(3)
        << (freqRange.second - freqRange.first) / chunkGrid[FREQ]->size()
        << " Hz)");
    LOG_INFO_STR("  Time:      "
        << casa::MVTime::Format(casa::MVTime::YMD, 6)
        << casa::MVTime(casa::Quantum<casa::Double>(timeRange.first, "s"))
        << " - "
        << casa::MVTime::Format(casa::MVTime::YMD, 6)
        << casa::MVTime(casa::Quantum<casa::Double>(timeRange.second, "s")));
    LOG_INFO_STR("  Duration:  "
        << setprecision(3) << (timeRange.second - timeRange.first) / 3600.0
        << " hours (" << chunkGrid[TIME]->size() << " samples of "
        << setprecision(3)
        << (timeRange.second - timeRange.first) / chunkGrid[TIME]->size()
        << " s on average)");

    itsResult = CommandResult(CommandResult::OK, "Ok.");
}


void CommandExecutor::visit(const RecoverCommand &/*command*/)
{
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

    LOG_DEBUG("Handling a RecoverCommand");
    itsResult = CommandResult(CommandResult::ERROR, "Not yet implemented.");
}


void CommandExecutor::visit(const SynchronizeCommand &/*command*/)
{
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

    LOG_DEBUG("Handling a SynchronizeCommand");
    itsResult = CommandResult(CommandResult::ERROR, "Not yet implemented.");
}


void CommandExecutor::visit(const Strategy &command)
{
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

    LOG_DEBUG("Handling a Strategy");
    LOG_DEBUG_STR("Command: " << endl << command);

    ASSERTSTR(false, "Should not get here...");
}


void CommandExecutor::visit(const MultiStep &command)
{
    LOG_DEBUG("Handling a MultiStep");
    LOG_DEBUG_STR("Command: " << endl << command);

    ASSERTSTR(false, "Should not get here...");
}


void CommandExecutor::visit(const PredictStep &command)
{
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

    LOG_DEBUG("Handling a PredictStep");
    LOG_DEBUG_STR("Command: " << endl << command);

    ASSERTSTR(itsKernel, "No kernel available.");

    // Set visibility selection.
    if(!itsKernel->setSelection(command.correlation().selection,
        command.baselines().station1, command.baselines().station2,
        command.correlation().type))
    {        
        itsResult = CommandResult(CommandResult::ERROR, "Failed to set data"
            " selection.");
        return;
    }        
        
    // Initialize model.
    itsKernel->setModelConfig(Prediffer::SIMULATE, command.instrumentModels(),
        command.sources());    

    // Simulate visibilities.
    itsKernel->simulate();
    
    // Write visibilities if required.
    if(!command.outputData().empty())
    {
        try
        {
            itsMeasurement->write(itsChunkSelection, itsChunk,
                command.outputData(), false);
        }
        catch(Exception &ex)
        {
            itsResult = CommandResult(CommandResult::ERROR, "Failed to write"
                " chunk.");
            return;                
        }
    }
    
    itsResult = CommandResult(CommandResult::OK, "Ok.");
}


void CommandExecutor::visit(const SubtractStep &command)
{
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

    LOG_DEBUG("Handling a SubtractStep");
    LOG_DEBUG_STR("Command: " << endl << command);

    ASSERTSTR(itsKernel, "No kernel available.");

    // Set visibility selection.
    if(!itsKernel->setSelection(command.correlation().selection,
        command.baselines().station1, command.baselines().station2,
        command.correlation().type))
    {        
        itsResult = CommandResult(CommandResult::ERROR, "Failed to set data"
            " selection.");
        return;
    }        
        
    // Initialize model.
    itsKernel->setModelConfig(Prediffer::SUBTRACT, command.instrumentModels(),
        command.sources());    

    // Subtract visibilities.
    itsKernel->subtract();

    // Write visibilities if required.
    if(!command.outputData().empty())
    {
        try
        {
            itsMeasurement->write(itsChunkSelection, itsChunk,
                command.outputData(), false);
        }
        catch(Exception &ex)
        {
            itsResult = CommandResult(CommandResult::ERROR, "Failed to write"
                " chunk.");
            return;                
        }
    }

    itsResult = CommandResult(CommandResult::OK, "Ok.");
}


void CommandExecutor::visit(const CorrectStep &command)
{
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

    LOG_DEBUG("Handling a CorrectStep");
    LOG_DEBUG_STR("Command: " << endl << command);

    ASSERTSTR(itsKernel, "No kernel available.");

    // Set visibility selection.
    if(!itsKernel->setSelection(command.correlation().selection,
        command.baselines().station1, command.baselines().station2,
        command.correlation().type))
    {        
        itsResult = CommandResult(CommandResult::ERROR, "Failed to set data"
            " selection.");
        return;
    }        
        
    // Initialize model.
    itsKernel->setModelConfig(Prediffer::CORRECT, command.instrumentModels(),
        command.sources());    

    // Correct visibilities.
    itsKernel->correct();

    // Write visibilities if required.
    if(!command.outputData().empty())
    {
        try
        {
            itsMeasurement->write(itsChunkSelection, itsChunk,
                command.outputData(), false);
        }
        catch(Exception &ex)
        {
            itsResult = CommandResult(CommandResult::ERROR, "Failed to write"
                " chunk.");
            return;                
        }
    }

    itsResult = CommandResult(CommandResult::OK, "Ok.");
}


void CommandExecutor::visit(const SolveStep &command)
{
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
    LOG_DEBUG("Handling a SolveStep");
    LOG_DEBUG_STR("Command: " << endl << command);

    ASSERTSTR(itsKernel, "No kernel available.");

    // Set visibility selection.
    if(!itsKernel->setSelection(command.correlation().selection,
        command.baselines().station1, command.baselines().station2,
        command.correlation().type))
    {        
        itsResult = CommandResult(CommandResult::ERROR, "Failed to set data"
            " selection.");
        return;
    }        
        
    // Initialize model.
    itsKernel->setModelConfig(Prediffer::CONSTRUCT, command.instrumentModels(),
        command.sources());    

    itsKernel->setParameterSelection(command.parms(), command.exclParms());
    
    // Construct solution grid.
    const pair<size_t, size_t> cellSize(command.domainSize().bandWidth,
        command.domainSize().timeInterval);

    // Construct frequency axis.
    Axis<double>::Pointer faxis = itsMeasurement->getFreqAxis();
    LOG_DEBUG_STR("Frequency axis: " << faxis->size() << " channels.");
    
    if(faxis->size() % cellSize.first == 0)
    {
        LOG_DEBUG_STR("Frequency axis: regular");
        const pair<double, double> range = faxis->range();
        const size_t count = faxis->size() / cellSize.first;
        const double delta = (range.second - range.first) / count;

        faxis.reset(new RegularAxis<double>(range.first, delta, count));
    }
    else
    {
        size_t i = 0;
        vector<double> fborders;

        while(i < faxis->size())
        {
            fborders.push_back(faxis->lower(i));
            i += cellSize.first;
        }        
        fborders.push_back(faxis->range().second);
        
        faxis.reset(new IrregularAxis<double>(fborders));
    }        


    // Construct time axis.
    Axis<double>::Pointer taxis = itsMeasurement->getTimeAxis();

    size_t i = 0;
    vector<double> tborders;
    while(i < taxis->size())
    {
        tborders.push_back(taxis->lower(i));
        i += cellSize.second;
    }        
    tborders.push_back(taxis->range().second);
    taxis.reset(new IrregularAxis<double>(tborders));

    // Set solution grid.
    Grid<double> grid(faxis, taxis);
    itsKernel->setCellGrid(grid);
    
    const Grid<double> &chunkGrid = itsChunk->dims.getGrid();
    Box<double> bbox = chunkGrid.getBoundingBox() & grid.getBoundingBox();
    ASSERT(!bbox.empty());

    // Find the first and last solution cell that intersect the current chunk.
    pair<Location, bool> result;
    result = grid.locate(bbox.start, true);
    ASSERT(result.second);
    Location chunkStart = result.first;

    result = grid.locate(bbox.end, false);
    ASSERT(result.second);
    Location chunkEnd = result.first;

    LOG_DEBUG_STR("Cells in chunk: [(" << chunkStart.first << ","
        << chunkStart.second << "), (" << chunkEnd.first << ","
        << chunkEnd.second << ")]");
        
    // Temporary stub for testing purposes.
    for(size_t ts = chunkStart.second; ts <= chunkEnd.second; ++ts)
    {
        Location start(chunkStart.first, ts);
        Location end(chunkEnd.first, ts);

        for(size_t it = 0; it < command.solverOptions().maxIter; ++it)
        {
            EquationMsg::Pointer eq = itsKernel->construct(start, end);
        }
    }

    
/*
    NSTimer timer;

    // Construct context.
    GenerateContext context;
    context.baselines = command.baselines();
    context.correlation = command.correlation();
    context.sources = command.sources();
    context.instrumentModel = command.instrumentModels();
    context.unknowns = command.parms();
    context.excludedUnknowns = command.exclParms();
//    if(command.domainSize().bandWidth != 0)
//    {
//        LOG_WARN_STR("Subdivision in frequency not yet implemented; setting"
//            " will be ignored.");
//    }
    context.domainSize = pair<size_t, size_t>(command.domainSize().bandWidth,
        command.domainSize().timeInterval);

    // Set context.
    if(!itsKernel->setContext(context))
    {
        itsResult = CommandResult(CommandResult::ERROR, "Could not set"
            " context.");
        return;
    }

    // Get unknowns.
    const vector<vector<double> > &domainUnknowns = itsKernel->getUnknowns();

    // Get solve domain grid size.
    pair<size_t, size_t> gridSize = itsKernel->getSolveDomainGridSize();

    size_t blockSize = 1;
    size_t blockCount =
        ceil(gridSize.second / static_cast<double>(blockSize));

    vector<casa::LSQFit> solvers(gridSize.first * blockSize);
    vector<vector<double> > border(gridSize.first);
    for(size_t i = 0; i < gridSize.first; ++i)
    {
        border[i] = domainUnknowns[i];
    }

    size_t convergedTotal = 0, stoppedTotal = 0;
    for(size_t block = 0; block < blockCount; ++block)
    {
        pair<size_t, size_t> frange(0, gridSize.first - 1);
        pair<size_t, size_t> trange(block * blockSize,
            (block + 1) * blockSize - 1);

        if(trange.second >= gridSize.second)
        {
            trange.second = gridSize.second - 1;
        }

        pair<size_t, size_t> drange(
            trange.first * gridSize.first + frange.first,
            trange.second * gridSize.first + frange.second);
        size_t nDomains = drange.second - drange.first + 1;

        LOG_DEBUG_STR("frange: [" << frange.first << ", " << frange.second
            << "]");
        LOG_DEBUG_STR("trange: [" << trange.first << ", " << trange.second
            << "]");
        LOG_DEBUG_STR("drange: [" << drange.first << ", " << drange.second
            << "]");

        LOG_DEBUG_STR("Processing domain(s) " << drange.first << " - "
            << drange.second << " (" << domainUnknowns.size()
            << " domain(s) in total)");

        LOG_DEBUG_STR("Initial values: ");
        for(size_t i = drange.first; i <= drange.second; ++i)
        {
            size_t idx = (i - drange.first) % gridSize.first;
            LOG_DEBUG_STR("Domain " << i << " Index " << idx << ": "
                << border[idx]);
        }

        // Register all solve domains with the solver.
        BlobStreamableVector<DomainRegistrationRequest> request;
        for(size_t i = drange.first; i <= drange.second; ++i)
        {
            size_t idx = (i - drange.first) % gridSize.first;
            itsKernel->updateUnknowns(i, border[idx]);
            request.getVector().push_back(new DomainRegistrationRequest(i,
                border[idx],
                command.maxIter(),
                command.epsilon()));
        }
        itsSolverConnection->sendObject(request);

        // Main iteration loop.
        bool finished = false;
        size_t iteration = 1, converged = 0, stopped = 0;
        while(!finished)
        {
            LOG_DEBUG_STR("[START] Iteration: " << iteration);
            LOG_DEBUG_STR("[START] Generating normal equations...");
            timer.reset();
            timer.start();

            // Generate normal equations.
            itsKernel->generate(make_pair(frange.first, trange.first),
                make_pair(frange.second, trange.second), solvers);

            timer.stop();
            LOG_DEBUG_STR("[ END ] Generating normal equations; " << timer);


            LOG_DEBUG_STR("[START] Sending equations to solver and waiting"
                " for results...");
            timer.reset();
            timer.start();

            // Send iteration requests to the solver in one go.
            BlobStreamableVector<IterationRequest> iterationRequests;
            for(size_t i = 0; i < nDomains; ++i)
            {
                iterationRequests.getVector().push_back(
                    new IterationRequest(drange.first + i,
                        solvers[i]));
            }
            itsSolverConnection->sendObject(iterationRequests);

            BlobStreamableVector<IterationResult> *resultv =
                dynamic_cast<BlobStreamableVector<IterationResult>*>(
                    itsSolverConnection->recvObject());

            ASSERT(resultv);

            timer.stop();
            LOG_DEBUG_STR("[ END ] Sending/waiting; " << timer);

            LOG_DEBUG_STR("[START] Processing results...");
            timer.reset();
            timer.start();

            const vector<IterationResult*> &results = resultv->getVector();

            // For each solve domain:
            //     - check for convergence
            //     - update cached values of the unknowns
            converged = stopped = 0;
            for(size_t i = 0; i < results.size(); ++i)
            {
                const IterationResult *result = results[i];
                size_t resultCode = result->getResultCode();

                if(resultCode == NONREADY)
                {
                    // Update cached values of the unknowns.
                    itsKernel->updateUnknowns(result->getDomainIndex(),
                        result->getUnknowns());

                    // Log the updated unknowns.
                    itsKernel->logIteration(
                        command.name(),
                        startDomain + i,
                        result->getRank(),
                        result->getChiSquared(),
                        result->getLMFactor());

#ifdef LOG_SOLVE_DOMAIN_STATS
                    LOG_DEBUG_STR("Domain: " << result->getDomainIndex()
                        << ", Rank: " << result->getRank()
                        << ", Chi^2: " << result->getChiSquared()
                        << ", LM factor: " << result->getLMFactor()
                        << ", Message: " << result->getResultText());
#endif
                }
                else if(resultCode == SOLINCREMENT
                        || resultCode == DERIVLEVEL
                        || resultCode == N_ReadyCode)
                {
                    converged++;
                }
                else
                {
                    stopped++;
                }
            }

            // Update border.
            for(size_t i = (blockSize - 1) * gridSize.first;
                i <= (blockSize) * gridSize.first - 1;
                ++i)
            {
                size_t idx = i % gridSize.first;

                if(results[i]->getResultCode() == NONREADY)
                {
                    // Save solution of last solve domain in this block.
                    border[idx] = results[i]->getUnknowns();
                }
            }
            delete resultv;

            timer.stop();
            LOG_DEBUG_STR("[ END ] Processing results; " << timer);
            LOG_DEBUG_STR("[ END ] Iteration: " << iteration);

            ostringstream oss;
            oss << "Block statistics: " << endl;
            oss << "    converged: " << converged << "/" << nDomains
                << " (" << (((double) converged) / nDomains * 100.0) << "%)"
                << endl;
            oss << "    stopped  : " << stopped << "/" << nDomains
                << " (" << (((double) stopped) / nDomains * 100.0) << "%)";
            LOG_DEBUG(oss.str());
            //finished = (converged + stopped ==
            // context.solveDomains.size()) || (converged>=
            //(command.minConverged() *
            //context.solveDomains.size()) / 100.0);

            finished = ((converged + stopped) == nDomains);
//                || (iteration == command.maxIter());
            iteration++;
        }

        convergedTotal += converged;
        stoppedTotal += stopped;

        ostringstream oss;
        oss << "Global statistics: " << endl;
        oss << "    converged: " << convergedTotal << "/"
            << domainUnknowns.size() << " ("
            << (((double) convergedTotal) / domainUnknowns.size() * 100.0)
            << "%)" << endl;
        oss << "    stopped  : " << stoppedTotal << "/"
            << domainUnknowns.size() << " ("
            << (((double) stoppedTotal) / domainUnknowns.size() * 100.0)
            << "%)";
        LOG_DEBUG(oss.str());

        LOG_DEBUG_STR("[START] Writing solutions into parameter"
            " databases...");
        timer.reset();
        timer.start();

        NSTimer write_timer, unlock_timer;
        itsKernel->lock();
        for(size_t i = drange.first; i <= drange.second; ++i)
        {
            write_timer.start();
            itsKernel->writeParms(i);
            write_timer.stop();
        }
        unlock_timer.start();
        itsKernel->unlock();
        unlock_timer.stop();

        LOG_DEBUG_STR("write timer: " << write_timer);
        LOG_DEBUG_STR("unlock timer: " << unlock_timer);

        timer.stop();
        LOG_DEBUG_STR("[ END ] Writing solutions; " << timer);
    }

    LOG_DEBUG_STR("[START] Writing solutions into parameter"
        " databases...");
    timer.reset();
    timer.start();

    NSTimer write_timer, unlock_timer;
    itsKernel->lock();
    itsKernel->writeParms();

    unlock_timer.start();
    itsKernel->unlock();
    unlock_timer.stop();

    LOG_DEBUG_STR("write timer: " << write_timer);
    LOG_DEBUG_STR("unlock timer: " << unlock_timer);

    timer.stop();
    LOG_DEBUG_STR("[ END ] Writing solutions; " << timer);
*/

    itsResult = CommandResult(CommandResult::OK, "Ok.");
}


void CommandExecutor::visit(const ShiftStep &command)
{
    LOG_DEBUG("Handling a ShiftStep");
    LOG_DEBUG_STR("Command: " << endl << command);

    itsResult = CommandResult(CommandResult::ERROR, "Not yet implemented.");
}


void CommandExecutor::visit(const RefitStep &command)
{
    LOG_DEBUG("Handling a RefitStep");
    LOG_DEBUG_STR("Command: " << endl << command);

    itsResult = CommandResult(CommandResult::ERROR, "Not yet implemented.");
}

} //# namespace BBS
} //# namespace LOFAR
