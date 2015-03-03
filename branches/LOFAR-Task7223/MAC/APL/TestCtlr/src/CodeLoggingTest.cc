//#  CodeLoggingTest.cc: Testprogram for testing CodeLoggingProcessor
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
#include <Common/SystemUtil.h>
#include <Common/Exception.h>

#include <time.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>

using namespace LOFAR;

// Use a terminate handler that can produce a backtrace.
Exception::TerminateHandler t(Exception::terminate);

int main(int argc, char* argv[])
{
	if (argc != 2) {
		printf("Syntax: %s PVSS_data_point\n", LOFAR::basename(argv[0]).c_str());
		printf("Every 10 seconds a line is logged to this datapoint until program is killed");
		return (1);
	}

	INIT_LOGGER(LOFAR::basename(argv[0]));

	LOG_INFO_STR("MACProcessScope: " << argv[1]);

	while (true) {
		time_t		now;
		char		timestr[30];

		now = time(0);
		strcpy (timestr, ctime(&now));
		timestr[strlen(timestr)-1] = '\0';	// strip off cr
		LOG_INFO_STR("Current logtime = " << timestr);

		sleep (10);
	}

	return 0;
}


