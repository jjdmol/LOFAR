//#  StationCorrelatorController.cc: interpretes commands from ACC and executes them on the StationCorrelator object
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

#include <Common/LofarLogger.h>

#include <tinyCEP/Profiler.h>
#include <ACC/ProcControlServer.h>
#include <ACC/ParameterSet.h>
#include <ACC/ProcessControl.h>

#include <StationCorrelator.h>
#include <StationCorrelatorController.h>

using namespace LOFAR;
using namespace LOFAR::ACC;

StationCorrelatorController::StationCorrelatorController(StationCorrelator& stationCor, int noRuns)
  : itsStationCorrelator(stationCor),
    itsNoRuns(noRuns)
{};
StationCorrelatorController::~StationCorrelatorController()
{};

bool StationCorrelatorController::define   () const
{
  cout<<"Define called"<<endl;
  LOG_TRACE_FLOW("Define called by ACC");
  itsStationCorrelator.baseDefine();
  cout<<"Define ready"<<endl;
  return true;
}
bool StationCorrelatorController::init     () const
{
  cout<<"Init called"<<endl;
  LOG_TRACE_FLOW("Init called by ACC");
  Profiler::init();
  itsStationCorrelator.basePrerun();
  cout<<"Init ready"<<endl;
  return true;
}
bool StationCorrelatorController::run      () const
{
  cout<<"Running"<<endl;
  LOG_TRACE_FLOW("Run called by ACC");
  Profiler::activate();
  itsStationCorrelator.baseRun(itsNoRuns);
  return true;
}
bool StationCorrelatorController::pause    (const	string&	condition) const
{
  cout<<"Pause called"<<endl;
  LOG_ERROR("Pause not implemented in ApplicationHolder");
  cout<<"Pause ready"<<endl;
  return false;
}
bool StationCorrelatorController::quit  	 () const
{
  cout<<"Quit called"<<endl;
  LOG_TRACE_FLOW("Quit called by ACC");
  Profiler::deActivate();
  itsStationCorrelator.basePostrun();
  itsStationCorrelator.baseQuit();
  cout<<"Quit ready"<<endl;
  return true;
}
bool StationCorrelatorController::snapshot (const string&	destination) const
{
  cout<<"Snapshot called"<<endl;
  LOG_ERROR("Snapshot not implemented in ApplicationHolder");
  cout<<"Snapshot ready"<<endl;
  return false;
}
bool StationCorrelatorController::recover  (const string&	source) const
{
  cout<<"Recover called"<<endl;
  LOG_ERROR("recover not implemented in ApplicationHolder");
  cout<<"Recover ready"<<endl;
  return false;
}
bool StationCorrelatorController::reinit	 (const string&	configID) const
{
  cout<<"Reinit called"<<endl;
  LOG_ERROR("Reinit not implemented in ApplicationHolder");
  cout<<"Reinit ready"<<endl;
  return false;
}  
string StationCorrelatorController::askInfo   (const string& 	keylist) const
{
  cout<<"AskInfo called"<<endl;
  cout<<"AskInfo ready"<<endl;
  return "no info available yet";
}


