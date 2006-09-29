//# BBSKernel.cc: minimal control wrapper around the kernel
//#
//# Copyright (C) 2004
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
//# $Id:

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <APS/ParameterSet.h>
#include <PLC/ProcControlServer.h>
#include <Common/LofarLogger.h>
#include <BBSControl/BBSStrategy.h>
#include <BBSControl/BBSStep.h>
#include <BBSControl/BBSSingleStep.h>
#include <BBSControl/BBSSolveStep.h>
#include <BBSControl/BBSKernelProcessControl.h>
#include <BBSKernel/Prediffer.h>
#include <BBSKernel/MNS/MeqDomain.h>

#include <string>

using namespace LOFAR;
using namespace LOFAR::BBS;
using namespace std;
using LOFAR::ACC::APS::ParameterSet;
using LOFAR::ACC::PLC::ProcControlServer;
using LOFAR::ACC::PLC::DH_ProcControl;

int main(int argc, char **argv)
{
    BBSKernelProcessControl process;
    
    string programName = basename(argv[0]);
    INIT_LOGGER(programName.c_str());
        
    if((argc != 3) || (strncmp("ACC", argv[1], 3) != 0))
    {
        // Not started by ACC.
        LOG_TRACE_FLOW(programName + " not started by ACC.");

        if(argc != 2)
        {
            LOG_INFO("No parset file specified, usage: " + programName + " [ACC] <parset file>");
            return 1;
        }
        
        try
        {
            ParameterSet parset(argv[1]);
            BBSStrategy strategy(parset);
            
            process.handle(&strategy);
            
            vector<const BBSStep*> steps = strategy.getAllSteps();
            for(int i = 0; i < steps.size(); ++i)
            {
                cout << "step type: " << steps[i]->type() << endl;
                
                if(steps[i]->type() == "BBSPredictStep")
                {
                    process.handle(dynamic_cast<const BBSPredictStep*>(steps[i]));
                }
                if(steps[i]->type() == "BBSSubtractStep")
                {
                    process.handle(dynamic_cast<const BBSSubtractStep*>(steps[i]));
                }
                if(steps[i]->type() == "BBSCorrectStep")
                {
                    process.handle(dynamic_cast<const BBSCorrectStep*>(steps[i]));
                }
                if(steps[i]->type() == "BBSSolveStep")
                {
                    process.handle(dynamic_cast<const BBSSolveStep*>(steps[i]));
                }
            }
        }
        catch(...)
        {
            LOG_TRACE_FLOW_STR("Parset file " << argv[1] << " not found.");
            return 1;
        }

        /*
        LOFAR::ACC::APS::globalParameterSet()->adoptCollection(parset);

        LOG_TRACE_FLOW("Calling define()...");
        if(!process->define())
        {
            return 1;
        }
        
        LOG_TRACE_FLOW("Calling init()...");
        if(!process->init())
        {
            return 1;
        }

        LOG_TRACE_FLOW("Calling run()...");
        if(!process->run())
        {
            return 1;
        }

        LOG_TRACE_FLOW("Calling quit()...");
        if (!process->quit())
        {
            return 1;
        }
        */
        
        LOG_TRACE_FLOW("Deleting process " + programName + ".");
    }
    else
    {
        // Started by ACC.
        LOG_TRACE_FLOW(programName + " started by ACC.");

        // All ACC processes expect "ACC" as first argument,
        // so the parameter file is the second argument.

        // Read in parameterfile and get my name
        ParameterSet parset(argv[2]);
        string processID = parset.getString("process.name");
        ParameterSet parsubset = parset.makeSubset(processID + ".");

        LOFAR::ACC::APS::globalParameterSet()->adoptCollection(parsubset);

        ProcControlServer controlServer(parsubset.getString("ACnode"), parsubset.getUint16("ACport"), &process);
        LOG_TRACE_FLOW("Registering at ApplicationController");
        controlServer.registerAtAC(processID);
        LOG_TRACE_FLOW("Registered at ApplicationController");

        // Main processing loop
        bool running = false;
        bool quit = false;
        while(!quit)
        {
            LOG_TRACE_FLOW("Polling ApplicationController for a new message.");
            if(controlServer.pollForMessage())
            {
                LOG_TRACE_FLOW("New message received from ApplicationController.");
                
                // Get pointer to message
                DH_ProcControl* msg = controlServer.getDataHolder();
                controlServer.handleMessage(msg);

                switch(msg->getCommand())
                {
                    case LOFAR::ACC::PLC::PCCmdQuit:
                        quit = true;
                        break;
                    case LOFAR::ACC::PLC::PCCmdRun:
                        running = true;
                        break;
                    case LOFAR::ACC::PLC::PCCmdPause:
                        running = false;
                        break;
                }
            }
            else
            {
                if(running)
                {
                    process.run();
                }
            }
        }            
        
        LOG_INFO_STR("Unregistering at ApplicationController");
        controlServer.unregisterAtAC("");
    }
    
    LOG_INFO_STR(programName << " terminated normally.");
    return 0;
}
