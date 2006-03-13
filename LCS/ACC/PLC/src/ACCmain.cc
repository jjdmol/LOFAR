//#  ACCmain.cc: main loop that can be used by any ACC enabled program
//#
//#  Copyright (C) 2006
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
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <libgen.h>
#include <PLC/ACCmain.h>
#include <Common/LofarLogger.h>
#include <APS/ParameterSet.h>
#include <PLC/ProcControlServer.h>

namespace LOFAR {
  namespace ACC {
    namespace PLC {

      using APS::ParameterSet;

      int ACCmain (int argc, char* argv[], ProcessControl* theProcess) {

	string programName = basename(argv[0]);
	INIT_LOGGER(programName.c_str());

	try {

	  // Check invocation syntax
	  if ((argc!=3) || (strncmp("ACC", argv[1], 3) != 0)) {

	    // we were not called by ACC
	    LOG_TRACE_FLOW(programName + " not started by ACC");

	    // See if there is a parameterset available
	    vector<string> possibleNames;
	    
	    // try to find a parameterset
	    // start with program name + .parset
	    // also try parts of the program name (splitted with "_")
	    string pName = programName;
	    string::size_type pos;
	    do {
	      possibleNames.push_back(pName + ".parset");
	      //possibleNames.push_back(pName + ".cfg");
	      //possibleNames.push_back(pName + ".ps");
	      
	      pos = pName.rfind('_');
	      pName = pName.substr(0, pos);
	    } while (pos != string::npos);

	    ParameterSet* ps = 0;
	    vector<string>::iterator i;
	    for (i = possibleNames.begin(); i != possibleNames.end(); i++) {
	      try {
		LOG_TRACE_FLOW_STR("Trying to use "<<*i<<" as parameterSet");
		ps = new ParameterSet(*i);
		LOG_INFO_STR("Using "<<*i<<" as parameter set.");
		break;
	      } catch (...) {
	        LOG_TRACE_FLOW_STR(*i << " not found");
		ps = 0;
	      }
	    }
	    
	    if (ps == 0) {
	      LOG_INFO_STR("Could not find a parameter set.");
	      LOG_TRACE_FLOW(programName + " starting define");
	      theProcess->define();
	    } else {
	      LOG_TRACE_FLOW(programName + " starting define");
	      theProcess->define(*ps);
	      delete ps;
	    }
	    LOG_TRACE_FLOW(programName + " initting");
	    theProcess->init();
	    LOG_TRACE_FLOW(programName + " running");
	    theProcess->run();
	    LOG_TRACE_FLOW(programName + " quitting");
	    theProcess->quit();
	    LOG_TRACE_FLOW(programName + " deleting process");

	  } else {

	    LOG_TRACE_FLOW("Main program started by ACC");

	    // Now all ACC processes expect "ACC" as first argument
	    // So the parameter file is the second argument

	    // Read in parameterfile and get my name
	    ParameterSet itsOrigParamSet(argv[2]);
	    string procID = itsOrigParamSet.getString("process.name");
	    ParameterSet ParamSet = itsOrigParamSet.makeSubset(procID);

	    ProcControlServer pcServer(ParamSet.getString("ACnode"),
				       ParamSet.getUint16("ACport"),
				       theProcess);
       

	    LOG_TRACE_FLOW("Registering at ApplController");
	    pcServer.registerAtAC(procID);
	    LOG_TRACE_FLOW("Registered at ApplController");

	    // Main processing loop
	    while (1) {
	      LOG_TRACE_FLOW("Polling ApplController for message");
	      if (pcServer.pollForMessage()) {
		LOG_TRACE_FLOW("Message received from ApplController");
	  
		// get pointer to received data
		DH_ProcControl* newMsg = pcServer.getDataHolder();
	  
		pcServer.handleMessage(newMsg);

		if (newMsg->getCommand() == ACC::PLC::PCCmdQuit) {
		  break;
		}
	      } 
	    }
    
	    LOG_INFO_STR("Shutting down: ApplicationController");
	    pcServer.unregisterAtAC("");		// send to AC before quiting
	  }
	} catch (Exception& ex) {
	  LOG_FATAL_STR("Caught exception: " << ex << endl);
	  LOG_FATAL_STR(argv[0] << " terminated by exception!");
	  return 1;
	} catch (...) {
	  LOG_FATAL_STR("Caught unknown exception, exitting");
	  return 1;
	}  
	LOG_INFO_STR(programName << " terminated normally");
	return 0;
      }

    } // namespace PLC
  } // namespace ACC
} // namespace LOFAR
