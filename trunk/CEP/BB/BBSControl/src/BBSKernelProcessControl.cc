//#  BBSKernelProcessControl.cc: 
//#
//#  Copyright (C) 2002-2007
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  Note: This source is read best with tabstop 4.
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <BBSControl/BBSKernelProcessControl.h>
#include <BBSControl/BBSStrategy.h>
#include <BBSControl/BBSStep.h>
#include <BBSControl/BBSPredictStep.h>
#include <BBSControl/BBSSubtractStep.h>
#include <BBSControl/BBSCorrectStep.h>
#include <BBSControl/BBSSolveStep.h>
#include <BBSControl/BBSSubtractStep.h>
#include <BBSControl/BlobStreamableVector.h>
#include <BBSControl/DomainRegistrationRequest.h>
#include <BBSControl/IterationRequest.h>
#include <BBSControl/IterationResult.h>

#include <BBSKernel/Solver.h>
#include <BBSKernel/BBSStatus.h>

#include <APS/ParameterSet.h>
#include <Blob/BlobStreamable.h>

#include <Common/Exceptions.h>
#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>
#include <Common/Timer.h>
#include <Common/lofar_iomanip.h>
#include <Common/lofar_smartptr.h>

#include <Transport/DH_BlobStreamable.h>
#include <Transport/TH_Socket.h>
#include <Transport/CSConnection.h>

#include <stdlib.h>

using namespace LOFAR::ACC::APS;

namespace LOFAR 
{
namespace BBS 
{
    using LOFAR::operator<<;


    //# Ensure classes are registered with the ObjectFactory.
    template class BlobStreamableVector<DomainRegistrationRequest>;
    template class BlobStreamableVector<IterationRequest>;
    template class BlobStreamableVector<IterationResult>;


    //##----   P u b l i c   m e t h o d s   ----##//
    BBSKernelProcessControl::BBSKernelProcessControl() :
        ProcessControl(),
        itsPrediffer(0)
    {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
    }

    
    tribool BBSKernelProcessControl::define()
    {
        LOG_DEBUG("BBSKernelProcessControl::define()");
        
        try {
            itsControllerConnection.reset(new BlobStreamableConnection(
                    globalParameterSet()->getString("Controller.Host"),
                    globalParameterSet()->getString("Controller.Port")));
            
            char *user = getenv("USER");
            ASSERT(user);
            string userString(user);
            itsSolverConnection.reset(new BlobStreamableConnection(
                "localhost",
                "=BBSSolver_" + userString,
                Socket::LOCAL));
        }
        catch(Exception& e)
        {
            LOG_ERROR_STR(e);
            return false;
        }

        return true;
    }


    tribool BBSKernelProcessControl::init()
    {
        LOG_DEBUG("BBSKernelProcessControl::init()");
        
        try {
            LOG_DEBUG_STR("Trying to connect to controller@" << globalParameterSet()->getString("Controller.Host") << ":" << globalParameterSet()->getString("Controller.Port"   ) << "...");

            if(!itsControllerConnection->connect())
            {
                LOG_ERROR("+ could not connect to controller");
                return false;
            }
            LOG_DEBUG("+ ok");

            LOG_DEBUG("Trying to connect to solver@localhost");
            if(!itsSolverConnection->connect())
            {
                LOG_ERROR("+ could not connect to solver");
                return false;
            }
            LOG_DEBUG("+ ok");
        }
        catch(Exception& e)
        {
            LOG_ERROR_STR(e);
            return false;
        }
        
        // All went well.
        return true;
    }


    tribool BBSKernelProcessControl::run()
    {
        LOG_DEBUG("BBSKernelProcessControl::run()");

        try
        {
            // Receive the next message
            scoped_ptr<BlobStreamable> message(itsControllerConnection->recvObject());
            if(message)
                return dispatch(message.get());
            else
                return false;
        }
        catch(Exception& e)
        {
            LOG_ERROR_STR(e);
            itsControllerConnection->sendObject(BBSStatus(BBSStatus::ERROR, e.message()));
            return false;
        }
    }


    tribool BBSKernelProcessControl::pause(const string& /*condition*/)
    {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
        LOG_WARN("Not supported");
        return false;
    }


    tribool BBSKernelProcessControl::quit()
    {
        LOG_DEBUG("BBSKernelProcessControl::quit()");
        return true;
    }


    tribool BBSKernelProcessControl::snapshot(const string& /*destination*/)
    {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
        LOG_WARN("Not supported");
        return false;
    }


    tribool BBSKernelProcessControl::recover(const string& /*source*/)
    {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
        LOG_WARN("Not supported");
        return false;
    }


    tribool BBSKernelProcessControl::reinit(const string& /*configID*/)
    {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
        LOG_WARN("Not supported");
        return false;
    }


