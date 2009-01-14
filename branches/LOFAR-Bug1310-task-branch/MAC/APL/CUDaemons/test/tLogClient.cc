//#  tLogClient.cc: one line description
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
#include <unistd.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;


//
// MAIN (param1)
//
int main (int argc, char* argv[]) {

	// Always bring up the logger first
	string progName = basename(argv[0]);
	INIT_LOGGER (progName.c_str());

	// Check invocation syntax
	if (argc < 3) {
		LOG_FATAL_STR ("Invocation error, syntax: " << progName <<
						" DPname loginterval");
		return (-1);
	}

	LOG_INFO_STR ("MACProcessScope: " << argv[1]);

	// Tell operator we are trying to start up.
	LOG_INFO_STR("Starting up: " << argv[0] << "(" << argv[1] << ", "
								<< argv[2] << ")");

	int		level(0);
	int		count(1);
	while (true) {
		switch (level) {
			case 0:	LOG_DEBUG_STR ("Log message " << count++ << " on level DEBUG");   break;
			case 1:	LOG_INFO_STR  ("Log message " << count++ << " on level INFO");    break;
			case 2:	LOG_WARN_STR  ("Log message " << count++ << " on level WARNING"); break;
			case 3:	LOG_ERROR_STR ("Log message " << count++ << " on level ERROR");   break;
			case 4:	LOG_FATAL_STR ("Log message " << count++ << " on level FATAL");   break;
		}

		level++;
		if (level > 4) {
			level = 0;
		}
		sleep (atoi(argv[2]));
	}

	LOG_INFO_STR(argv[0] << " terminated normally");

	return (0);
}
