//#  BBSKernelProcessControl.cc: 
//#
//#  Copyright (C) 2004
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

#include <APS/ParameterSet.h>
#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>
#include <ParmDB/ParmDB.h>
#include <ParmDB/ParmDBMeta.h>
#include <BBSControl/BBSStrategy.h>
#include <BBSControl/BBSSingleStep.h>
#include <BBSControl/BBSSolveStep.h>
#include <BBSKernel/BBSKernelStructs.h>
#include <BBSKernel/Prediffer.h>
#include <BBSKernel/Solver.h>

#include <iostream>
#include <iomanip>
using namespace std;

namespace LOFAR 
{
namespace BBS 
{
    using LOFAR::operator<<;
        
    BBSKernelProcessControl::BBSKernelProcessControl()
    :   ProcessControl(),
        itsPrediffer(0),
        itsHistory(0)
    {
    }
    
    // Destructor
    BBSKernelProcessControl::~BBSKernelProcessControl()
    {
        delete itsPrediffer;
        delete itsHistory;
    }

    // \name Command to control the processes.
    // There are a dozen commands that can be sent to a application process
    // to control its flow. The return values for these command are:<br>
    // - True   - Command executed succesfully.
    // - False  - Command could not be executed.
    // 
    // @{

    // During the \c define state the process check the contents of the
    // ParameterSet it received during start-up. When everthing seems ok the
    // process constructs the communication channels for exchanging data
    // with the other processes. The connection are NOT made in the stage.
    boost::logic::tribool BBSKernelProcessControl::define()
    {
        LOG_TRACE_FLOW("define()");
        //LOFAR::ACC::APS::ParameterSet* parset = LOFAR::ACC::APS::globalParameterSet();
        
        // Get BBSController IP-address?
        // Initialize some data holders?
        
        return true;
    }

    // When a process receives an \c init command it allocates the buffers it
    // needs and makes the connections with the other processes. When the
    // process succeeds in this it is ready for dataprocessing (or whatever
    // task the process has).
    boost::logic::tribool BBSKernelProcessControl::init()
    {
        LOG_TRACE_FLOW("init()");
        
        // Create connection with BBSController?
        
        return true;
    }

    // During the \c run phase the process does the work it is designed for.
    // The run phase stays active until another command is send.
    boost::logic::tribool BBSKernelProcessControl::run()
    {
        LOG_TRACE_FLOW("run()");
        
        // Non-blocking receive from BBSControl
        
        // If received msg
        bool status = true;
/*        
        if(...)
        {
            if(msg->type() == BBSStrategy::type())
            {
                status = handle(dynamic_cast<BBSStrategy*>(msg));
            }
            if(msg->type() == BBSPredictStep::type())
            {
                status = handle(dynamic_cast<BBSPredictStep*>(msg));
            }
            else if(msg->type() == BBSSubtractStep::type())
            {
                status = handle(dynamic_cast<BBSSubtractStep*>(msg));
            }
            else if(msg->type() == BBSCorrectStep::type())
            {
                status = handle(dynamic_cast<BBSCorrectStep*>(msg));
            }
            else if(msg->type() == BBSSolveStep::type())
            {
                status = handle(dynamic_cast<BBSSolveStep*>(msg));
            }
            else
            {
                LOG_ERROR("Received message of unknown type, skipped.")
            }
        }
*/            
        return status;
    }

    // With the \c pause command the process stops its run phase and starts
    // waiting for another command. The \c condition argument contains the
    // contition the process should use for ending the run phase. This
    // condition is a key-value pair that can eg. contain a timestamp or a
    // number of a datasample.
    boost::logic::tribool BBSKernelProcessControl::pause(const string& condition)
    {
        return false;
    }

    // \c Quit stops the process.
    // The process \b must call \c unregisterAtAC at ProcControlServer during 
    // the execution of this command to pass the final results to the 
    // Application Controller.
    boost::logic::tribool BBSKernelProcessControl::quit()
    {
        LOG_TRACE_FLOW("quit()");
        return true;
    }

    // With the \c snapshot command the process is instructed to save itself
    // in a database is such a way that on another moment in time it can
    // be reconstructed and can continue it task.<br>
    // The \c destination argument contains database info the process
    // must use to save itself.
    boost::logic::tribool BBSKernelProcessControl::snapshot(const string& destination)
    {
        return false;
    }

    // \c Recover reconstructs the process as it was saved some time earlier.
    // The \c source argument contains the database info the process must use
    // to find the information it needs.
    boost::logic::tribool BBSKernelProcessControl::recover(const string& source)
    {
        return false;
    }

    // With \c reinit the process receives a new parameterset that it must use
    // to reinitialize itself.
    boost::logic::tribool BBSKernelProcessControl::reinit(const string& configID)
    {
        return false;
    }
    // @}

    // Define a generic way to exchange info between client and server.
    std::string BBSKernelProcessControl::askInfo(const string& keylist)
    {
        return std::string("");
    }

    // This is somewhat hairy: convert from struct Correlation to struct CorrelationMask, which are
    // basically the same. However, one is defined in the BBSControl package and the other in the
    // BBSKernel package. It may be better to move the Correlation struct, and also the Baselines struct,
    // to the BBSKernel package and then have BBSControl depend on that.
    //
    // Note: Correlation::NONE is meaningless; it is an error if this should occur.
    //
    /*
    CorrelationMask BBSKernelProcessControl::convertCorrelationToMask(const Correlation &correlation)
    {
        CorrelationMask mask;
        
        switch(correlation.selection)
        {
            case Correlation::AUTO:
                mask.selection = CorrelationMask::AUTO;
                break;
            case Correlation::CROSS:
                mask.selection = CorrelationMask::CROSS;
                break;
            default:
                mask.selection = CorrelationMask::ALL;
                break;
        }
        mask.type = correlation.type;        
        
        return mask;
    }
    */
    
