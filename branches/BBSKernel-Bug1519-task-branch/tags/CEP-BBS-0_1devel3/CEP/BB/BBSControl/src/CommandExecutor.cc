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
#include <BBSControl/BBSStructs.h>

#include <BBSControl/BlobStreamableVector.h>
#include <BBSControl/BlobStreamableConnection.h>
#include <BBSControl/DomainRegistrationRequest.h>
#include <BBSControl/IterationRequest.h>
#include <BBSControl/IterationResult.h>

#include <BBSControl/InitializeCommand.h>
#include <BBSControl/FinalizeCommand.h>
#include <BBSControl/NextChunkCommand.h>
#include <BBSControl/RecoverCommand.h>
#include <BBSControl/SynchronizeCommand.h>

#include <BBSControl/BBSStrategy.h>
#include <BBSControl/BBSMultiStep.h>
#include <BBSControl/BBSPredictStep.h>
#include <BBSControl/BBSSubtractStep.h>
#include <BBSControl/BBSCorrectStep.h>
#include <BBSControl/BBSSolveStep.h>
#include <BBSControl/BBSShiftStep.h>
#include <BBSControl/BBSRefitStep.h>

#include <BBSKernel/Prediffer.h>

#include <Common/LofarLogger.h>
#include <Common/Timer.h>

