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
    RegionOfInterest roi = strategy->regionOfInterest();
    Correlation correlation = strategy->correlation();

    VisSelection selection;
    if(!strategy->stations().empty())
        selection.setStations(strategy->stations());

    if(!correlation.type.empty())
        selection.setCorrelations(correlation.type);

    if(correlation.selection == "AUTO")
    {
        selection.setBaselineFilter(VisSelection::AUTO);
    }
    else if(correlation.selection == "CROSS")
    {
        selection.setBaselineFilter(VisSelection::CROSS);
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

    NSTimer timer;

    ASSERTSTR(itsKernel, "No kernel available.");

    // Construct context.
    GenerateContext context;
    context.baselines = command.baselines();
    context.correlation = command.correlation();
    context.sources = command.sources();
    context.instrumentModel = command.instrumentModels();
    context.unknowns = command.parms();
    context.excludedUnknowns = command.exclParms();

/*
    pair<size_t, size_t> domainSize(command.domainSize().bandWidth,
        command.domainSize().timeInterval);

    if (domainSize.first != 0)
    {
        LOG_WARN_STR("Subdividing in frequency not support yet; setting"
            " will be ignored.");
        domainSize.first = 0;
    }

    context.domainSize = domainSize;
*/

    LOG_WARN_STR("Subdivision specfication not yet implemented; using domains"
        " of 1 timeslot.");
    context.domainSize = make_pair(0, 1);

    // Set context.
    if(!itsKernel->setContext(context))
    {
        itsResult = CommandResult(CommandResult::ERROR,
            "Could not set context.");
        return;
    }

    // Get domain descriptors.
    const vector<SolveDomainDescriptor> &descriptors =
        itsKernel->getSolveDomainDescriptors();

    size_t blockSize = 1;
    size_t nBlocks =
        ceil(descriptors.size() / static_cast<double>(blockSize));

    vector<double> solution(descriptors.front().unknowns);
    vector<casa::LSQFit> solvers(blockSize);
    size_t convergedTotal = 0, stoppedTotal = 0;
    for(size_t block = 0; block < nBlocks; ++block)
    {
        pair<size_t, size_t> domainRange(block * blockSize,
            (block + 1) * blockSize - 1);
        if(domainRange.second >= descriptors.size())
            domainRange.second = descriptors.size() - 1;
        size_t nDomains = domainRange.second - domainRange.first + 1;

        LOG_DEBUG_STR("Processing domain(s) " << domainRange.first << " - "
            << domainRange.second << " (" << descriptors.size()
            << " domain(s) in total)");
        LOG_DEBUG_STR("Initial values: " << solution);

        // Register all solve domains with the solver.
        BlobStreamableVector<DomainRegistrationRequest> request;
        for(size_t i = domainRange.first; i <= domainRange.second; ++i)
        {
            itsKernel->updateSolvableParms(i, solution);
            request.getVector().push_back(new DomainRegistrationRequest(i,
                solution,
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
            itsKernel->generate(make_pair(0, domainRange.first),
                make_pair(0, domainRange.second), solvers);

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
                    new IterationRequest(domainRange.first + i,
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
                    itsKernel->updateSolvableParms(
                        result->getDomainIndex(),
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

            if(results.back()->getResultCode() == NONREADY)
            {
                // Save solution of last solve domain in this block.
                solution = results.back()->getUnknowns();
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

            finished = ((converged + stopped) == nDomains)
                || (iteration == command.maxIter());
            iteration++;
        }

        convergedTotal += converged;
        stoppedTotal += stopped;

        ostringstream oss;
        oss << "Global statistics: " << endl;
        oss << "    converged: " << convergedTotal << "/"
            << descriptors.size() << " ("
            << (((double) convergedTotal) / descriptors.size() * 100.0)
            << "%)" << endl;
        oss << "    stopped  : " << stoppedTotal << "/"
            << descriptors.size() << " ("
            << (((double) stoppedTotal) / descriptors.size() * 100.0)
            << "%)";
        LOG_DEBUG(oss.str());

        LOG_DEBUG_STR("[START] Writing solutions into parameter"
            " databases...");
        timer.reset();
        timer.start();

        for(size_t i = domainRange.first; i <= domainRange.second; ++i)
        {
            itsKernel->writeParms(i);
        }

        timer.stop();
        LOG_DEBUG_STR("[ END ] Writing solutions; " << timer);
    }

    // Try to force log4cplus to flush its buffers.
    cout << "DONE." << endl;

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
