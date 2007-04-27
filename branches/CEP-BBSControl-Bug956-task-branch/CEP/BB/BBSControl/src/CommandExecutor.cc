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
#include <BBSControl/DomainRegistrationRequest.h>
#include <BBSControl/IterationRequest.h>
#include <BBSControl/IterationResult.h>

#include <BBSControl/BBSStrategy.h>
#include <BBSControl/BBSMultiStep.h>
#include <BBSControl/BBSPredictStep.h>
#include <BBSControl/BBSSubtractStep.h>
#include <BBSControl/BBSCorrectStep.h>
#include <BBSControl/BBSSolveStep.h>
#include <BBSControl/BBSShiftStep.h>
#include <BBSControl/BBSRefitStep.h>

#include <Common/LofarLogger.h>
#include <Common/Timer.h>

#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/MVTime.h>

namespace LOFAR
{
namespace BBS 
{
using LOFAR::operator<<;


//# Ensure classes are registered with the ObjectFactory.
template class BlobStreamableVector<DomainRegistrationRequest>;
template class BlobStreamableVector<IterationRequest>;
template class BlobStreamableVector<IterationResult>;


bool CommandExecutor::convertTime(string in, double &out)
{
    //# TODO: Convert from default epoch to MS epoch (as it may differ from 
    //# the default!)
    casa::Quantum<casa::Double> time;
    
    if(in.empty() || !casa::MVTime::read(time, in))
        return false;
    
    out = static_cast<double>(time.getValue("s"));
    return true;
}


void CommandExecutor::visit(const InitializeCommand &command)
{
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

    LOG_DEBUG("Handling an InitializeCommand");

    scoped_ptr<const BBSStrategy> strategy(itsCommandQueue->getStrategy());
    ASSERT(strategy);

    try
    {
        //# Create a new kernel.
        itsKernel.reset(new Prediffer(strategy->dataSet(),
            strategy->inputData(),
            strategy->parmDB().localSky,
            strategy->parmDB().instrument,
            strategy->parmDB().history,
            0,
            false));
    }
    catch(Exception& e)
    {
        LOG_WARN_STR(e);
    }

    //# Select stations and correlations.
    itsKernel->setSelection(strategy->stations(), strategy->correlation());

    //# Store chunk size.
    itsChunkSize = strategy->domainSize().timeInterval;
    
    LOG_DEBUG_STR("ChunkSize: " << itsChunkSize);

    //# Get Region Of Interest.
    RegionOfInterest roi = strategy->regionOfInterest();
    
    //# Get Local Data Domain.
    MeqDomain ldd = itsKernel->getLocalDataDomain();

    //# Parse and validate time selection.
    bool start = false, end = false;

    start = (roi.time.size() >= 1) && convertTime(roi.time[0], itsROITime[0]);
    end = (roi.time.size() >= 2) && convertTime(roi.time[1], itsROITime[1]);

    if(start && end && itsROITime[0] > itsROITime[1])
    {
        LOG_WARN("Specified start/end times are incorrect; All times will" 
            " be selected.");
        start = end = false;
    }

    if(!start || itsROITime[0] <= ldd.startY())
        itsROITime[0] = ldd.startY();

    if(!end || itsROITime[1] >= ldd.endY())
        itsROITime[1] = ldd.endY();

    //# Get frequency (channel) selection.
    itsROIFrequency[0] = itsROIFrequency[0] = -1;
    if(roi.frequency.size() >= 1)
        itsROIFrequency[0] = roi.frequency[0];
    if(roi.frequency.size() >= 2)
        itsROIFrequency[1] = roi.frequency[1];
    
    //# Reset chunk iteration.
    itsChunkPosition = itsROITime[0];
    
    LOG_DEBUG_STR("Region Of Interest:" << endl
        << "  + Channels: " << itsROIFrequency[0] << " - " << itsROIFrequency[1]
        << endl
        << "  + Time:     " << itsROITime[0] << " - " << itsROITime[1] << endl);
}


void CommandExecutor::visit(const FinalizeCommand &command)
{
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

    LOG_DEBUG("Handling a FinalizeCommand");
}


void CommandExecutor::visit(const NextChunkCommand &command)
{
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

    LOG_DEBUG("Handling a NextChunkCommand");

    //# Iteration should be moved to the kernel code. Ideally, we would only 
    //# issue an itsKernel->nextChunk() here. However, as this is entangled with 
    //# the new MS interface, we'll emulate it by setting a new local work 
    //# domain for now.
    
    bool result = itsKernel->setWorkDomain(static_cast<int>(itsROIFrequency[0]),
        static_cast<int>(itsROIFrequency[1]),
        itsChunkPosition,
        itsChunkSize);

    itsChunkPosition += itsChunkSize;
        
    if(!result)
        LOG_DEBUG_STR("NextChunk: OUT_OF_DATA");
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
//        return false;
        return;

    // Execute predict.
    itsKernel->predictVisibilities();
//    return true;
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
//        return false;
        return;

    // Execute subtract.
    itsKernel->subtractVisibilities();
//    return true;
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
//        return false;
        return;

    // Execute correct.
    itsKernel->correctVisibilities();
    //return true;
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

    // Create solve domains. For now this just splits the work domain in a
    // rectangular grid, with cells of size command.itsDomainSize. Should 
    // become more interesting in the near future.
    const MeqDomain &workDomain = itsKernel->getWorkDomain();

    const int freqCount = (int) ceil((workDomain.endX() -
        workDomain.startX()) / command.domainSize().bandWidth);
    const int timeCount = (int) ceil((workDomain.endY() -
        workDomain.startY()) / command.domainSize().timeInterval);

    double timeOffset = workDomain.startY();
    double timeSize = command.domainSize().timeInterval;

    int time = 0;
    while(time < timeCount)
    {
        double freqOffset = workDomain.startX();
        double freqSize = command.domainSize().bandWidth;

        if(timeOffset + timeSize > workDomain.endY())
        {
            timeSize = workDomain.endY() - timeOffset;
        }

        int freq = 0;
        while(freq < freqCount)
        {
            if(freqOffset + freqSize > workDomain.endX())
            {
                freqSize = workDomain.endX() - freqOffset;
            }

            context.solveDomains.push_back(MeqDomain(freqOffset,
                freqOffset + freqSize,
                timeOffset,
                timeOffset + timeSize));

            freqOffset += freqSize;
            freq++;
        }

        timeOffset += timeSize;
        time++;
    }

    // Set context.
    if(!itsKernel->setContext(context))
    {
        return;
    }

    // Register all solve domains with the solver.
    BlobStreamableVector<DomainRegistrationRequest> request;
    const vector<SolveDomainDescriptor> &descriptors =
        itsKernel->getSolveDomainDescriptors();
    for(size_t i = 0; i < descriptors.size(); ++i)
    {
        request.getVector().push_back(new DomainRegistrationRequest(
            i,
            command.epsilon(),
            command.maxIter(),
            descriptors[i].unknowns));
    }
    itsSolverConnection->sendObject(request);

    // Main iteration loop.
    bool finished = false;
    unsigned int iteration = 1;
    while(!finished)
    {
        LOG_DEBUG_STR("[START] Iteration: " << iteration);

        LOG_DEBUG_STR("[START] Generating normal equations...");
        timer.reset();
        timer.start();

        // Generate normal equations.
        vector<casa::LSQFit> equations;
        itsKernel->generateEquations(equations);

        timer.stop();
        LOG_DEBUG_STR("[END  ] Generating normal equations; " << timer);

        LOG_DEBUG_STR("[START] Sending equations to solver and waiting for"
            " results...");
        timer.reset();
        timer.start();

        // Send iteration requests to the solver in one go.
        BlobStreamableVector<IterationRequest> iterationRequests;
        for(size_t i = 0; i < equations.size(); ++i)
        {
            iterationRequests.getVector().push_back(new IterationRequest(i,
                equations[i]));
        }
        itsSolverConnection->sendObject(iterationRequests);

        BlobStreamableVector<IterationResult> *resultv =
            dynamic_cast<BlobStreamableVector<IterationResult>*>(
                itsSolverConnection->recvObject());
        ASSERT(resultv);

        timer.stop();
        LOG_DEBUG_STR("[END ] Sending/waiting; " << timer);

        LOG_DEBUG_STR("[START] Processing results...");
        timer.reset();
        timer.start();

        const vector<IterationResult*> &results = resultv->getVector();


        // For each solve domain:
        //     - wait for result
        //     - check for convergence
        //     - update cached values of the unknowns
        unsigned int converged = 0, stopped = 0;
        for(size_t i = 0; i < results.size(); ++i)
        {
            const IterationResult *result = results[i];

            // Check for convergence.
            if(result->getResultCode() != 0)
                converged++;

//                if(result->getResultCode() != casa::LSQFit::NONREADY)
//                {
//                    if(result->getResultCode() == casa::LSQFit::SOLINCREMENT
//||
//                        result->getResultCode() == casa::LSQFit::DERIVLEVEL)
//                    {
//                        converged++;
//                    }
//                    else
//                        stopped++;
//                }

//                LOG_DEBUG_STR("+ Domain: " << result->getDomainIndex());
                //LOG_DEBUG_STR("  + result: " << result->getResultCode() <<
                //              ", " << result->getResultText());
//                LOG_DEBUG_STR("  + rank: " << result->getRank() <<
//                            ", chi^2: " << result->getChiSquared() <<
//                            ", LM factor: " << result->getLMFactor());
//                LOG_DEBUG_STR("  + unknowns: " << result->getUnknowns());

            if(result->getResultCode() != 2)
            {
#ifdef LOG_SOLVE_DOMAIN_STATS
                LOG_DEBUG_STR("Domain: " << result->getDomainIndex()
                    << ", Rank: " << result->getRank()
                    << ", Chi^2: " << result->getChiSquared()
                    << ", LM factor: " << result->getLMFactor());
#endif
                // Update cached values of the unknowns.
                itsKernel->updateSolvableParms(i, result->getUnknowns());

                // Log the updated unknowns.
                itsKernel->logIteration(
                    command.getName(),
                    i,
                    result->getRank(),
                    result->getChiSquared(),
                    result->getLMFactor());
            }
#ifdef LOG_SOLVE_DOMAIN_STATS
            else
                LOG_DEBUG_STR("Domain: " << result->getDomainIndex()
                    << " - Already converged");
#endif
        }
        timer.stop();
        LOG_DEBUG_STR("[END  ] Processing results; " << timer);

        LOG_DEBUG_STR("[END  ] Iteration: " << iteration
            << ", Solve domains converged: " << converged << "/"
            << context.solveDomains.size()
            << " (" << (((double) converged) / context.solveDomains.size() *
            100.0) << "%)");
        //LOG_INFO_STR("Solve domains stopped: " << stopped << "/" <<
        //context.solveDomains.size() << " (" << (((double) stopped) /
        //context.solveDomains.size() * 100.0) << "%)");

        delete resultv;
        //finished = (converged + stopped == context.solveDomains.size()) ||
        //    (converged>= (command.minConverged() *
        //      context.solveDomains.size()) / 100.0);
        finished = (converged == context.solveDomains.size())
            || (iteration == command.maxIter());
        iteration++;
    }

    LOG_DEBUG_STR("[START] Writing solutions into parameter databases...");
    timer.reset();
    timer.start();

    itsKernel->writeParms();

    timer.stop();
    LOG_DEBUG_STR("[END  ] Writing solutions; " << timer);
    //return true;
}


void CommandExecutor::visit(const BBSShiftStep &command)
{
    LOG_DEBUG("Handling a BBSShiftStep");
    LOG_DEBUG_STR("Command: " << endl << command);
}


void CommandExecutor::visit(const BBSRefitStep &command)
{
    LOG_DEBUG("Handling a BBSRefitStep");
    LOG_DEBUG_STR("Command: " << endl << command);
}

} //# namespace BBS
} //# namespace LOFAR
