//#  tLoggingProcessor.cc: one_line_description
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

//# Includes
#include <Common/LofarLogger.h>
#include <Common/lofar_string.h>

using namespace LOFAR;

int main (int	argc, char*		argv[])
{
	INIT_LOGGER(basename(argv[0]));

	switch (atoi(argv[1])) {
	case 1:	// do everything right
		// not registered yet.
		LOG_INFO("This message is not shown in PVSS");

		// register at right level
		LOG_INFO("MACProcessScope: LOFAR.Logger.Test");
		LOG_INFO("TEST1:Finally this message should appear in PVSS");
		LOG_WARN("TEST1:And this warning message also");
	break;

	case 2:	// register first at wrong level
		// not registered yet.
		LOG_INFO("This message is not shown in PVSS");

		// registering at wrong loglevel
		LOG_DEBUG("MACProcessScope: LOFAR.Logger.Test");
		LOG_INFO("This message is still not shown in PVSS");
		
		// register at right level
		LOG_INFO("MACProcessScope: LOFAR.Logger.Test");
		LOG_INFO("TEST2:Finally this message should appear in PVSS");
		LOG_WARN("TEST2:And this warning message also");
	break;

	case 3:  { // Don't register at all and produce more that 10 messages
		for (int i = 1; i < 15; i++) {
			LOG_INFO_STR("TEST3: Message " << i);
		}
	}
	break;

	case 4: {	// Register with wrong DP and produce more that 10 messages
		for (int i = 1; i < 15; i++) {
			LOG_INFO("MACProcessScope: LOFAR.Logger.WrongTest");
			LOG_INFO_STR("TEST4: Message " << i);
		}
	}
	break;

	default:
		cout << "Syntax: tLoggingProcessor testnr" << endl;
		break;
	}

	return (0);
}

