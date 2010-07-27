//# LofarLogger.cc: Interface to the log4cplus logging package
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <cstdio>					// snprintf
#include <unistd.h>					// readlink
#include <libgen.h>				// basename
#include <cstring>
#include <Common/LofarLogger.h>

#if defined(HAVE_LOG4CXX)
# include <log4cxx/ndc.h>
#endif


namespace LOFAR {

//#------------------------- Internal implementation -----------------------------
//
// lofarLoggerInitNode()
//
// Creates a NDC with the text "application@node" and pushes it
// on the NDC stack
void lofarLoggerInitNode(void) {
	const int	MAXLEN = 128;
	int		applNameLen = 0;
	char	hostName [MAXLEN];
	char	applName [MAXLEN];
	char	loggerId [MAXLEN];

	// try to resolve the hostname
	if (gethostname (hostName, MAXLEN-1) < 0) {
		hostName[0]='\0';
	}

	// try to resolve the applicationname
	applNameLen = readlink("/proc/self/exe", applName, MAXLEN-1);
	if (applNameLen >= 0)
		applName[applNameLen] = '\0';
	else
		strcpy (applName, "");
//	}	

	// construct loggerId and register it.
	snprintf(loggerId, MAXLEN-1, "%s@%s", basename(applName), hostName);
#if defined(HAVE_LOG4CPLUS)
	log4cplus::getNDC().push(loggerId);
#elif defined(HAVE_LOG4CXX)
        log4cxx::NDC::push(loggerId);
#endif
}

}	// namespace LOFAR
