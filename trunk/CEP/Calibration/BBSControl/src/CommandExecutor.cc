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
#include <BBSControl/NoiseStep.h>

#include <BBSControl/LocalSolveController.h>
#include <BBSControl/GlobalSolveController.h>

#include <BBSKernel/MeasurementAIPS.h>
#include <BBSKernel/ParmManager.h>
#include <BBSKernel/Evaluator.h>
#include <BBSKernel/Equator.h>
#include <BBSKernel/Solver.h>

#include <Blob/BlobIStream.h>
#include <Blob/BlobIBufStream.h>

#include <Common/ParameterSet.h>

#include <Common/LofarLogger.h>
#include <Common/lofar_fstream.h>
#include <Common/lofar_iomanip.h>
#include <BBSControl/StreamUtil.h>
#include <Common/Timer.h>

#ifdef LOG_SOLVE_DOMAIN_STATS
#include <Common/lofar_sstream.h>
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
        LOG_INFO_STR("Input: " << path << "::" << strategy->inputColumn());

        itsInputColumn = strategy->inputColumn();
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
        // Open sky model database.
        itsSourceDb.reset(new SourceDB(ParmDBMeta("casa",
            strategy->parmDB().sky)));
        ParmManager::instance().initCategory(SKY, itsSourceDb->getParmDB());
    }
    catch(Exception &ex)
    {
        itsResult = CommandResult(CommandResult::ERROR, "Failed to open sky"
            " model database: " + strategy->parmDB().sky);
        return;
    }        


    try
    {
        // Open instrument model parameter database.
        ParmManager::instance().initCategory(INSTRUMENT,
            ParmDB(ParmDBMeta("casa", strategy->parmDB().instrument)));
    }
    catch(Exception &ex)
    {
        itsResult = CommandResult(CommandResult::ERROR, "Failed to open"
            " instrument model parameter database: "
            + strategy->parmDB().instrument);
        return;
    }        

    // Create model.
    itsModel.reset(new Model(itsMeasurement->getInstrument(), *itsSourceDb,
        itsMeasurement->getPhaseCenter()));

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

    // Check preconditions. Currently it is assumed that each chunk spans the
    // frequency axis of the _entire_ (meta) measurement.
    const pair<double, double> freqRangeCmd(command.getFreqRange());
    const VisDimensions &dimsObs = itsMeasurement->getDimensions();
    const pair<double, double> freqRangeObs(dimsObs.getFreqRange());
    ASSERT((freqRangeObs.first >= freqRangeCmd.first
        || casa::near(freqRangeObs.first, freqRangeCmd.first))
        && (freqRangeObs.second <= freqRangeCmd.second)
        || casa::near(freqRangeObs.second, freqRangeCmd.second));

    // Update domain.
    const pair<double, double> timeRangeCmd(command.getTimeRange());
    itsDomain = Box(Point(freqRangeCmd.first, timeRangeCmd.first),
        Point(freqRangeCmd.second, timeRangeCmd.second));

    // Notify ParmManager. NB: The domain from the NextChunkCommand is used for
    // parameters. This domain spans the entire meta measurement in frequency
    // (even though locally visibility data is available for only a small part
    // of this domain).
    ParmManager::instance().setDomain(itsDomain);
    
    // Update chunk selection.
    itsChunkSelection.clear(VisSelection::TIME_START);
    itsChunkSelection.clear(VisSelection::TIME_END);
    itsChunkSelection.setTimeRange(timeRangeCmd.first, timeRangeCmd.second);

    // Deallocate chunk.
    ASSERTSTR(itsChunk.use_count() == 0 || itsChunk.use_count() == 1,
        "Chunk shoud be unique (or uninitialized) by now.");
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

    ASSERTSTR(itsChunk, "No visibility data available.");
    ASSERTSTR(itsModel, "No model available.");

    // Parse visibility selection.
    vector<baseline_t> baselines;
    vector<string> products;
    
    if(!(parseBaselineSelection(baselines, command)
        && parseProductSelection(products, command)))
    {        
        itsResult = CommandResult(CommandResult::ERROR, "Unable to parse"
            " visibility selection.");
        return;
    }        
        
    // Initialize model.
    if(!itsModel->makeFwdExpressions(command.modelConfig(), baselines))
    {
        itsResult = CommandResult(CommandResult::ERROR, "Unable to initialize"
            " model.");
        return;
    }
        
    // Compute simulated visibilities.
    Evaluator evaluator(itsChunk, itsModel);
    evaluator.setSelection(baselines, products);
    evaluator.process(Evaluator::ASSIGN);

    // De-initialize model.
    itsModel->clearExpressions();

    // Optionally write the simulated visibilities.
    if(!command.outputColumn().empty())
    {
        itsMeasurement->write(itsChunkSelection, itsChunk,
            command.outputColumn(), false);
    }

    itsResult = CommandResult(CommandResult::OK, "Ok.");
}