    std::string BBSKernelProcessControl::askInfo(const string& /*keylist*/)
    {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
        return std::string("");
    }


    //##----   P r i v a t e   m e t h o d s   ----##//
    bool BBSKernelProcessControl::dispatch(const BlobStreamable *message)
    {
        // If the message contains a `strategy', handle the `strategy'.
        const BBSStrategy *strategy = dynamic_cast<const BBSStrategy*>(message);
        if(strategy)
        {
            if(handle(strategy))
            {
                itsControllerConnection->sendObject(BBSStatus(BBSStatus::OK));
                return true;
            }
            else
            {
                itsControllerConnection->sendObject(BBSStatus(BBSStatus::ERROR));
                return false;
            }
        }

        // If the message contains a `step', handle the `step'.
        const BBSStep *step = dynamic_cast<const BBSStep*>(message);
        if(step)
        {
            if(handle(step))
            {
                itsControllerConnection->sendObject(BBSStatus(BBSStatus::OK));
                return true;
            }
            else
            {
                itsControllerConnection->sendObject(BBSStatus(BBSStatus::ERROR));
                return false;
            }
        }

        // We received a message we can't handle
        ostringstream oss;
        oss << "Received message of unsupported type";
//            << itsControllerConnection.itsDataHolder->classType() << "'. Skipped.";
        LOG_WARN(oss.str());
        itsControllerConnection->sendObject(BBSStatus(BBSStatus::ERROR, oss.str()));
        return false;
    }
    
    
    bool BBSKernelProcessControl::handle(const BBSStrategy *strategy)
    {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

        try
        {
            // Pre-condition check
            ASSERT(strategy);

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
            
            // Time is specified as an offset from the start time of
            // the measurement set.
            const MeqDomain domain = itsPrediffer->getLocalDataDomain();
            LOG_DEBUG_STR("Local data domain: " << domain);
            itsRegionOfInterest[2] += domain.startY();
            itsRegionOfInterest[3] += domain.startY();
            
            if(itsRegionOfInterest[0] > domain.endX()
                || itsRegionOfInterest[2] > domain.endY())
            {
                LOG_DEBUG_STR("Region of interest does not intersect the local data domain.");
                return false;
            }

            // Clip region of interest against local data domain.
            if(itsRegionOfInterest[0] < domain.startX())
                itsRegionOfInterest[0] = domain.startX(); 
            if(itsRegionOfInterest[1] < domain.endX())
                itsRegionOfInterest[1] = domain.endX(); 

            if(itsRegionOfInterest[2] > domain.startY())
                itsRegionOfInterest[2] = domain.startY(); 
            if(itsRegionOfInterest[3] > domain.endY())
                itsRegionOfInterest[3] = domain.endY(); 
            
            // Select stations and correlations.
            return itsPrediffer->setSelection(strategy->stations(), strategy->correlation());
        }
        catch(Exception& e)
        {
            LOG_WARN_STR(e);
            return false;
        }
    }


    bool BBSKernelProcessControl::handle(const BBSStep *bs)
    {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
        
        ASSERT(itsPrediffer);

        // Currently only the size in time of the work domain size is honoured.
        int timeCount = std::max(1, (int) (0.5 + (itsRegionOfInterest[3] - itsRegionOfInterest[2]) / itsWorkDomainSize.timeInterval));
        
        double timeStep = (itsRegionOfInterest[3] - itsRegionOfInterest[2]);
        if(timeCount > 1)
            timeStep /= timeCount;
        
        bool result = true;
        double time = itsRegionOfInterest[2];
        for(int i = 0; i < timeCount && result; ++i)
        {
            ASSERT(itsPrediffer->setWorkDomain(itsRegionOfInterest[0],
                itsRegionOfInterest[1],
                time,
                time + timeStep));
            
            time = itsPrediffer->getWorkDomain().endY();
            result = false;
            {
                const BBSPredictStep* step = dynamic_cast<const BBSPredictStep*>(bs);
                if(step)
                {
                    result = handle(step);
                }
            }
                    
            {
                const BBSSubtractStep* step = dynamic_cast<const BBSSubtractStep*>(bs);
                if(step)
                {
                    result = handle(step);
                }
            }
            
            {
                const BBSCorrectStep* step = dynamic_cast<const BBSCorrectStep*>(bs);
                if(step)
                {
                    result = handle(step);
                }
            }
            
            {
                const BBSSolveStep* step = dynamic_cast<const BBSSolveStep*>(bs);
                if(step)
                {
                    result = handle(step);
                }
            }
        }
        
        return result;
    }


