//#  ApplicationHolderController.cc: interpretes commands from ACC and executes them on the ApplicationHolder object
//#
//#  Copyright (C) 2002-2004
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
#include <Transport/TH_MPI.h>

#include <Common/LofarLogger.h>

#include <tinyCEP/Profiler.h>
#include <PLC/ProcControlServer.h>
#include <PLC/ProcessControl.h>
#include <APS/ParameterSet.h>

#include <tinyCEP/ApplicationHolderController.h>

// for strncmp
#include <string.h>

using namespace LOFAR;
using namespace LOFAR::ACC::APS;
using namespace LOFAR::ACC::PLC;

using namespace boost::logic;

ApplicationHolderController::ApplicationHolderController(TinyApplicationHolder& AH, int noRuns)
  : itsAH(AH),
    itsNoRuns(noRuns),
    itsNoTotalRuns(0),
    itsIsRunning(false),
    itsShouldQuit(false)
{};
ApplicationHolderController::~ApplicationHolderController()
{};

tribool ApplicationHolderController::define   () 
{
  LOG_TRACE_FLOW("Define called by ACC");
  try {
    itsAH.setParameters(*globalParameterSet());
    itsAH.baseDefine();
  } catch (Exception&	ex) {
    LOG_WARN_STR("Exception during define: " << ex.what());
    std::cerr << "Exception during define: " << ex.what() << std::endl;
    return false;
  }
  return true;
}
tribool ApplicationHolderController::init     () 
{
  LOG_TRACE_FLOW("Init called by ACC");
  try {
    Profiler::init();
    itsAH.basePrerun();
  } catch (Exception&	ex) {
    LOG_WARN_STR("Exception during init: " << ex.what());
    std::cerr << "Exception during init: " << ex.what() << std::endl;
    return false;
  }
  return true;
}
tribool ApplicationHolderController::run      () 
{
  LOG_TRACE_FLOW("Run called by ACC");
  try {
    Profiler::activate();
    itsAH.baseRun(itsNoRuns);
    itsNoTotalRuns += itsNoRuns;
    itsIsRunning = true;
  } catch (Exception&	ex) {
    LOG_WARN_STR("Exception during run: " << ex.what());
    std::cerr << "Exception during run: " << ex.what() << std::endl;
    return false;
  }
  return true;
}
tribool ApplicationHolderController::pause    (const	string&	)
{
  LOG_TRACE_FLOW("Pause called");
  itsIsRunning = false;
  return true;
}
tribool ApplicationHolderController::release    ()
{
  return true;
}
tribool ApplicationHolderController::quit  	 () 
{
  LOG_TRACE_FLOW("Quit called by ACC");
  try {
    itsIsRunning = false;
    itsShouldQuit = true;
    Profiler::deActivate();
    itsAH.basePostrun();
    itsAH.baseQuit();
  } catch (Exception&	ex) {
    LOG_WARN_STR("Exception during quit: " << ex.what());
    std::cerr << "Exception during quit: " << ex.what() << std::endl;
    return false;
  }
  LOG_TRACE_FLOW("Quit ready");
  return true;
}
tribool ApplicationHolderController::snapshot (const string&) 
{
  LOG_TRACE_FLOW("Snapshot called by ACC");
  try {
    LOG_ERROR("Snapshot not implemented in ApplicationHolder");
  } catch (Exception&	ex) {
    return false;
  }
  return indeterminate; // this is not implemented
}
tribool ApplicationHolderController::recover  (const string& ) 
{
  LOG_TRACE_FLOW("Recover called by ACC");
  try {
    LOG_ERROR("recover not implemented in ApplicationHolder");
  } catch (Exception&	ex) {
    return false;
  }
  return indeterminate; // this is not implemented
}
tribool ApplicationHolderController::reinit	 (const string&	) 
{
  LOG_TRACE_FLOW("Reinit called by ACC");
  try {
    itsAH.basePrerun();
  } catch (Exception&	ex) {
    LOG_WARN_STR("Exception during reinit: " << ex.what());
    std::cerr << "Exception during reinit: " << ex.what() << std::endl;
    return false;
  }
  return true;
}  
string ApplicationHolderController::askInfo   (const string& ) 
{
  LOG_TRACE_FLOW("AskInfo called by ACC");
  return "no info available yet";
}
