//#  LofarLogger.cc: Interface to the log4cplus logging package
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

//# Includes
#include <Common/LofarLogger.h>

namespace LOFAR {

//#------------------------- Global user functions -------------------------------
//
// formatString(format, ...) --> string
//
// Define a global function the accepts printf like arguments and returns a string.
//
const std::string formatString(const	char* format, ...) {
	char		tmp_cstring[1024];
	va_list		ap;

	va_start (ap, format);
	vsnprintf(tmp_cstring, sizeof(tmp_cstring), format, ap);
	va_end   (ap);

	return   std::string(tmp_cstring);
}


//#------------------------- Internal implementation -----------------------------
using namespace log4cplus;

//# ------------------------ implement the five trace levels ------------------------
const LogLevel	TRACE1_LOG_LEVEL	= 1;
const LogLevel	TRACE2_LOG_LEVEL	= 2;
const LogLevel	TRACE3_LOG_LEVEL	= 3;
const LogLevel	TRACE4_LOG_LEVEL	= 4;
const LogLevel	TRACE5_LOG_LEVEL	= 5;

#define _TRACE1_STRING	LOG4CPLUS_TEXT("TRACE1")
#define _TRACE2_STRING	LOG4CPLUS_TEXT("TRACE2")
#define _TRACE3_STRING	LOG4CPLUS_TEXT("TRACE3")
#define _TRACE4_STRING	LOG4CPLUS_TEXT("TRACE4")
#define _TRACE5_STRING	LOG4CPLUS_TEXT("TRACE5")

tstring traceLevel2String(LogLevel ll) {
	switch (ll) {
	case TRACE1_LOG_LEVEL:	return _TRACE1_STRING;
	case TRACE2_LOG_LEVEL:	return _TRACE2_STRING;
	case TRACE3_LOG_LEVEL:	return _TRACE3_STRING;
	case TRACE4_LOG_LEVEL:	return _TRACE4_STRING;
	case TRACE5_LOG_LEVEL:	return _TRACE5_STRING;
	}

	return tstring();		// not found
}

LogLevel string2TraceLevel (const tstring& lname) {
	if (lname == _TRACE1_STRING)	return TRACE1_LOG_LEVEL;
	if (lname == _TRACE2_STRING)	return TRACE2_LOG_LEVEL;
	if (lname == _TRACE3_STRING)	return TRACE3_LOG_LEVEL;
	if (lname == _TRACE4_STRING)	return TRACE4_LOG_LEVEL;
	if (lname == _TRACE5_STRING)	return TRACE5_LOG_LEVEL;

	return NOT_SET_LOG_LEVEL;			// not found
}

// initTracemodule
//
// Function that is used when the TRACE levels are NOT compiled out. It registers
// the TRACEn management routines at the Log4Cplus LogLevelManager and sets up the
// global trace-logger named "TRACE", with the additivity set to false.
// Attached to the trace-logger is one Appender that logs to stderr.
//
void initTraceModule (void) {
	//# register our own loglevels
	getLogLevelManager().pushToStringMethod(traceLevel2String);
	getLogLevelManager().pushFromStringMethod(string2TraceLevel);
	
	//# Setup a property object to initialise the TRACE Logger
	helpers::Properties		traceProp;		
	traceProp.setProperty("log4cplus.logger.TRACE", "TRACE, TRACE");
	traceProp.setProperty("log4cplus.additivity.TRACE", "false");
	traceProp.setProperty("log4cplus.appender.TRACE", "log4cplus::ConsoleAppender");
//	traceProp.setProperty("log4cplus.appender.TRACE.layout","log4cplus::PatternLayout");
//	traceProp.setProperty("log4cplus.appender.TRACE.layout.ConversionPattern","%c %m%n");
	traceProp.setProperty("log4cplus.appender.TRACE.layout", "log4cplus::TTCCLayout");
	traceProp.setProperty("log4cplus.appender.TRACE.layout.Use_gmtime", "true");

	PropertyConfigurator(traceProp).configure();

}
LOFAR::LoggerReference	TraceLoggerRef("TRACE");			// create the tracelogger

}	// namespace LOFAR
