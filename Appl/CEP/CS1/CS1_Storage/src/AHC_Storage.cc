//#  AHC_Storage.cc: interpretes commands from ACC and executes them on the ApplicationHolder object
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
#include <CS1_Storage/AHC_Storage.h>

#include <tinyCEP/Profiler.h>
#include <PLC/ProcControlServer.h>
#include <PLC/ProcessControl.h>
#include <APS/ParameterSet.h>
#include <CS1_Interface/CS1_Parset.h>

#include <tinyCEP/ApplicationHolderController.h>

// for strncmp
#include <string.h>

using namespace LOFAR;
using namespace LOFAR::ACC::APS;
using namespace LOFAR::ACC::PLC;

using namespace boost::logic;

namespace LOFAR { 
  namespace CS1 {
  
AHC_Storage::AHC_Storage(TinyApplicationHolder& AH, int noRuns)
  : ApplicationHolderController (AH, noRuns)
{};

AHC_Storage::~AHC_Storage()
{};

tribool AHC_Storage::pause    (const	string&	)
{
  LOG_TRACE_FLOW("Pause called");
  CS1_Parset itsCS1PS = CS1_Parset(ACC::APS::globalParameterSet());
  uint nRuns = 1;
  
  double startTime = itsCS1PS.startTime();
  double stopTime = itsCS1PS.stopTime();
  double stepTime  = itsCS1PS.BGLintegrationTime();
  uint totalRuns = uint(1 + ceil((stopTime - startTime) / stepTime));
  
  totalRuns = ((totalRuns+15)&~15);
  totalRuns = totalRuns / itsCS1PS.IONintegrationSteps();
  
  int nrRunsLeft = totalRuns - nRuns;
  
  time_t now = time(0);
  
  char buf[26];
  ctime_r(&now, buf);
  buf[24] = '\0';
 
  cout << "time = " << buf
#ifdef HAVE_MPI					  
       << ", rank = "  << TH_MPI::getCurrentRank() 
#endif					  
       <<", nrRunsLeft = totalRuns - nRuns = " << totalRuns << " - " << nRuns << " = " << nrRunsLeft << endl;
  
  if (totalRuns > nRuns){
    for (int i=0; i < nrRunsLeft; i++) {
      if (!ApplicationHolderController::run()) {
        return (1);
      }
    }
  }
  
  
  return true;
}

tribool AHC_Storage::quit  	 () 
{
  LOG_TRACE_FLOW("Quit called");
  ApplicationHolderController::quit();
#if 0
  pcServer.sendResult(newMsg->getCommand(), ACC::PLC::PcCmdMaskOk);
						newMsg->setCommand(ACC::PLC::PCCmd(newMsg->getCommand() &~ ACC::PLC::PCCmdResult));
						quiting = true;
#endif						
  LOG_TRACE_FLOW("Quit ready");
  return true;
}
  }
}  