    // This is somewhat hairy: convert a BBSStep to a context. If the issue mentioned above
    // at convertCorrelationToMask() is resolved, this method will become simpler. The baselines
    // and correlation could then be copied directly.
    void BBSKernelProcessControl::convertStepToContext(const BBSStep *step, Context &context)
    {
        ASSERT(step->itsBaselines.station1.size() == step->itsBaselines.station2.size());
        
        context.baselines = step->itsBaselines;
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
        context.correlation = step->itsCorrelation;
        context.sources = step->itsSources;        
        context.instrumentModel = step->itsInstrumentModels;
    }
    
    
    bool BBSKernelProcessControl::handle(const BBSStrategy *strategy)
    {
        delete itsHistory;
        itsHistory = 0;
        
        if(!strategy->itsParmDB.history.empty())
        {
            LOFAR::ParmDB::ParmDBMeta historyDBMeta("aips", strategy->itsParmDB.history);
            itsHistory = new LOFAR::ParmDB::ParmDB(historyDBMeta);
        }
        
        // Create a new Prediffer.
        delete itsPrediffer;
        itsPrediffer = new Prediffer(   strategy->itsDataSet,
                                        strategy->itsInputData,
                                        strategy->itsParmDB.localSky,
                                        strategy->itsParmDB.instrument,
                                        0,
                                        false);
        
        // Set data selection and work domain.
        bool status;
        status = itsPrediffer->setSelection(strategy->itsStations, strategy->itsCorrelation)
                 && itsPrediffer->setWorkDomain(-1, -1, 0, 1e12);
        return status;
    }
    
    
    bool BBSKernelProcessControl::handle(const BBSPredictStep *step)
    {
        ASSERTSTR(itsPrediffer, "No Prediffer available.");
        ASSERTSTR(step, "Step corrupted.");
        
        PredictContext context;
        convertStepToContext(step, context);
        context.outputColumn = step->itsOutputData;
        
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
        convertStepToContext(step, context);
        context.outputColumn = step->itsOutputData;
        
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
        convertStepToContext(step, context);
        context.outputColumn = step->itsOutputData;
        
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
        ASSERTSTR(itsPrediffer, "No Prediffer available.");
        ASSERTSTR(step, "Step corrupted.");
        
        // Construct context.
        GenerateContext context;
        convertStepToContext(step, context);
        context.unknowns = step->itsParms;
        context.excludedUnknowns = step->itsExclParms;
        
        // Create solve domains. For now this just splits the work domain in a rectangular grid,
        // with cells of size step->itsDomainSize. Should become more interesting in the near future.
        const MeqDomain &workDomain = itsPrediffer->getWorkDomain();
        
        const int freqCount = (int) ceil((workDomain.endX() - workDomain.startX()) / step->itsDomainSize.bandWidth);
        const int timeCount = (int) ceil((workDomain.endY() - workDomain.startY()) / step->itsDomainSize.timeInterval);

        double timeOffset = workDomain.startY();
        double timeSize = step->itsDomainSize.timeInterval;
        
        int time = 0;
        while(time < timeCount)
        {
            double freqOffset = workDomain.startX();
            double freqSize = step->itsDomainSize.bandWidth;

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
        
        cout << "Solve domains:" << endl;
        cout << itsPrediffer->getSolveDomains() << endl;
        
        // Initialize the solver.
        Solver solver;
        solver.initSolvableParmData(1, itsPrediffer->getSolveDomains(), itsPrediffer->getWorkDomain());
        solver.setSolvableParmData(itsPrediffer->getSolvableParmData(), 0);
        itsPrediffer->showSettings();

        // Main iteration loop.
        unsigned int iteration = 0;
        bool converged = false;
        while(iteration < step->itsMaxIter && !converged)
        {
            // Generate normal equations and pass them to the solver.
            vector<casa::LSQFit> equations;
            itsPrediffer->generateEquations(equations);
            solver.mergeFitters(equations, 0);

            // Do one Levenberg-Maquardt step.
            solver.solve(false);
            
            // Optionally log to history.
            if(itsHistory)
            {
                solver.log(*itsHistory, step->itsName);
            }

            // Check for convergence.
            int convergedSolveDomains = 0;
            for(unsigned int i = 0; i < context.solveDomains.size(); ++i)
            {
                Quality quality = solver.getQuality(i);
                if(quality.itsChi < step->itsEpsilon)
                {
                    convergedSolveDomains++;
                }
            }
            converged = (((double) convergedSolveDomains) / context.solveDomains.size()) > step->itsMinConverged;
            
            cout << "iteration " << iteration << ":  " << setprecision(10) << solver.getSolvableValues(0) << endl;
            cout << "solve domains converged: " << convergedSolveDomains << "/" << context.solveDomains.size() <<
                " (" << (((double) convergedSolveDomains) / context.solveDomains.size() * 100.0) << "%)" << endl;

            // Send updates back to the Prediffer.
            itsPrediffer->updateSolvableParms(solver.getSolvableParmData());
            
            iteration++;
        }
        
        //cout << "Writing solutions into ParmDB ..." << endl;
        //itsPrediffer->writeParms();
        return true;
    }
} // namespace BBS
} // namespace LOFAR
