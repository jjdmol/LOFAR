//#  PythonControlMain.cc: Main entry for the PythonControl controller.
//#
//#  Copyright (C) 2006
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
//#  $Id$
//#
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/Exception.h>
#include <MessageBus/MessageBus.h>

#include "PythonControl.h"

using namespace LOFAR::GCF::TM;
using namespace LOFAR::CEPCU;
using namespace LOFAR;

// Use a terminate handler that can produce a backtrace.
LOFAR::Exception::TerminateHandler t(Exception::terminate);

int main(int argc, char* argv[])
{
	// args: cntlrname
	if(argc < 2) {
		printf("Unexpected number of arguments: %d\n",argc);
		printf("%s usage: %s <controller name>\n",argv[0],argv[0]);
		return(-1);
	}

	try {
		GCFScheduler::instance()->init(argc, argv, argv[1]);

    MessageBus::init();

		ParentControl*	pc = ParentControl::instance();
		pc->start();	// make initial transition

		PythonControl	pyc(argv[1]);
		pyc.start(); 	// make initial transition

		GCFScheduler::instance()->setDelayedQuit(true);
		GCFScheduler::instance()->run();	// until stop was called.

		pc->quit();		// let tasks quit nicely.
		pyc.quit();
		GCFScheduler::instance()->run(0.3); // let tasks die.
	} catch( LOFAR::Exception &ex ) {
		LOG_FATAL_STR("Caught exception: " << ex);
		return 1;
	}

	return (0);
}

