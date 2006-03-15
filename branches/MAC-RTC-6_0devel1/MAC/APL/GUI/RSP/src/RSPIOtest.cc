//#  RSPIOmainMain.cc: one line description
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
#include <Common/LofarLogger.h>
#include <RSP/RSPport.h>

using namespace LOFAR;
using namespace LOFAR::RSP;

//
// MAIN (param1, param2)
//
int main (int argc, char* argv[]) {

	// Always bring up the logger first
	string progName = basename(argv[0]);
	INIT_LOGGER (progName.c_str());

	// Check invocation syntax
	if (argc != 2) {
		LOG_FATAL_STR ("Invocation error, syntax: " << progName <<
				   " hostname");
		return (-1);
	}

	// Tell operator we are trying to start up.
	LOG_INFO_STR("Starting up: " << argv[0] << "(" << argv[1] << ")");
	try {
		RSPport		IOport (argv[1]);

		uint32		rcuMask = 0;
		vector<BoardStatus>	boardArr = IOport.getBoardStatus(rcuMask);

		sleep (2);

//		BoardStatus		bp = sysStat.board()(0);
		BoardStatus*	bp = &boardArr[0];
		cout << "voltage 1.2V: " << bp->rsp.voltage_1_2 / 192.0 * 2.5 << endl;
		cout << "voltage 2.5V: " << bp->rsp.voltage_2_5 / 192.0 * 3.3 << endl;
		cout << "voltage 3.3V: " << bp->rsp.voltage_3_3 / 192.0 * 5.0 << endl;

		LOG_INFO_STR("Shutting down: " << argv[0]);
	}
	catch (LOFAR::Exception& ex) {
		LOG_FATAL_STR("Caught exception: " << ex << endl);
		LOG_FATAL_STR(argv[0] << " terminated by exception!");
		return (1);
	}

	LOG_INFO_STR(argv[0] << " terminated normally");

	return (0);

}
