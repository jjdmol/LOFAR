//#  testRunOnNode.cc: Program to test the interface to the LofarLogger.
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
//#  $Id$


#include <Common/RunOnNode.h>

using namespace std;
using namespace LOFAR;

int main (int, char *argv[]) {

	INIT_LOGGER_AND_WATCH("testLogger", 10000);
	LOG_INFO (formatString("Program %s has started", argv[0]));

	LOG_TRACE_FLOW("Initialise RunOnNode; tell this process to be (0,0)");
	SETNODE(0,0);

	LOG_TRACE_FLOW("Check for conditional execution");
	RUNINPROCESS(0,0)  LOG_TRACE_FLOW("Executing code for (0,0)");
	RUNINPROCESS(3,7)  LOG_ERROR("Executing code for (3,7)");

// todo:test reset of node (not implemented yet)
// 	LOG_TRACE_FLOW("Now re-init to (3,7)");
// 	SETNODE(3,7);
// 	LOG_TRACE_FLOW("Check for conditional execution");
// 	RUNINPROCESS(0,0)  LOG_TRACE_ERROR("Executing code for (0,0)");
// 	RUNINPROCESS(3,7)  LOG_TRACE_FLOW("Executing code for (3,7)");

	
}