    bool BBSKernelProcessControl::handle(const BBSPredictStep *step)
    {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
        ASSERTSTR(itsPrediffer, "No Prediffer available.");
        ASSERTSTR(step, "Step corrupted.");

        PredictContext context;
        context.baselines = step->baselines();
        context.correlation = step->correlation();
        context.sources = step->sources();        
        context.instrumentModel = step->instrumentModels();
        context.outputColumn = step->outputData();

        // Set context.
        if(!itsPrediffer->setContext(context))
        {
            return false;
        }

        // Execute predict.
        itsPrediffer->predictVisibilities();
        return true;
    }


    bool BBSKernelProcessControl::handle(const BBSSubtractStep *step)
    {
        ASSERTSTR(itsPrediffer, "No Prediffer available.");
        ASSERTSTR(step, "Step corrupted.");

        SubtractContext context;
        context.baselines = step->baselines();
        context.correlation = step->correlation();
        context.sources = step->sources();        
        context.instrumentModel = step->instrumentModels();
        context.outputColumn = step->outputData();

        // Set context.
        if(!itsPrediffer->setContext(context))
        {
            return false;
        }

        // Execute subtract.
        itsPrediffer->subtractVisibilities();
        return true;
    }


    bool BBSKernelProcessControl::handle(const BBSCorrectStep *step)
    {
        ASSERTSTR(itsPrediffer, "No Prediffer available.");
        ASSERTSTR(step, "Step corrupted.");

        CorrectContext context;
        context.baselines = step->baselines();
        context.correlation = step->correlation();
        context.sources = step->sources();        
        context.instrumentModel = step->instrumentModels();
        context.outputColumn = step->outputData();

        // Set context.
        if(!itsPrediffer->setContext(context))
        {
            return false;
        }

        // Execute correct.
        itsPrediffer->correctVisibilities();
        return true;
    }


    bool BBSKernelProcessControl::handle(const BBSSolveStep *step)
    {
        NSTimer timer;
        
        ASSERTSTR(itsPrediffer, "No Prediffer available.");
        ASSERTSTR(step, "Step corrupted.");

        // Construct context.
        GenerateContext context;
        context.baselines = step->baselines();
        context.correlation = step->correlation();
        context.sources = step->sources();        
        context.instrumentModel = step->instrumentModels();
        context.unknowns = step->parms();
        context.excludedUnknowns = step->exclParms();

        // Create solve domains. For now this just splits the work domain in a rectangular grid,
        // with cells of size step->itsDomainSize. Should become more interesting in the near future.
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
        const vector<SolveDomainDescriptor> &descriptors = itsPrediffer->getSolveDomainDescriptors();
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
            
            LOG_DEBUG_STR("[START] Sending equations to solver and waiting for results...");
            timer.reset();
            timer.start();
            
            // Send iteration requests to the solver in one go.
            BlobStreamableVector<IterationRequest> iterationRequests;
            for(size_t i = 0; i < equations.size(); ++i)
            {
                iterationRequests.getVector().push_back(new IterationRequest(i, equations[i]));
            }
            itsSolverConnection->sendObject(iterationRequests);        
            
            BlobStreamableVector<IterationResult> *resultv = dynamic_cast<BlobStreamableVector<IterationResult>*>(itsSolverConnection->recvObject());
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

                /*
                if(result->getResultCode() != casa::LSQFit::NONREADY)
                {
                    if(result->getResultCode() == casa::LSQFit::SOLINCREMENT ||
                        result->getResultCode() == casa::LSQFit::DERIVLEVEL)
                    {
                        converged++;
                    }
                    else
                        stopped++;
                }
                */

                /*
                LOG_DEBUG_STR("+ Domain: " << result->getDomainIndex());
                //LOG_DEBUG_STR("  + result: " << result->getResultCode() <<
                //              ", " << result->getResultText());
                LOG_DEBUG_STR("  + rank: " << result->getRank() <<
                              ", chi^2: " << result->getChiSquared() <<
                              ", LM factor: " << result->getLMFactor());
                LOG_DEBUG_STR("  + unknowns: " << result->getUnknowns());
                */
                
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
                << ", Solve domains converged: " << converged << "/" << context.solveDomains.size() 
                << " (" << (((double) converged) / context.solveDomains.size() * 100.0) << "%)");
            //LOG_INFO_STR("Solve domains stopped: " << stopped << "/" << context.solveDomains.size() << " (" << (((double) stopped) / context.solveDomains.size() * 100.0) << "%)");

            delete resultv;
            //finished = (converged + stopped == context.solveDomains.size()) ||
            //    (converged>= (step->minConverged() * context.solveDomains.size()) / 100.0);
            finished = (converged == context.solveDomains.size()) || (iteration == step->maxIter());
            iteration++;
        }

        LOG_DEBUG_STR("[START] Writing solutions into parameter databases...");
        timer.reset();
        timer.start();
        
        itsPrediffer->writeParms();
        
        timer.stop();
        LOG_DEBUG_STR("[END  ] Writing solutions; " << timer);
        return true;
    }

} // namespace BBS
} // namespace LOFAR