//#include <casa/Quanta/Quantum.h>
//#include <casa/Quanta/MVTime.h>

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
namespace BBS 
{
using LOFAR::operator<<;


//# Ensure classes are registered with the ObjectFactory.
template class BlobStreamableVector<DomainRegistrationRequest>;
template class BlobStreamableVector<IterationRequest>;
template class BlobStreamableVector<IterationResult>;


CommandExecutor::~CommandExecutor()
{
}


void CommandExecutor::visit(const InitializeCommand &/*command*/)
{
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

    LOG_DEBUG("Handling an InitializeCommand");

    shared_ptr<const BBSStrategy> strategy(itsCommandQueue->getStrategy());
    ASSERT(strategy);

    try
    {
        // Create a new kernel.
        itsKernel.reset(new Prediffer(strategy->dataSet(),
//            strategy->subband(),
            0,
            strategy->inputData(),
            strategy->parmDB().localSky,
            strategy->parmDB().instrument,
            strategy->parmDB().history,
//            strategy->readUVW()));
            false));
    }
    catch(Exception &ex)
    {
        // TODO: get exception msg and put into result msg.
        itsResult = CommandResult(CommandResult::ERROR, "Could not create"
            " kernel.");
    }

    // Create selection from information in strategy.
    VisSelection selection;
    if(!strategy->stations().empty())
        selection.setStations(strategy->stations());

    Correlation correlation = strategy->correlation();
    if(!correlation.type.empty())
        selection.setPolarizations(correlation.type);

    if(correlation.selection == "AUTO")
    {
        selection.setBaselineFilter(VisSelection::AUTO);
    }
    else if(correlation.selection == "CROSS")
    {
        selection.setBaselineFilter(VisSelection::CROSS);
    }

    RegionOfInterest roi = strategy->regionOfInterest();
    if(!roi.frequency.empty())
    {
        selection.setStartChannel(roi.frequency[0]);
    }
    if(roi.frequency.size() > 1)
    {
        selection.setEndChannel(roi.frequency[1]);
    }
    if(!roi.time.empty())
    {
        selection.setStartTime(roi.time[0]);
    }
    if(roi.time.size() > 1)
    {
        selection.setEndTime(roi.time[1]);
    }

    itsKernel->setSelection(selection);

    size_t size = strategy->domainSize().timeInterval;
    LOG_DEBUG_STR("Chunk size: " << size << " timeslot(s)");
    itsKernel->setChunkSize(size);

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

    if(itsKernel->nextChunk())
    {
        itsResult = CommandResult(CommandResult::OK, "Ok.");
    }
    else
    {
        LOG_DEBUG_STR("NextChunk: OUT_OF_DATA");
        itsResult = CommandResult(CommandResult::OUT_OF_DATA);
    }
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


void CommandExecutor::visit(const BBSStrategy &command)
{
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

    LOG_DEBUG("Handling a BBSStrategy");
    LOG_DEBUG_STR("Command: " << endl << command);

    ASSERTSTR(false, "Should not get here...");
}


void CommandExecutor::visit(const BBSMultiStep &command)
{
    LOG_DEBUG("Handling a BBSMultiStep");
    LOG_DEBUG_STR("Command: " << endl << command);

    ASSERTSTR(false, "Should not get here...");
}


void CommandExecutor::visit(const BBSPredictStep &command)
{
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

    LOG_DEBUG("Handling a BBSPredictStep");
    LOG_DEBUG_STR("Command: " << endl << command);

    ASSERTSTR(itsKernel, "No kernel available.");

    PredictContext context;
    context.baselines = command.baselines();
    context.correlation = command.correlation();
    context.sources = command.sources();
    context.instrumentModel = command.instrumentModels();
    context.outputColumn = command.outputData();

    // Set context.
    if(!itsKernel->setContext(context))
    {
        itsResult = CommandResult(CommandResult::ERROR,
            "Could not set context.");
        return;
    }

    // Execute predict.
    itsKernel->predict();
    itsResult = CommandResult(CommandResult::OK, "Ok.");
}


void CommandExecutor::visit(const BBSSubtractStep &command)
{
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

    LOG_DEBUG("Handling a BBSSubtractStep");
    LOG_DEBUG_STR("Command: " << endl << command);

    ASSERTSTR(itsKernel, "No kernel available.");

    SubtractContext context;
    context.baselines = command.baselines();
    context.correlation = command.correlation();
    context.sources = command.sources();
    context.instrumentModel = command.instrumentModels();
    context.outputColumn = command.outputData();

    // Set context.
    if(!itsKernel->setContext(context))
    {
        itsResult = CommandResult(CommandResult::ERROR,
            "Could not set context.");
        return;
    }

    // Execute subtract.
    itsKernel->subtract();
    itsResult = CommandResult(CommandResult::OK, "Ok.");
}


void CommandExecutor::visit(const BBSCorrectStep &command)
{
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

    LOG_DEBUG("Handling a BBSCorrectStep");
    LOG_DEBUG_STR("Command: " << endl << command);

    ASSERTSTR(itsKernel, "No kernel available.");

    CorrectContext context;
    context.baselines = command.baselines();
    context.correlation = command.correlation();
    context.sources = command.sources();
    context.instrumentModel = command.instrumentModels();
    context.outputColumn = command.outputData();

    // Set context.
    if(!itsKernel->setContext(context))
    {
        itsResult = CommandResult(CommandResult::ERROR,
            "Could not set context.");
        return;
    }

    // Execute correct.
    itsKernel->correct();
    itsResult = CommandResult(CommandResult::OK, "Ok.");
}


void CommandExecutor::visit(const BBSSolveStep &command)
{
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
    LOG_DEBUG("Handling a BBSSolveStep");
    LOG_DEBUG_STR("Command: " << endl << command);
    ASSERTSTR(itsKernel, "No kernel available.");

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
/*
                    // Log the updated unknowns.
                    itsKernel->logIteration(
                        command.getName(),
                        startDomain + i,
                        result->getRank(),
                        result->getChiSquared(),
                        result->getLMFactor());
*/

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

/*
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
*/
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

    itsResult = CommandResult(CommandResult::OK, "Ok.");
}


void CommandExecutor::visit(const BBSShiftStep &command)
{
    LOG_DEBUG("Handling a BBSShiftStep");
    LOG_DEBUG_STR("Command: " << endl << command);

    itsResult = CommandResult(CommandResult::ERROR, "Not yet implemented.");
}


void CommandExecutor::visit(const BBSRefitStep &command)
{
    LOG_DEBUG("Handling a BBSRefitStep");
    LOG_DEBUG_STR("Command: " << endl << command);

    itsResult = CommandResult(CommandResult::ERROR, "Not yet implemented.");
}

} //# namespace BBS
} //# namespace LOFAR
