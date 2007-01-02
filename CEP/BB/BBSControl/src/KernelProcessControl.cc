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
//#  $Id: 

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
#include <BBSKernel/BBSKernelStructs.h>
#include <BBSKernel/Prediffer.h>
#include <BBSKernel/Solver.h>
#include <BBSKernel/BBSStatus.h>
#include <ParmDB/ParmDB.h>
#include <ParmDB/ParmDBMeta.h>
#include <APS/ParameterSet.h>
#include <Transport/DH_BlobStreamable.h>
#include <Transport/TH_Socket.h>
#include <Transport/CSConnection.h>
#include <Common/Exceptions.h>
#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>
#include <Common/lofar_iomanip.h>
#include <Common/lofar_smartptr.h>

using namespace LOFAR::ACC::APS;

namespace LOFAR 
{
namespace BBS 
{
    using LOFAR::operator<<;


    //##----   P u b l i c   m e t h o d s   ----##//

    BBSKernelProcessControl::BBSKernelProcessControl() :
        ProcessControl(),
        itsPrediffer(0), 
        itsHistory(0),
        itsDataHolder(0), 
        itsTransportHolder(0), 
        itsConnection(0)
    {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
    }


    BBSKernelProcessControl::~BBSKernelProcessControl()
    {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
        delete itsPrediffer;
        delete itsHistory;
        delete itsDataHolder;
        delete itsTransportHolder;
        delete itsConnection;
    }