void CommandExecutor::visit(const SubtractStep &command)
{
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

    LOG_DEBUG("Handling a SubtractStep");
    LOG_DEBUG_STR("Command: " << endl << command);

    ASSERTSTR(itsChunk, "No visibility data available.");
    ASSERTSTR(itsModel, "No model available.");

    // Parse visibility selection.
    vector<baseline_t> baselines;
    vector<string> products;
    
    if(!(parseBaselineSelection(baselines, command)
        && parseProductSelection(products, command)))
    {        
        itsResult = CommandResult(CommandResult::ERROR, "Unable to parse"
            " visibility selection.");
        return;
    }        
        
    // Initialize model.
    if(!itsModel->makeFwdExpressions(command.modelConfig(), baselines))
    {
        itsResult = CommandResult(CommandResult::ERROR, "Unable to initialize"
            " model.");
        return;
    }
        
    // Compute simulated visibilities.
    Evaluator evaluator(itsChunk, itsModel);
    evaluator.setSelection(baselines, products);
    evaluator.process(Evaluator::SUBTRACT);

    // De-initialize model.
    itsModel->clearExpressions();

    // Optionally write the simulated visibilities.
    if(!command.outputColumn().empty())
    {
        itsMeasurement->write(itsChunkSelection, itsChunk,
            command.outputColumn(), false);
    }

    itsResult = CommandResult(CommandResult::OK, "Ok.");
}


void CommandExecutor::visit(const CorrectStep &command)
{
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

    LOG_DEBUG("Handling a CorrectStep");
    LOG_DEBUG_STR("Command: " << endl << command);

    ASSERTSTR(itsChunk, "No visibility data available.");
    ASSERTSTR(itsModel, "No model available.");

    // Parse visibility selection.
    vector<baseline_t> baselines;
    vector<string> products;
    
    if(!(parseBaselineSelection(baselines, command)
        && parseProductSelection(products, command)))
    {        
        itsResult = CommandResult(CommandResult::ERROR, "Unable to parse"
            " visibility selection.");
        return;
    }        
        
    // Initialize model.
    if(!itsModel->makeInvExpressions(command.modelConfig(), itsChunk,
        baselines))
    {
        itsResult = CommandResult(CommandResult::ERROR, "Unable to initialize"
            " model.");
        return;
    }
        
    // Compute simulated visibilities.
    Evaluator evaluator(itsChunk, itsModel);
    evaluator.setSelection(baselines, products);
    evaluator.process(Evaluator::ASSIGN);

    // De-initialize model.
    itsModel->clearExpressions();

    // Optionally write the simulated visibilities.
    if(!command.outputColumn().empty())
    {
        itsMeasurement->write(itsChunkSelection, itsChunk,
            command.outputColumn(), false);
    }

    itsResult = CommandResult(CommandResult::OK, "Ok.");
}


