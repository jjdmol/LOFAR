//#  CEPlogProcessorMain.cc: Deamon to dispatch the BG/P logging to PVSS
//#
//#  Copyright (C) 2009
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
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>
#include <Common/Exception.h>
#include <GCF/TM/GCF_Scheduler.h>
#include "CEPlogProcessor.h"

using namespace LOFAR;
using namespace GCF::TM;
using namespace LOFAR::APL;

// Use a terminate handler that can produce a backtrace.
Exception::TerminateHandler t(Exception::terminate);

//
// MAIN (parameterfile)
//
int main (int argc, char* argv[]) 
{
	try {
		GCFScheduler::instance()->init(argc, argv, "CEPlogProcessor");

		CEPlogProcessor		loggerTask("CEPlogger");
		loggerTask.start(); // make initial transition

		GCFScheduler::instance()->run();

		LOG_INFO_STR("Shutting down: " << argv[0]);
	}
	catch (LOFAR::Exception&	ex) {
		LOG_FATAL_STR("Caught exception: " << ex);
		LOG_FATAL     ("Terminated by exception!");
		return (1);
	}

	LOG_INFO("Terminated normally");
	return (0);
}
