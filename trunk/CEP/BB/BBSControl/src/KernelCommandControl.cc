//# KernelCommandControl.cc: 
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
#include <BBSControl/KernelCommandControl.h>

#include <BBSControl/BBSStrategy.h>
#include <BBSControl/BBSStep.h>
#include <BBSControl/BBSMultiStep.h>
#include <BBSControl/BBSSingleStep.h>
#include <BBSControl/BBSPredictStep.h>
#include <BBSControl/BBSSubtractStep.h>
#include <BBSControl/BBSCorrectStep.h>
#include <BBSControl/BBSSolveStep.h>
#include <BBSControl/BBSShiftStep.h>
#include <BBSControl/BBSRefitStep.h>

#include <Common/LofarLogger.h>

namespace LOFAR
{
namespace BBS 
{

void KernelCommandControl::handle(const BBSStrategy &command)
{
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

    LOG_DEBUG("Handling a BBSStrategy");
    LOG_DEBUG_STR("Command: " << endl << command);

/*
    try
    {
        // Create a new Prediffer.
        itsPrediffer.reset(new Prediffer(strategy->dataSet(),
            strategy->inputData(),
            strategy->parmDB().localSky,
            strategy->parmDB().instrument,
            strategy->parmDB().history,
            0,
            false));

        // Store work domain size / region of interest.
        itsWorkDomainSize = strategy->domainSize();
        itsRegionOfInterest = strategy->regionOfInterest();

        const MeqDomain domain = itsPrediffer->getLocalDataDomain();

        if(itsRegionOfInterest.size() == 4)
        {
            if(itsRegionOfInterest[0] > itsRegionOfInterest[1])
            {
                double tmp = itsRegionOfInterest[0];
                itsRegionOfInterest[0] = itsRegionOfInterest[1];
                itsRegionOfInterest[1] = tmp;
            }

            if(itsRegionOfInterest[2] > itsRegionOfInterest[3])
            {
                double tmp = itsRegionOfInterest[2];
                itsRegionOfInterest[2] = itsRegionOfInterest[3];
                itsRegionOfInterest[3] = tmp;
            }

            // Time is specified as an offset from the start time of
            // the measurement set.
            itsRegionOfInterest[2] += domain.startY();
            itsRegionOfInterest[3] += domain.startY();

            if(itsRegionOfInterest[0] > domain.endX()
                || itsRegionOfInterest[1] < domain.startX()
                || itsRegionOfInterest[2] > domain.endY()
                || itsRegionOfInterest[3] < domain.startY())
            {
                LOG_DEBUG_STR("Region of interest does not intersect the
local data domain.");
                return false;
            }

            // Clip region of interest against local data domain.
            if(itsRegionOfInterest[0] < domain.startX())
                itsRegionOfInterest[0] = domain.startX();
            if(itsRegionOfInterest[1] > domain.endX())
                itsRegionOfInterest[1] = domain.endX();

            if(itsRegionOfInterest[2] < domain.startY())
                itsRegionOfInterest[2] = domain.startY();
            if(itsRegionOfInterest[3] > domain.endY())
                itsRegionOfInterest[3] = domain.endY();
        }
        else
        {
            LOG_INFO("Strategy.RegionOfInterest not specified or has
incorrect format; will use the local data domain instead.");
            itsRegionOfInterest.resize(4);
            itsRegionOfInterest[0] = domain.startX();
            itsRegionOfInterest[1] = domain.endX();
            itsRegionOfInterest[2] = domain.startY();
            itsRegionOfInterest[3] = domain.endY();
        }

        // Select stations and correlations.
        return itsPrediffer->setSelection(strategy->stations(),
strategy->correlation());
    }
    catch(Exception& e)
    {
        LOG_WARN_STR(e);
        return false;
    }*/
}


void KernelCommandControl::handle(const BBSStep &command)
{
    LOG_DEBUG("Handling a BBSStep");
    LOG_DEBUG_STR("Command: " << endl << command);
/*
    // Should go to NEXT_CHUNK
    ASSERT(itsPrediffer->setWorkDomain(itsRegionOfInterest[0],
        itsRegionOfInterest[1],
        itsRegionOfInterest[2],
        itsRegionOfInterest[3]));*/
}


void KernelCommandControl::handle(const BBSMultiStep &command)
{
    LOG_DEBUG("Handling a BBSMultiStep");
    LOG_DEBUG_STR("Command: " << endl << command);
}


void KernelCommandControl::handle(const BBSSingleStep &command)
{
    LOG_DEBUG("Handling a BBSSingleStep");
    LOG_DEBUG_STR("Command: " << endl << command);
}


void KernelCommandControl::handle(const BBSPredictStep &command)
{
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

    LOG_DEBUG("Handling a BBSPredictStep");
    LOG_DEBUG_STR("Command: " << endl << command);

    /*ASSERTSTR(itsPrediffer, "No Prediffer available.");

    PredictContext context;
    context.baselines = command->baselines();
    context.correlation = command->correlation();
    context.sources = command->sources();
    context.instrumentModel = command->instrumentModels();
    context.outputColumn = command->outputData();

    // Set context.
    if(!itsPrediffer->setContext(context))
        return false;

    // Execute predict.
    itsPrediffer->predictVisibilities();
    return true;*/
}


void KernelCommandControl::handle(const BBSSubtractStep &command)
{
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

    LOG_DEBUG("Handling a BBSSubtractStep");
    LOG_DEBUG_STR("Command: " << endl << command);

    /*ASSERTSTR(itsPrediffer, "No Prediffer available.");

    SubtractContext context;
    context.baselines = step->baselines();
    context.correlation = step->correlation();
    context.sources = step->sources();
    context.instrumentModel = step->instrumentModels();
    context.outputColumn = step->outputData();

    // Set context.
    if(!itsPrediffer->setContext(context))
        return false;

    // Execute subtract.
    itsPrediffer->subtractVisibilities();
    return true;*/
}


void KernelCommandControl::handle(const BBSCorrectStep &command)
{
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

    LOG_DEBUG("Handling a BBSCorrectStep");
    LOG_DEBUG_STR("Command: " << endl << command);

    /*ASSERTSTR(itsPrediffer, "No Prediffer available.");

    CorrectContext context;
    context.baselines = step->baselines();
    context.correlation = step->correlation();
    context.sources = step->sources();
    context.instrumentModel = step->instrumentModels();
    context.outputColumn = step->outputData();

    // Set context.
    if(!itsPrediffer->setContext(context))
        return false;

    // Execute correct.
    itsPrediffer->correctVisibilities();
    return true;*/
}


void KernelCommandControl::handle(const BBSSolveStep &command)
{
    LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

    LOG_DEBUG("Handling a BBSSolveStep");
    LOG_DEBUG_STR("Command: " << endl << command);

    /*NSTimer timer;

    ASSERTSTR(itsPrediffer, "No Prediffer available.");

    // Construct context.
    GenerateContext context;
    context.baselines = step->baselines();
    context.correlation = step->correlation();
    context.sources = step->sources();
    context.instrumentModel = step->instrumentModels();
    context.unknowns = step->parms();
    context.excludedUnknowns = step->exclParms();

    // Create solve domains. For now this just splits the work domain in a
    // rectangular grid, with cells of size step->itsDomainSize. Should become
    // more interesting in the near future.
    const MeqDomain &workDomain = itsPrediffer->getWorkDomain();

    const int freqCount = (int) ceil((workDomain.endX() -
        workDomain.startX()) / step->domainSize().bandWidth);
    const int timeCount = (int) ceil((workDomain.endY() -
        workDomain.startY()) / step->domainSize().timeInterval);

    double timeOffset = workDomain.startY();
    double timeSize = step->domainSize().timeInterval;

    int time = 0;
    while(time < timeCount)
    {
        double freqOffset = workDomain.startX();
        double freqSize = step->domainSize().bandWidth;

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
    if(!itsPrediffer->setContext(context))
    {
        return false;
    }

    // Register all solve domains with the solver.
    BlobStreamableVector<DomainRegistrationRequest> request;
    const vector<SolveDomainDescriptor> &descriptors =
        itsPrediffer->getSolveDomainDescriptors();
    for(size_t i = 0; i < descriptors.size(); ++i)
    {
        request.getVector().push_back(new DomainRegistrationRequest(
            i,
            step->epsilon(),
            step->maxIter(),
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
        itsPrediffer->generateEquations(equations);

        timer.stop();
        LOG_DEBUG_STR("[END  ] Generating normal equations; " << timer);

        LOG_DEBUG_STR("[START] Sending equations to solver and waiting for
            results...");
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
                itsPrediffer->updateSolvableParms(i, result->getUnknowns());

                // Log the updated unknowns.
                itsPrediffer->logIteration(
                    step->getName(),
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
        //    (converged>= (step->minConverged() *
        //      context.solveDomains.size()) / 100.0);
        finished = (converged == context.solveDomains.size())
            || (iteration == step->maxIter());
        iteration++;
    }

    LOG_DEBUG_STR("[START] Writing solutions into parameter databases...");
    timer.reset();
    timer.start();

    itsPrediffer->writeParms();

    timer.stop();
    LOG_DEBUG_STR("[END  ] Writing solutions; " << timer);
    return true;*/
}


void KernelCommandControl::handle(const BBSShiftStep &command)
{
    LOG_DEBUG("Handling a BBSShiftStep");
    LOG_DEBUG_STR("Command: " << endl << command);
}


void KernelCommandControl::handle(const BBSRefitStep &command)
{
    LOG_DEBUG("Handling a BBSRefitStep");
    LOG_DEBUG_STR("Command: " << endl << command);
}

} //# namespace BBS
} //# namespace LOFAR