    tribool BBSKernelProcessControl::define()
    {
        LOG_INFO("BBSKernelProcessControl::define()");
        try {
            // Create a new data holder.
            itsDataHolder = new DH_BlobStreamable();

            // Create a new client TH_Socket. Do not connect yet.
            itsTransportHolder = new TH_Socket(globalParameterSet()->getString("Controller.Host"),
            globalParameterSet()->getString("Controller.Port"),
                                            true,         // sync
                                            Socket::TCP,  // protocol
                                            false);       // open socket now
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
        LOG_INFO("BBSKernelProcessControl::init()");
        try {
            // DH_BlobStreamable is initialized implicitly by its constructor.
            ASSERT(itsDataHolder);
            if(!itsDataHolder->isInitialized())
            {
                LOG_ERROR("Initialization of DataHolder failed");
                return false;
            }
    
            // Connect the client socket to the server.
            ASSERT(itsTransportHolder);
            if(!itsTransportHolder->init())
            {
                LOG_ERROR("Initialization of TransportHolder failed");
                return false;
            }
    
            // Create a new CSConnection object.
            itsConnection = new CSConnection("RWConn", 
                                             itsDataHolder, 
                                             itsDataHolder, 
                                             itsTransportHolder);
            if(!itsConnection || !itsConnection->isConnected())
            {
                LOG_ERROR("Creation of Connection failed");
                return false;
            }
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
        LOG_INFO("BBSKernelProcessControl::run()");

      try {
        // Receive the next message
        scoped_ptr<BlobStreamable> msg(recvObject());
	if (!msg) return false;

	// If the message contains a `strategy', handle the `strategy'.
	BBSStrategy* strategy = dynamic_cast<BBSStrategy*>(msg.get());
	if (strategy) {
          if (handle(strategy)) {
            sendObject(BBSStatus(BBSStatus::OK));
            return true;
          }
          else {
            sendObject(BBSStatus(BBSStatus::ERROR));
            return false;
          }
        }

	// If the message contains a `step', handle the `step'.
	BBSStep* step = dynamic_cast<BBSStep*>(msg.get());
	if (step) {
          if (handle(step)) {
            BBSStatus sts(BBSStatus::OK);
            LOG_DEBUG_STR("Sending BBSStatus: " << sts);
            sendObject(BBSStatus(BBSStatus::OK));
            return true;
          }
          else {
            sendObject(BBSStatus(BBSStatus::ERROR));
            return false;
          }
        }

	// We received a message we can't handle
        ostringstream oss;
        oss << "Received message of unsupported type `" 
            << itsDataHolder->classType() << "'. Skipped.";
	LOG_WARN(oss.str());
        sendObject(BBSStatus(BBSStatus::ERROR, oss.str()));
	return false;

      }
      catch (Exception& e) {
	LOG_ERROR_STR(e);
        sendObject(BBSStatus(BBSStatus::ERROR, e.message()));
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
        LOG_INFO("BBSKernelProcessControl::quit()");
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


    bool BBSKernelProcessControl::handle(const BBSStrategy *strategy)
    {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

        try
        {
            // Pre-condition check
            ASSERT(strategy);

            delete itsHistory;
            itsHistory = 0;
            if(!strategy->parmDB().history.empty())
            {
                LOFAR::ParmDB::ParmDBMeta historyDBMeta("aips", strategy->parmDB().history);
                itsHistory = new LOFAR::ParmDB::ParmDB(historyDBMeta);
            }

            // Create a new Prediffer.
            delete itsPrediffer;
            itsPrediffer = new Prediffer(strategy->dataSet(),
                                         strategy->inputData(),
                                         strategy->parmDB().localSky,
                                         strategy->parmDB().instrument,
                                         0,
                                         false);

            // Store work domain size.
            itsWorkDomainSize = strategy->domainSize();

            // Select stations and correlations.
            return itsPrediffer->setSelection(strategy->stations(), strategy->correlation());
//                    && itsPrediffer->setWorkDomainSize(workDomainSize.bandWidth, workDomainSize.timeInterval));
//                    && itsPrediffer->setWorkDomain(-1, -1, 0, 1e12));
//                    && itsPrediffer->setWorkDomain(4, 59, 0, 1e12));
        }
        catch(Exception& e)
        {
            LOG_WARN_STR(e);
            return false;
        }
    }


    bool BBSKernelProcessControl::handle(const BBSStep* bs)
    {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
        ASSERT(itsPrediffer);

        const MSDesc &descriptor = itsPrediffer->getMSDescriptor();

        LOG_DEBUG_STR("MS descriptor:" << endl << descriptor);

        double startFreq = descriptor.startFreq[0];
        double endFreq = descriptor.endFreq[0];

        /*
            (Some?) Westerbork measurement sets store the channels
            in reverse order. In this case, start and end frequency
            need to be exchanged.
        */
        if(startFreq > endFreq)
        {
            double tmp;
            tmp = startFreq;
            startFreq = endFreq;
            endFreq = tmp;
        }

        double startTime = descriptor.startTime;
        double endTime = descriptor.endTime;
        double time = startTime;

        bool result = true;
        while(result && (time < endTime))
        {
            ASSERT(itsPrediffer->setWorkDomain(startFreq, endFreq, time, time + itsWorkDomainSize.timeInterval));

            result = false;

            {
                const BBSPredictStep* step = dynamic_cast<const BBSPredictStep*>(bs);
                if(step)
                {
                    result = doHandle(step);
                }
            }
                    
            {
                const BBSSubtractStep* step = dynamic_cast<const BBSSubtractStep*>(bs);
                if(step)
                {
                    result = doHandle(step);
                }
            }
            
            {
                const BBSCorrectStep* step = dynamic_cast<const BBSCorrectStep*>(bs);
                if(step)
                {
                    result = doHandle(step);
                }
            }
            
            {
                const BBSSolveStep* step = dynamic_cast<const BBSSolveStep*>(bs);
                if(step)
                {
                    result = doHandle(step);
                }
            }

            time += itsWorkDomainSize.timeInterval;
        }
        
        return result;
    }


    //##----   P r i v a t e   m e t h o d s   ----##//

    void BBSKernelProcessControl::convertStepToContext(const BBSStep *step, Context &context)
    {
        ASSERT(step->baselines().station1.size() == step->baselines().station2.size());

        context.baselines = step->baselines();
        /*
        vector<string>::const_iterator it1 = step->itsBaselines.station1.begin();
        vector<string>::const_iterator it2 = step->itsBaselines.station2.begin();
        while(it1 != step->itsBaselines.station1.end())
        {
        context.baselines.push_back(pair<string, string>(*it1, *it2));
        ++it1;
        ++it2;        
        }
        */
        context.correlation = step->correlation();
        context.sources = step->sources();        
        context.instrumentModel = step->instrumentModels();
    }


    bool BBSKernelProcessControl::sendObject(const BlobStreamable& bs)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      try {
	// Serialize the object
	itsDataHolder->serialize(bs);
	LOG_DEBUG_STR("Sending a " << itsDataHolder->classType() << " object");

	// Do a blocking send
	if (itsConnection->write() == CSConnection::Error) {
	  LOG_ERROR("Connection::write() failed");
	  return false;
	}
      }
      catch (Exception& e) {
	LOG_ERROR_STR(e);
	return false;
      }
      // All went well.
      return true;
    }


    BlobStreamable* BBSKernelProcessControl::recvObject()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      try {
	// Do a blocking receive
	if (itsConnection->read() == CSConnection::Error) {
	  THROW(IOException, "CSConnection::read() failed");
	}

	// Deserialize the object
	BlobStreamable* bs = itsDataHolder->deserialize();
	if (!bs) {
	  LOG_ERROR("Error while receiving object");
	}
	LOG_DEBUG_STR("Received a " << itsDataHolder->classType() <<
		      " object");

	// Return the object
	return bs;
      }
      catch (Exception& e) {
	LOG_ERROR_STR(e);
	return 0;
      }
    }


    bool BBSKernelProcessControl::doHandle(const BBSPredictStep *step)
    {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
        ASSERTSTR(itsPrediffer, "No Prediffer available.");
        ASSERTSTR(step, "Step corrupted.");

        PredictContext context;
        convertStepToContext(step, context);
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


    bool BBSKernelProcessControl::doHandle(const BBSSubtractStep *step)
    {
        ASSERTSTR(itsPrediffer, "No Prediffer available.");
        ASSERTSTR(step, "Step corrupted.");

        SubtractContext context;
        convertStepToContext(step, context);
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


    bool BBSKernelProcessControl::doHandle(const BBSCorrectStep *step)
    {
        ASSERTSTR(itsPrediffer, "No Prediffer available.");
        ASSERTSTR(step, "Step corrupted.");

        CorrectContext context;
        convertStepToContext(step, context);
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


    bool BBSKernelProcessControl::doHandle(const BBSSolveStep *step)
    {
        ASSERTSTR(itsPrediffer, "No Prediffer available.");
        ASSERTSTR(step, "Step corrupted.");

        // Construct context.
        GenerateContext context;
        convertStepToContext(step, context);
        context.unknowns = step->parms();
        context.excludedUnknowns = step->exclParms();

        // Create solve domains. For now this just splits the work domain in a rectangular grid,
        // with cells of size step->itsDomainSize. Should become more interesting in the near future.
        const MeqDomain &workDomain = itsPrediffer->getWorkDomain();

        const int freqCount = (int) ceil((workDomain.endX() - workDomain.startX()) / step->domainSize().bandWidth);
        const int timeCount = (int) ceil((workDomain.endY() - workDomain.startY()) / step->domainSize().timeInterval);

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

                context.solveDomains.push_back(MeqDomain(freqOffset, freqOffset + freqSize, timeOffset, timeOffset + timeSize));

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

        //LOG_INFO_STR("Solve domains:" << endl << itsPrediffer->getSolveDomains());

        // Initialize the solver.
        Solver solver;
        solver.initSolvableParmData(1, itsPrediffer->getSolveDomains(), itsPrediffer->getWorkDomain());
        solver.setSolvableParmData(itsPrediffer->getSolvableParmData(), 0);

        // Optionally log to history.
        if(itsHistory)
        {
            solver.log(*itsHistory, step->getName());
        }

        // Output some settings to stdout
        itsPrediffer->showSettings();

        // Main iteration loop.
        unsigned int iteration = 0;
        bool converged = false;
        while(iteration < step->maxIter() && !converged)
        {
            // Generate normal equations and pass them to the solver.
            vector<casa::LSQFit> equations;
            itsPrediffer->generateEquations(equations);
            solver.mergeFitters(equations, 0);

            // Do one Levenberg-Maquardt step.
//            solver.solve(false);
            solver.solve(true);

            // Optionally log to history.
            if(itsHistory)
            {
                solver.log(*itsHistory, step->getName());
            }

            LOG_INFO_STR("Iteration " << iteration << ":  " << setprecision(10));
            
            // Log convergence info and check for convergence.
            int convergedSolveDomains = 0;
            for(unsigned int i = 0; i < context.solveDomains.size(); ++i)
            {
                Quality quality = solver.getQuality(i);
                
                //LOG_DEBUG_STR("domain " << i << ": rank=" << quality.itsRank << " fit=" << quality.itsFit << " chi_sqr=" << quality.itsChi);
                
                if(quality.itsFit < 0 && abs(quality.itsFit) <= step->epsilon())
//                if(quality.itsChi < step->epsilon())
                {
                    convergedSolveDomains++;
                }
            }
            converged = (convergedSolveDomains >= (step->minConverged() * context.solveDomains.size()) / 100.0);

            LOG_INFO_STR("Solve domains converged: " << convergedSolveDomains << "/" << context.solveDomains.size() << " (" << (((double) convergedSolveDomains) / context.solveDomains.size() * 100.0) << "%)");

            // Send updates back to the Prediffer.
            itsPrediffer->updateSolvableParms(solver.getSolvableParmData());
            iteration++;
        }

        LOG_INFO_STR("Writing solutions into parameter databases...");
        itsPrediffer->writeParms();
        return true;
    }

} // namespace BBS

} // namespace LOFAR
