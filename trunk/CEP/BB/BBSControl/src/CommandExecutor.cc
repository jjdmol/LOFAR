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

#include <BBSControl/BlobStreamableConnection.h>
#include <BBSControl/CommandQueue.h>
#include <BBSControl/Messages.h>
#include <BBSControl/Types.h>

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

#include <BBSKernel/MeasurementAIPS.h>
#include <BBSKernel/Solver.h>

#include <Blob/BlobIStream.h>
#include <Blob/BlobIBufStream.h>

#include <APS/ParameterSet.h>

#include <Common/LofarLogger.h>
#include <Common/lofar_fstream.h>
#include <Common/lofar_iomanip.h>
#include <BBSControl/StreamUtil.h>
#include <Common/Timer.h>

#ifdef LOG_SOLVE_DOMAIN_STATS
#include <Common/lofar_sstream.h>
#endif

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
using LOFAR::operator<<;

CommandExecutor::~CommandExecutor()
{
}


void CommandExecutor::visit(const InitializeCommand &/*command*/)
{
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

    LOG_DEBUG("Handling an InitializeCommand");

    shared_ptr<const Strategy> strategy(itsCommandQueue->getStrategy());
    ASSERT(strategy);

    // Read MetaMeasurement file.
    try
    {
        ifstream fin(strategy->dataSet().c_str());
        BlobIBufStream bufs(fin);
        BlobIStream ins(bufs);
        ins >> itsMetaMeasurement;
    }
    catch(Exception &ex)
    {
        itsResult = CommandResult(CommandResult::ERROR, "Unable to read meta" 
            " measurement.");
        return;
    }        

    if(itsKernelId >= itsMetaMeasurement.getPartCount())
    {
        itsResult = CommandResult(CommandResult::ERROR, "Kernel id does not map"
          " to any measurement in " + strategy->dataSet());
        return;
    }
    
    try
    {
        // Open measurement.
        string path = itsMetaMeasurement.getPath(itsKernelId);
        LOG_INFO_STR("Input: " << path << "::" << strategy->inputData());

        itsInputColumn = strategy->inputData();
        itsMeasurement.reset(new MeasurementAIPS(path));
    }
    catch(Exception &ex)
    {
        itsResult = CommandResult(CommandResult::ERROR, "Unable to open"
          " measurement: " + itsMetaMeasurement.getPath(itsKernelId));
        return;
    }

    try
    {
        // Open sky model parmdb.
        LOFAR::ParmDB::ParmDBMeta skyDbMeta("aips",
            strategy->parmDB().localSky);
        LOG_INFO_STR("Sky model database: "
            << skyDbMeta.getTableName());
        itsSkyDb.reset(new LOFAR::ParmDB::ParmDB(skyDbMeta));
    }
    catch(Exception &ex)
    {
        itsResult = CommandResult(CommandResult::ERROR, "Failed to open sky"
            " model parameter database: " + strategy->parmDB().localSky);
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
        itsResult = CommandResult(CommandResult::ERROR, "Failed to open"
            " instrument model parameter database."
            + strategy->parmDB().instrument);
        return;
    }        

    // Create kernel.
    itsKernel.reset(new Prediffer(itsMeasurement, *itsSkyDb.get(),
        *itsInstrumentDb.get()));

    // Initialize the chunk selection from information in strategy.
    if(!strategy->stations().empty())
    {
        itsChunkSelection.setStations(strategy->stations());
    }
    
    Correlation correlation = strategy->correlation();
    if(!correlation.type.empty())
    {
        itsChunkSelection.setPolarizations(correlation.type);
    }
    
    if(correlation.selection == "AUTO")
    {
        itsChunkSelection.setBaselineFilter(VisSelection::AUTO);
    }
    else if(correlation.selection == "CROSS")
    {
        itsChunkSelection.setBaselineFilter(VisSelection::CROSS);
    }

    itsResult = CommandResult(CommandResult::OK, "Ok.");
}