void CommandExecutor::visit(const SolveStep &command)
{
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
    LOG_DEBUG("Handling a SolveStep");
    LOG_DEBUG_STR("Command: " << endl << command);
    
    ASSERTSTR(itsChunk, "No visibility data available.");
    ASSERTSTR(itsModel, "No model available.");

    // Parse visibility selection.
    vector<baseline_t> baselines;
    vector<string> products;
    
    if(!(parseBaselineSelection(baselines, command)
        && parseProductSelection(products, command)))
    {        
        itsResult = CommandResult(CommandResult::ERROR, "Unable to parse"
            " visibility selection.");
        return;
    }        
        
    // Initialize model.
    if(!itsModel->makeFwdExpressions(command.modelConfig(), baselines))
    {
        itsResult = CommandResult(CommandResult::ERROR, "Unable to initialize"
            " model.");
        return;
    }
    
    if(command.calibrationGroups().empty())
    {
        // Construct solution grid.
        const CellSize &cellSize(command.cellSize());

        Axis::ShPtr freqAxis(itsMetaMeasurement.getFreqAxis(itsKernelId));
        if(cellSize.freq == 0)
        {
            const pair<double, double> range = freqAxis->range();
            freqAxis.reset(new RegularAxis(range.first, range.second
                - range.first, 1));
        }
        else if(cellSize.freq > 1)
        {
            freqAxis = freqAxis->compress(cellSize.freq);
        }

        Axis::ShPtr timeAxis(itsMetaMeasurement.getTimeAxis());
        const size_t timeStart = timeAxis->locate(itsDomain.lowerY());
        const size_t timeEnd = timeAxis->locate(itsDomain.upperY(), false);
        ASSERT(timeStart <= timeEnd && timeEnd < timeAxis->size());
        timeAxis = timeAxis->subset(timeStart, timeEnd);

        if(cellSize.time == 0)
        {
            const pair<double, double> range = timeAxis->range();
            timeAxis.reset(new RegularAxis(range.first, range.second
                - range.first, 1));
        }
        else if(cellSize.time > 1)
        {
            timeAxis = timeAxis->compress(cellSize.time);
        }

        Grid grid(freqAxis, timeAxis);

        // Determine the number of cells to process simultaneously.
        uint cellChunkSize = command.cellChunkSize() == 0 ? grid[TIME]->size()
            : command.cellChunkSize();

        try
        {
            LocalSolveController controller(itsChunk, itsModel,
                command.solverOptions());

            controller.init(command.parms(), command.exclParms(), grid,
                baselines, products, cellChunkSize, command.propagate());

            controller.run();

            // Store solutions to disk.
            // TODO: Revert solutions on failure?
            ParmManager::instance().flush();
    
            itsResult = CommandResult(CommandResult::OK, "Ok.");
        }
        catch(Exception &ex)
        {
            itsResult = CommandResult(CommandResult::ERROR, "Unable to"
                " initialize or run local solve controller.");
        }
    }
    else
    {
        // Construct solution grid.
        const CellSize &cellSize(command.cellSize());

        // Determine group id.
        vector<uint32> groupEnd = command.calibrationGroups();
        partial_sum(groupEnd.begin(), groupEnd.end(), groupEnd.begin());
        const size_t groupId = upper_bound(groupEnd.begin(), groupEnd.end(),
            itsKernelId) - groupEnd.begin();
        ASSERT(groupId < groupEnd.size());
        LOG_DEBUG_STR("Group id: " << groupId);
        
        double freqBegin = itsMetaMeasurement.getFreqRange(groupId > 0
            ? groupEnd[groupId - 1] : 0).first;
        double freqEnd =
            itsMetaMeasurement.getFreqRange(groupEnd[groupId] - 1).second;
            
        LOG_DEBUG_STR("Group freq range: " << setprecision(15) << freqBegin << " - "
            << freqEnd);
        Axis::ShPtr freqAxis(new RegularAxis(freqBegin, freqEnd - freqBegin,
            1));

        Axis::ShPtr timeAxis(itsMetaMeasurement.getTimeAxis());
        const size_t timeStart = timeAxis->locate(itsDomain.lowerY());
        const size_t timeEnd = timeAxis->locate(itsDomain.upperY(), false);
        ASSERT(timeStart <= timeEnd && timeEnd < timeAxis->size());
        timeAxis = timeAxis->subset(timeStart, timeEnd);

        if(cellSize.time == 0)
        {
            const pair<double, double> range = timeAxis->range();
            timeAxis.reset(new RegularAxis(range.first, range.second
                - range.first, 1));
        }
        else if(cellSize.time > 1)
        {
            timeAxis = timeAxis->compress(cellSize.time);
        }

        Grid grid(freqAxis, timeAxis);

        // Determine the number of cells to process simultaneously.
        uint cellChunkSize = command.cellChunkSize() == 0 ? grid[TIME]->size()
            : command.cellChunkSize();

        try
        {
            GlobalSolveController controller(itsKernelId, itsChunk, itsModel,
                itsGlobalSolver);

            controller.init(command.parms(), command.exclParms(), grid,
                baselines, products, cellChunkSize, command.propagate());

            controller.run();

            // Store solutions to disk.
            // TODO: Revert solutions on failure?
            ParmManager::instance().flush();
    
            itsResult = CommandResult(CommandResult::OK, "Ok.");
        }
        catch(Exception &ex)
        {
            itsResult = CommandResult(CommandResult::ERROR, "Unable to"
                " initialize or run global solve controller.");
        }
    }

    // De-initialize model.
    itsModel->clearExpressions();
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


void CommandExecutor::visit(const NoiseStep &command)
{
    LOG_DEBUG("Handling a NoiseStep");
    LOG_DEBUG_STR("Command: " << endl << command);

    itsResult = CommandResult(CommandResult::ERROR, "Not yet implemented.");

/*
    try
    {
        itsNoiseGenerator.setDistributionParms(command.mean(), command.sigma());
        itsNoiseGenerator.attachChunk(itsChunk);
        itsNoiseGenerator.process();
        itsNoiseGenerator.detachChunk();

        // Optionally write visibilities.
        if(!command.outputData().empty())
        {
            itsMeasurement->write(itsChunkSelection, itsChunk,
                command.outputData(), false);
        }
    }
    catch(Exception &ex)
    {
        itsResult = CommandResult(CommandResult::ERROR, "Error during"
           " processing: " + ex.message());
        return;
    }          

    itsResult = CommandResult(CommandResult::OK, "Ok.");
*/    
}


bool CommandExecutor::parseBaselineSelection(vector<baseline_t> &result,
    const Step &command)
{
    const string &filter = command.correlation().selection;
    if(!filter.empty() && filter != "AUTO" && filter != "CROSS")
    {
        LOG_ERROR_STR("Correlation.Selection should be empty or one of \"AUTO\""
            ", \"CROSS\".");
        return false;
    }

    const vector<string> &station1 = command.baselines().station1;
    const vector<string> &station2 = command.baselines().station2;
    if(station1.size() != station2.size())
    {
        LOG_ERROR("Baselines.Station1 and Baselines.Station2 should have the"
            "same length.");
        return false;
    }
    
    // Filter available baselines.
    set<baseline_t> selection;
    
    if(station1.empty())
    {
        // If no station groups are speficied, select all the baselines
        // available in the chunk that match the baseline filter.
        const VisDimensions &dims = itsChunk->getDimensions();
        const vector<baseline_t> &baselines = dims.getBaselines();
        
        vector<baseline_t>::const_iterator baselIt = baselines.begin();
        vector<baseline_t>::const_iterator baselItEnd = baselines.end();
        while(baselIt != baselItEnd)
        {
            if(filter.empty()
                || (baselIt->first == baselIt->second && filter == "AUTO")
                || (baselIt->first != baselIt->second && filter == "CROSS"))
            {
                selection.insert(*baselIt);
            }
            ++baselIt;
        }
    }
    else
    {
        vector<casa::Regex> stationRegex1(station1.size());
        vector<casa::Regex> stationRegex2(station2.size());

        try
        {
            transform(station1.begin(), station1.end(), stationRegex1.begin(),
                ptr_fun(casa::Regex::fromPattern));
            transform(station2.begin(), station2.end(), stationRegex2.begin(),
                ptr_fun(casa::Regex::fromPattern));
        }
        catch(casa::AipsError &ex)
        {
            LOG_ERROR_STR("Error parsing include/exclude pattern (exception: "
                << ex.what() << ")");
            return false;
        }

        for(size_t i = 0; i < stationRegex1.size(); ++i)
        {
            // Find the indices of all the stations of which the name matches
            // the regex specified in the context.
            set<uint> stationGroup1, stationGroup2;

            const Instrument &instrument = itsMeasurement->getInstrument();
            for(size_t i = 0; i < instrument.stations.size(); ++i)
            {
                casa::String stationName(instrument.stations[i].name);

                if(stationName.matches(stationRegex1[i]))
                {
                    stationGroup1.insert(i);
                }

                if(stationName.matches(stationRegex2[i]))
                {
                    stationGroup2.insert(i);
                }
            }

            // Generate all possible baselines (pairs) from the two groups of
            // station indices. If a baseline is available in the chunk _and_
            // matches the baseline filter, select it for processing.
            const VisDimensions &dims = itsChunk->getDimensions();

            for(set<uint>::const_iterator it1 = stationGroup1.begin();
                it1 != stationGroup1.end();
                ++it1)
            {
                for(set<uint>::const_iterator it2 = stationGroup2.begin();
                    it2 != stationGroup2.end();
                    ++it2)
                {
                    if(filter.empty()
                        || (*it1 == *it2 && filter == "AUTO")
                        || (*it1 != *it2 && filter == "CROSS"))
                    {
                        baseline_t baseline(*it1, *it2);

                        if(dims.hasBaseline(baseline))
                        {
                            selection.insert(baseline);
                        }
                    }
                }
            }
        }
    }

    // Verify that at least one baseline is selected.
    if(selection.empty())
    {
        LOG_ERROR("Baseline selection did not match any baselines in the"
            " observation.");
        return false;
    }
    
    result.resize(selection.size());
    copy(selection.begin(), selection.end(), result.begin());
    return true;
}


bool CommandExecutor::parseProductSelection(vector<string> &result,
    const Step &command)
{
    const VisDimensions &dims = itsMeasurement->getDimensions();
    const vector<string> &available = dims.getPolarizations();

    const vector<string> &products = command.correlation().type;
    if(products.empty())
    {
        result = available;
        return true;
    }

    // Select polarization products by name.
    set<string> selection;
    for(size_t i = 0; i < available.size(); ++i)
    {
        // Check if this polarization needs to be processed.
        for(size_t j = 0; j < products.size(); ++j)
        {
            if(available[i] == products[j])
            {
                selection.insert(available[i]);
                break;
            }
        }
    }

    // Verify that at least one polarization is selected.
    if(selection.empty())
    {
        LOG_ERROR("Polarization product selection did not match any"
            " polarization product in the observation.");
        return false;
    }

    // Copy selected polarizations.
    result.resize(selection.size());
    copy(selection.begin(), selection.end(), result.begin());
    return true;
}

} //# namespace BBS
} //# namespace LOFAR
