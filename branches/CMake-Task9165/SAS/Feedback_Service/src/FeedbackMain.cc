//#  FeedbackMain.cc: Main entry for the Feedback controller.
//#
//#  Copyright (C) 2015
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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
//#  $Id: FeedbackMain.cc 23491 2013-01-11 10:07:59Z overeem $
//#
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/Exception.h>
#include <MessageBus/MessageBus.h>
#include <GCF/TM/GCF_Control.h>

#include "Feedback.h"

using namespace LOFAR::GCF::TM;
using namespace LOFAR;
using namespace LOFAR::SAS;

// Use a terminate handler that can produce a backtrace.
Exception::TerminateHandler t(Exception::terminate);

int main(int argc, char* argv[])
{
	try {
		GCFScheduler::instance()->init(argc, argv, "FeedbackService");
    
    MessageBus::init();

		Feedback	fbTask;
		fbTask.start(); 					// make initial transition

		GCFScheduler::instance()->run();	// until stop was called.
	} catch( Exception &ex ) {
		LOG_FATAL_STR("Caught exception: " << ex);
		return 1;
	}

	return (0);
}