void CommandExecutor::visit(const FinalizeCommand &/*command*/)
{
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

    LOG_DEBUG("Handling a FinalizeCommand");

    itsResult = CommandResult(CommandResult::OK, "Ok.");
}


void CommandExecutor::visit(const NextChunkCommand &command)
{
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

    LOG_DEBUG("Handling a NextChunkCommand");

    // Update chunk selection.
    itsChunkSelection.clear(VisSelection::TIME_START);
    itsChunkSelection.clear(VisSelection::TIME_END);

    pair<double, double> range = command.getTimeRange();
    itsChunkSelection.setTimeRange(range.first, range.second);

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
    LOG_INFO_STR("Chunk dimensions: " << endl << itsChunk->getDimensions());

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
    itsKernel->setModelConfig(Prediffer::SIMULATE, command.modelConfig());
        
    // Compute simulated visibilities.
    itsKernel->simulate();

    // Optionally write the simulated visibilities.
    if(!command.outputData().empty())
    {
        itsMeasurement->write(itsChunkSelection, itsChunk, command.outputData(),
            false);
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
    itsKernel->setModelConfig(Prediffer::SUBTRACT, command.modelConfig());
        
    // Subtract the simulated visibilities from the observed visibilities.
    itsKernel->subtract();

    // Optionally write the residuals.
    if(!command.outputData().empty())
    {
        itsMeasurement->write(itsChunkSelection, itsChunk, command.outputData(),
            false);
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
    itsKernel->setModelConfig(Prediffer::CORRECT, command.modelConfig());
        
    // Correct the visibilities.
    itsKernel->correct();

    // Optionally write the corrected visibilities.
    if(!command.outputData().empty())
    {
        itsMeasurement->write(itsChunkSelection, itsChunk, command.outputData(),
            false);
    }
    
    itsResult = CommandResult(CommandResult::OK, "Ok.");
}


void CommandExecutor::visit(const SolveStep &command)
{
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
    LOG_DEBUG("Handling a SolveStep");
    LOG_DEBUG_STR("Command: " << endl << command);
    
    LOG_DEBUG_STR("Solve type: " << (command.kernelGroups().empty() ? "LOCAL" 
        : "GLOBAL"));
    LOG_DEBUG_STR("Kernel ID: " << itsKernelId);
    
    if(command.kernelGroups().empty())
    {
        handleLocalSolve(command);
    }
    else
    {
        handleGlobalSolve(command);
    }
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


void CommandExecutor::handleLocalSolve(const SolveStep &command)
{
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
    itsKernel->setModelConfig(Prediffer::CONSTRUCT, command.modelConfig());

    if(!itsKernel->setSolvableParameters(command.parms(), command.exclParms()))
    {
        itsResult = CommandResult(CommandResult::ERROR, "Failed to set"
            " parameter selection.");
        return;
    }
        
    // Get measurement dimensions.
    const VisDimensions &dims = itsMeasurement->getDimensions();

    // Construct solution cell grid.
    const CellSize cellSize(command.cellSize());

    Axis::Pointer freqAxis(dims.getFreqAxis());
    if(cellSize.freq == 0)
    {
        const pair<double, double> range = dims.getFreqRange();
        freqAxis.reset(new RegularAxis(range.first, range.second - range.first,
            1));
    }
    else if(cellSize.freq > 1)
    {
        freqAxis = freqAxis->compress(cellSize.freq);
    }

    Axis::Pointer timeAxis(dims.getTimeAxis());
    if(cellSize.time == 0)
    {
        const pair<double, double> range = dims.getTimeRange();
        timeAxis.reset(new RegularAxis(range.first, range.second - range.first,
            1));
    }
    else if(cellSize.time > 1)
    {
        timeAxis = timeAxis->compress(cellSize.time);
    }

    Grid cellGrid(freqAxis, timeAxis);

    // Set solution cell grid.
    itsKernel->setCellGrid(cellGrid);
    
    // Find cells to process.
    const Grid &visGrid = itsChunk->getDimensions().getGrid();
    Box bbox = visGrid.getBoundingBox() & cellGrid.getBoundingBox();
    ASSERT(!bbox.empty());

    Location chunkStartCell = cellGrid.locate(bbox.start);
    Location chunkEndCell = cellGrid.locate(bbox.end, false);

    LOG_DEBUG_STR("Cells in chunk: [(" << chunkStartCell.first << ","
        << chunkStartCell.second << "), (" << chunkEndCell.first << ","
        << chunkEndCell.second << ")]");
    
    // Number of cells along the time axis.
    const uint nCellsTime = chunkEndCell.second - chunkStartCell.second + 1;

    // Determine the number of cells to process simultaneously.
    uint cellChunkSize = std::min(command.cellChunkSize(), nCellsTime);
    if(cellChunkSize == 0)
    {
        cellChunkSize = nCellsTime;
    }
    
    const uint nCellChunks =
        static_cast<uint>(ceil(static_cast<double>(nCellsTime)
            / cellChunkSize));

    // Initialize solver.
    Solver solver;
    SolverOptions options(command.solverOptions());
    solver.reset(options.maxIter, options.epsValue, options.epsDerivative,
        options.collFactor, options.lmFactor, options.balancedEqs,
        options.useSVD);

    // Short-wire coefficient index.
    solver.setCoeffIndex(0, itsKernel->getCoeffIndex());
    itsKernel->setCoeffIndex(solver.getCoeffIndex());
    
    vector<CellCoeff> coeff;
    vector<CellEquation> equations;
    vector<CellSolution> solutions;

#ifdef LOG_SOLVE_DOMAIN_STATS
    uint nCells = (chunkEndCell.first - chunkStartCell.first + 1)
        * (chunkEndCell.second - chunkStartCell.second + 1);
    uint nConverged = 0, nStopped = 0;
#endif

    Location startCell(chunkStartCell), endCell(chunkEndCell);
    for(uint cellChunk = 0; cellChunk < nCellChunks; ++cellChunk)
    {
        // Move to next cell chunk.
        endCell.second = std::min(startCell.second + cellChunkSize - 1,
            chunkEndCell.second);

        // Short-wire initial coefficients.
        itsKernel->getCoeff(startCell, endCell, coeff);
        solver.setCoeff(0, coeff);

        // Iterate.
        bool done = false;
        while(!done)
        {
            // Construct equations and pass to solver.
            itsKernel->construct(startCell, endCell, equations);
            solver.setEquations(0, equations);
            
            // Perform an non-linear LSQ iteration.
            done = solver.iterate(solutions);
            
            // Update coefficients.
            itsKernel->setCoeff(solutions);

#ifdef LOG_SOLVE_DOMAIN_STATS
            ostringstream oss;
            oss << "Solver statistics:";
            for(size_t i = 0; i < solutions.size(); ++i)
            {
                oss << " [" << solutions[i].id << "] " << solutions[i].rank
                << " " << solutions[i].chiSqr;
                
                if(solutions[i].result == DERIVLEVEL)
                {
                    ++nConverged;
                }
                else if(solutions[i].result != NONREADY)
                {
                    ++nStopped;
                }
            }
            LOG_DEBUG(oss.str());
#endif            
        }

#ifdef LOG_SOLVE_DOMAIN_STATS
        LOG_DEBUG_STR("" << setw(3) << static_cast<double>(nConverged) / nCells
            * 100.0 << "% converged, " << setw(3)
            << static_cast<double>(nStopped) / nCells * 100.0 << "% stopped.");
#endif
                
        // Propagate solutions to the next cell chunk.
        if(command.propagate())
        {
            Location cell;
            vector<double> initialValues;
            
            for(size_t f = startCell.first; f <= endCell.first; ++f)
            {
                itsKernel->getCoeff(Location(f, endCell.second), initialValues);

                cout << "GET: " << f << "," << endCell.second << endl;
                cout << "COEFF: " << initialValues << endl;
                for(size_t t = chunkStartCell.second + (cellChunk + 1)
                        * cellChunkSize;
                    t <= std::min(chunkEndCell.second, chunkStartCell.second
                        + (cellChunk + 2) * cellChunkSize - 1);
                    ++t)
                {
                    cout << "SET: " << f << "," << t << endl;
                    itsKernel->setCoeff(Location(f, t), initialValues);
                }
            }
        }                

        startCell.second += cellChunkSize;
    }

    itsKernel->storeParameterValues();

    itsResult = CommandResult(CommandResult::OK, "Ok.");
}


void CommandExecutor::handleGlobalSolve(const SolveStep &command)
{
    ASSERTSTR(itsKernel, "No kernel available.");
    ASSERTSTR(itsSolver, "No global solver available.");

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
    itsKernel->setModelConfig(Prediffer::CONSTRUCT, command.modelConfig());

    if(!itsKernel->setSolvableParameters(command.parms(), command.exclParms()))
    {
        itsResult = CommandResult(CommandResult::ERROR, "Failed to set"
            " parameter selection.");
        return;
    }
        
    // Get measurement dimensions.
    const VisDimensions &dims = itsMeasurement->getDimensions();

    // Construct solution cell grid.
    vector<uint32> groupEnd = command.kernelGroups();    
    partial_sum(groupEnd.begin(), groupEnd.end(), groupEnd.begin());

    size_t groupId = 0;
    while(itsKernelId >= groupEnd[groupId])
    {
        ++groupId;
    }
    ASSERT(groupId < groupEnd.size());
    LOG_DEBUG_STR("Group id: " << groupId);
    
    double freqBegin = itsMetaMeasurement.getFreqRange(groupId > 0
        ? groupEnd[groupId - 1] : 0).first;
    double freqEnd =
        itsMetaMeasurement.getFreqRange(groupEnd[groupId] - 1).second;
        
    LOG_DEBUG_STR("Group freq range: " << setprecision(15) << freqBegin << " - "
        << freqEnd);
    Axis::Pointer freqAxis(new RegularAxis(freqBegin, freqEnd - freqBegin, 1));

    const CellSize cellSize(command.cellSize());

    Axis::Pointer timeAxis = dims.getTimeAxis();
    if(cellSize.time == 0)
    {
        const pair<double, double> range = dims.getTimeRange();
        timeAxis.reset(new RegularAxis(range.first, range.second - range.first,
            1));
    }
    else if(cellSize.time > 1)
    {
        timeAxis = timeAxis->compress(cellSize.time);
    }

    Grid cellGrid(freqAxis, timeAxis);

    // Set solution cell grid.
    itsKernel->setCellGrid(cellGrid);

    // Find cells to process.
    const Grid &visGrid = itsChunk->getDimensions().getGrid();
    Box bbox = visGrid.getBoundingBox() & cellGrid.getBoundingBox();
    ASSERT(!bbox.empty());

    Location chunkStartCell = cellGrid.locate(bbox.start);
    Location chunkEndCell = cellGrid.locate(bbox.end, false);

    LOG_DEBUG_STR("Cells in chunk: [(" << chunkStartCell.first << ","
        << chunkStartCell.second << "), (" << chunkEndCell.first << ","
        << chunkEndCell.second << ")]");
    
    // Number of cells along the time axis.
    const uint nCellsTime = chunkEndCell.second - chunkStartCell.second + 1;

    // Determine the number of cells to process simultaneously.
    uint cellChunkSize = std::min(command.cellChunkSize(), nCellsTime);
    if(cellChunkSize == 0)
    {
        cellChunkSize = nCellsTime;
    }
    
    const uint nCellChunks =
        static_cast<uint>(ceil(static_cast<double>(nCellsTime)
            / cellChunkSize));

    // Send coefficient index.
    CoeffIndexMsg kernelIndexMsg(itsKernelId);
    kernelIndexMsg.getContents() = itsKernel->getCoeffIndex();
    itsSolver->sendObject(kernelIndexMsg);

    shared_ptr<BlobStreamable> solverMsg(itsSolver->recvObject());
    shared_ptr<MergedCoeffIndexMsg> solverIndexMsg(dynamic_pointer_cast<MergedCoeffIndexMsg>(solverMsg));
    
    if(!solverIndexMsg)
    {
        itsResult = CommandResult(CommandResult::ERROR, "Protocol error");
        return;
    }
    
    itsKernel->setCoeffIndex(solverIndexMsg->getContents());

#ifdef LOG_SOLVE_DOMAIN_STATS
    uint nCells = (chunkEndCell.first - chunkStartCell.first + 1)
        * (chunkEndCell.second - chunkStartCell.second + 1);
    uint nConverged = 0, nStopped = 0;
#endif

    Location startCell(chunkStartCell), endCell(chunkEndCell);
    for(uint cellChunk = 0; cellChunk < nCellChunks; ++cellChunk)
    {
        // Move to next cell chunk.
        endCell.second = std::min(startCell.second + cellChunkSize - 1,
            chunkEndCell.second);

        CoeffMsg kernelCoeffMsg(itsKernelId);
        itsKernel->getCoeff(startCell, endCell, kernelCoeffMsg.getContents());
        itsSolver->sendObject(kernelCoeffMsg);

        bool done = false;
        while(!done)
        {
            EquationMsg kernelEqMsg(itsKernelId);
            itsKernel->construct(startCell, endCell, kernelEqMsg.getContents());
            itsSolver->sendObject(kernelEqMsg);

            solverMsg.reset(itsSolver->recvObject());
            shared_ptr<SolutionMsg> solverSolutionMsg(dynamic_pointer_cast<SolutionMsg>(solverMsg));
            if(!solverSolutionMsg)
            {
                itsResult = CommandResult(CommandResult::ERROR, "Protocol error");
                return;
            }
            
            const vector<CellSolution> &solutions =
                solverSolutionMsg->getContents();

            itsKernel->setCoeff(solutions);
            
            done = true;
            for(size_t i = 0; i < solutions.size(); ++i)
            {
                done = done && (solutions[i].result != NONREADY);
            }                

#ifdef LOG_SOLVE_DOMAIN_STATS
            ostringstream oss;
            oss << "Solver statistics:";
            for(size_t i = 0; i < solutions.size(); ++i)
            {
                oss << " [" << solutions[i].id << "] " << solutions[i].rank
                << " " << solutions[i].chiSqr;
                
                if(solutions[i].result == DERIVLEVEL)
                {
                    ++nConverged;
                }
                else if(solutions[i].result != NONREADY)
                {
                    ++nStopped;
                }
            }
            LOG_DEBUG(oss.str());
#endif            
        }

#ifdef LOG_SOLVE_DOMAIN_STATS
        LOG_DEBUG_STR("" << setw(3) << static_cast<double>(nConverged) / nCells
            * 100.0 << "% converged, " << setw(3)
            << static_cast<double>(nStopped) / nCells * 100.0 << "% stopped.");
#endif
                
        // Propagate solutions to the next cell chunk.
        if(command.propagate())
        {
            Location cell;
            vector<double> initialValues;
            
            for(size_t f = startCell.first; f <= endCell.first; ++f)
            {
                itsKernel->getCoeff(Location(f, endCell.second), initialValues);

                cout << "GET: " << f << "," << endCell.second << endl;
                cout << "COEFF: " << initialValues << endl;
                for(size_t t = chunkStartCell.second + (cellChunk + 1)
                        * cellChunkSize;
                    t <= std::min(chunkEndCell.second, chunkStartCell.second
                        + (cellChunk + 2) * cellChunkSize - 1);
                    ++t)
                {
                    cout << "SET: " << f << "," << t << endl;
                    itsKernel->setCoeff(Location(f, t), initialValues);
                }
            }
        }                

        startCell.second += cellChunkSize;
    }

    itsSolver->sendObject(ChunkDoneMsg(itsKernelId));

    itsKernel->storeParameterValues();

    itsResult = CommandResult(CommandResult::OK, "Ok.");
}


} //# namespace BBS
} //# namespace LOFAR
