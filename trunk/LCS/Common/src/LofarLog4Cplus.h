//#  LofarLog4Cplus.h: Interface to the log4cplus logging package
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

#ifndef LOFAR_COMMON_LOG4CPLUS_H
#define LOFAR_COMMON_LOG4CPLUS_H

// \file LofarLog4Cplus.h
// Interface to the log4cplus logging package

#include <lofar_config.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_string.h>
#include <Common/Exception.h>

//# Includes
#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>


//# -------------------- Initialisation of the logger module ---------------
//#
// Before you can use any function of the logger module you should initialize
// the module with an appropriate properties-file.
// There are two functions available for this:
//  - INIT_LOGGER				Only initializes the logger module.
//  - INIT_LOGGER_AND_WATCH		After initialisation a thread is started to
//								monitor any changes in the properties file.
//								An intervaltime in millisecs must be provided.
#define INIT_LOGGER(filename) \
	LOFAR::lofarLoggerInitNode(); \
	LofarInitTracingModule \
	if (!strstr(filename, ".log_prop")) { \
		log4cplus::PropertyConfigurator::doConfigure(log4cplus::tstring(filename)+".log_prop"); \
	} \
	else  {\
		log4cplus::PropertyConfigurator::doConfigure(filename); \
	}

#ifdef USE_THREADS
# define INIT_LOGGER_AND_WATCH(filename,watchinterval) \
	LOFAR::lofarLoggerInitNode(); \
	LofarInitTracingModule \
	if (!strstr(filename, ".log_prop")) { \
		log4cplus::ConfigureAndWatchThread tmpWatchThread(log4cplus::tstring(filename)+".log_prop",watchinterval); \
	} \
	else  {\
		log4cplus::ConfigureAndWatchThread tmpWatchThread(filename,watchinterval); \
	}
#else
# define INIT_LOGGER_AND_WATCH(filename,watchinterval) INIT_LOGGER(filename)
#endif

//# -------------------- Log Levels for the Operator messages ------------------
//#
//# LOG_FATAL(_STR) (message | stream)
//# LOG_ERROR(_STR) (message | stream)
//# LOG_WARN(_STR)  (message | stream)
//# LOG_INFO(_STR)  (message | stream)
//#
// The LofarLogger utility can log messages on six different levels:
// FATAL - ERROR - WARN - INFO - DEBUG - TRACE
// There are strict rules how to use these levels so please read the programmer
// manual of the LofarLogger before using it.
//
// For every level there are two calls made, LOG_<level> and LOG_<level>_STR.
// The first form expects a string as an argument, the second form a stream.
// The stream version allows you to use the operator<< but it is MUCH slower
// than its string counterpart.
// To simplifly the usage of strings you can call the global function 
// 'formatString' that accepts printf like arguments and returns a string.
//
// The functions LOG_FATAL till LOG_INFO produce messages that are meant for the
// operators. The message may NOT contain any 'inside' information, they should
// tell:
//  - WHAT situation has arisen
//  - WHY this situation is a fault situation
//  - HOW the situation should be solved (when appropriate).

// LOG_FATAL(_STR) should be used when an unrecoverable exception occures.
#define LOG_FATAL(message) 			LofarLog(FATAL_LOG_LEVEL,message)
#define LOG_FATAL_STR(stream)		LofarLogStr(FATAL_LOG_LEVEL,stream)

// LOG_ERROR(_STR) should be used in case of recoverable exceptions and illegal
// start parms.
#define LOG_ERROR(message) 			LofarLog(ERROR_LOG_LEVEL,message)
#define LOG_ERROR_STR(stream)		LofarLogStr(ERROR_LOG_LEVEL,stream)

// Use LOG_WARN(_STR) when an unexpected situation occured that could be solved
// be the software itself.
#define LOG_WARN(message) 			LofarLog(WARN_LOG_LEVEL,message)
#define LOG_WARN_STR(stream)		LofarLogStr(WARN_LOG_LEVEL,stream)

// LOG_INFO(_STR) should be used to notify operator startup and normal 
// termination of programs. It can also be used for other 'global' actions.
#define LOG_INFO(message) 			LofarLog(INFO_LOG_LEVEL,message)
#define LOG_INFO_STR(stream)		LofarLogStr(INFO_LOG_LEVEL,stream)

//# ------------------------- Debug Levels for the Integrator -------------------------
//#
//# LOG_DEBUG(_STR)	(message | stream)
//#
// Debug information is primairy meant for the integrator/user of your moudule. 
// Note that the user of your module is often a developer. The messages contain
// information about the I/O boundaries of a software module. They should be
// clear to someone who does not have detailed inside information.
//
// The debug messages can still be present in the final 'production' version of
// your module, so don't overload your program with it.

#ifdef DISABLE_DEBUG_OUTPUT
#define LOG_DEBUG(message)
#define LOG_DEBUG_STR(stream)
#else

// Use this macro for plain and 'printf' like messages.
#define LOG_DEBUG(message) 			LofarLog(DEBUG_LOG_LEVEL,message)
 
// Use this macro for operator<< messages
// Note: the 'printf' counterparts are MUCH faster and produce less code!
#define LOG_DEBUG_STR(stream)		LofarLogStr(DEBUG_LOG_LEVEL,stream)

#endif // DISABLE_DEBUG_OUTPUT

//# ----------------------- Trace Levels for the Programmer --------------------
//#
//# ALLOC_TRACER_CONTEXT
//# INIT_TRACER_CONTEXT  	 (scope, contextname)
//# ALLOC_TRACER_ALIAS		 (parent)
//# LOG_TRACE_<type>(_STR) 	 (stream)
//#
//# Where <type> = LOOP, VAR, CALC, COND, STAT, OBJ, RTTI or FLOW
//#
// The trace level is split up into five additive sublevels to be able to
// control the amount of output generated on this trace level. Again there are
// strict guidelines how to use the levels in order to improve maintenance of
// the source (by other developers).
//
// Tracing is implemented different from the other levels to get maximum speed
// for the decision whether of not to log a message. The fact remains though
// that the string-form of the calls stay much faster than their stream-form
// counterparts.
//
// Unlike the other loglevels the trace level will not be present anymore in the
// final production code.
#ifdef ENABLE_TRACER

// Allocates a global LoggerReference object in the scope of the class you
// place this macro in. The (also defined) function getLogger returns the
// created LoggerReference object.
#define	ALLOC_TRACER_CONTEXT \
	private:	static		LOFAR::LoggerReference	theirTraceLoggerRef;	\
	public:		static inline	LOFAR::LoggerReference	&getLogger() \
				{ return theirTraceLoggerRef; }	

// Attach a logger to the LoggerReference your defined in 'scope/class'.
#define INIT_TRACER_CONTEXT(scope,contextname) \
	::LOFAR::LoggerReference	scope::theirTraceLoggerRef(LOFAR::formatString("TRC.%s",contextname))

// Only defines a function the return the LoggerReference in scope 'scope/class'
#define ALLOC_TRACER_ALIAS(other) \
	public:		static inline	LOFAR::LoggerReference	&getLogger() \
				{ return other::theirTraceLoggerRef; }	
// 

#define LOG_TRACE_LOOP(message)		LofarLogTrace(1, message)
#define LOG_TRACE_VAR(message)		LofarLogTrace(2, message)
#define LOG_TRACE_CALC(message)		LofarLogTrace(3, message)
#define LOG_TRACE_COND(message)		LofarLogTrace(4, message)
#define LOG_TRACE_STAT(message)		LofarLogTrace(5, message)
#define LOG_TRACE_OBJ(message)		LofarLogTrace(6, message)
#define LOG_TRACE_RTTI(message)		LofarLogTrace(7, message)
#define LOG_TRACE_FLOW(message)		LofarLogTrace(8, message)
#define LOG_TRACE_LOOP_STR(stream)	LofarLogTraceStr(1, stream)
#define LOG_TRACE_VAR_STR(stream)	LofarLogTraceStr(2, stream)
#define LOG_TRACE_CALC_STR(stream)	LofarLogTraceStr(3, stream)
#define LOG_TRACE_COND_STR(stream)	LofarLogTraceStr(4, stream)
#define LOG_TRACE_STAT_STR(stream)	LofarLogTraceStr(5, stream)
#define LOG_TRACE_OBJ_STR(stream)	LofarLogTraceStr(6, stream)
#define LOG_TRACE_RTTI_STR(stream)	LofarLogTraceStr(7, stream)
#define LOG_TRACE_FLOW_STR(stream)	LofarLogTraceStr(8, stream)
#define TRACE_LEVEL_LOOP			1
#define TRACE_LEVEL_VAR				2
#define TRACE_LEVEL_CALC			3
#define TRACE_LEVEL_COND			4
#define TRACE_LEVEL_STAT			5
#define TRACE_LEVEL_OBJ				6
#define TRACE_LEVEL_RTTI			7
#define TRACE_LEVEL_FLOW			8

//#
//# LOG_TRACE_LIFETIME(_STR) (level,message | stream)
//#

// The macros LOG_TRACE_LIFETIME(_STR) create a TraceLogger object that will
// output your message during construct and destruction. Your message is
// preceeded with "ENTER:" or "EXIT:".
#define LOG_TRACE_LIFETIME(level,message) \
	LifetimeLogger _tmpLifetimeTraceObj(level, getLogger().logger(), \
	LOFAR::formatString("%s:%s", AUTO_FUNCTION_NAME, message), \
			__FILE__, __LINE__); 

#define LOG_TRACE_LIFETIME_STR(level,stream) \
	ostringstream	oss; \
	oss << AUTO_FUNCTION_NAME << ":" << stream; \
	LifetimeLogger		_tmpLifetimeTraceObj(level, getLogger().logger(), \
								oss.str(), __FILE__, __LINE__); \
	}
	
//# ----------- implementation details tracer part -------------
namespace LOFAR {
// \addtogroup Common
// @{
void	initTraceModule(void);
// @}
}
// Internal macro to define (or not) the initialisation routine of the
// trace module.
#define	LofarInitTracingModule	LOFAR::initTraceModule();

// Internal macro used by the LOG_TRACE_<level> macros.
#define LofarLogTrace(level,message) \
	if (getLogger().logger().isEnabledFor(level)) \
		getLogger().logger().forcedLog(level, message, __FILE__, __LINE__)

// Internal macro used by the LOG_TRACE_<level>_STR macros.
#define LofarLogTraceStr(level,stream) \
	if (getLogger().logger().isEnabledFor(level)) { \
		std::ostringstream	oss;	\
		oss << stream;					\
		getLogger().logger().forcedLog(level, oss.str(), __FILE__, __LINE__); }

#else	// ENABLE_TRACER
//# Define dummies if tracing is disabled.
#define LOG_TRACE_LOOP(message)
#define LOG_TRACE_VAR(message)
#define LOG_TRACE_CALC(message)
#define LOG_TRACE_COND(message)
#define LOG_TRACE_STAT(message)
#define LOG_TRACE_OBJ(message)
#define LOG_TRACE_RTTI(message)
#define LOG_TRACE_FLOW(message)
#define LOG_TRACE_LOOP_STR(stream)
#define LOG_TRACE_VAR_STR(stream)
#define LOG_TRACE_CALC_STR(stream)
#define LOG_TRACE_COND_STR(stream)
#define LOG_TRACE_STAT_STR(stream)
#define LOG_TRACE_OBJ_STR(stream)
#define LOG_TRACE_RTTI_STR(stream)
#define LOG_TRACE_FLOW_STR(stream)
#define TRACE_LEVEL_LOOP	0
#define TRACE_LEVEL_VAR		0
#define TRACE_LEVEL_CALC	0
#define TRACE_LEVEL_COND	0
#define TRACE_LEVEL_STAT	0
#define TRACE_LEVEL_OBJ		0
#define TRACE_LEVEL_RTTI	0
#define TRACE_LEVEL_FLOW	0

#define	LofarInitTracingModule
#define	ALLOC_TRACER_CONTEXT 
#define INIT_TRACER_CONTEXT(scope,contextname) 
#define LOG_TRACE_LIFETIME(level,message)
#define LOG_TRACE_LIFETIME_STR(level,stream)

#endif	// ENABLE_TRACER

//#------------------------ Assert en FailWhen -------------------------------
//#
//# THROW		  (exception,stream)
//# (DBG)ASSERT	  (condition,stream)
//# (DBG)FAILWHEN (condition,stream)
//#
//# Note: only THROW needs to be defines here, the others are build on THROW
//# in the LofarLogger.h file.

// The macro THROW first sends a logrequest to logger <module>.EXCEPTION
// before executing the real throw.
#undef THROW
#define THROW(exc,stream) { \
	std::ostringstream	oss;	\
	oss << stream;				\
	log4cplus::Logger::getInstance(LOFARLOGGER_FULLPACKAGE ".EXCEPTION").log( \
					log4cplus::DEBUG_LOG_LEVEL, oss.str(), __FILE__, __LINE__); \
	throw (exc(oss.str(), __HERE__)); \
	}

//# ----------- implementation details generic part -------------

// Define internal macro's for standard logging functions
#define LofarLog(level,message) \
	log4cplus::Logger::getInstance(LOFARLOGGER_FULLPACKAGE).log(log4cplus::level, message, __FILE__, __LINE__);
#define LofarLogStr(level,stream) {		\
	std::ostringstream	oss;			\
	oss << stream;						\
	LofarLog(level,oss.str())			\
	}

namespace LOFAR {
// \addtogroup Common
// @{
void	lofarLoggerInitNode(void);
// @}
}

//------------------------- LoggerReference class ---------------------------------
namespace LOFAR {

// \addtogroup Common
// @{

// The LoggerReference class is used for implementing faster logging
// for the trace levels. The class holds a Logger which is in fact a
// pointer to a shared Object.
class	LoggerReference {
public:
	// A logger with the given name is created and a pointer to this shared
	// object is stored in the instance of this class.
	LoggerReference(const std::string&	loggerName) : 
			itsLogger(log4cplus::Logger::getInstance(loggerName)) {	 }
	~LoggerReference() { }

	// Returns the pointer to the logger.
	log4cplus::Logger&	logger()	{ return itsLogger; }

private:
	LoggerReference();			// Force use of loggername

	//# Data
	log4cplus::Logger			itsLogger;
};

// The LifetimeLogger class produces TRACE messages when an instance of this
// class is created and when it is destroyed.
class LifetimeLogger {
public:
	LifetimeLogger(const log4cplus::LogLevel ll, const log4cplus::Logger& logger,
				   const log4cplus::tstring& msg, const char* file=NULL, int line=-1)
		: itsLevel(ll), itsLogger(logger), itsMsg(msg), itsFile(file), itsLine(line) 
	{
		if (itsLogger.isEnabledFor(itsLevel)) {
			itsLogger.forcedLog(itsLevel, LOG4CPLUS_TEXT("ENTER: ") + itsMsg, 
																itsFile, itsLine);
		}
	}

	~LifetimeLogger()
	{
		if (itsLogger.isEnabledFor(itsLevel)) {
			itsLogger.forcedLog(itsLevel, LOG4CPLUS_TEXT("EXIT: ") + itsMsg, 
																itsFile, itsLine);
		}
	}

private:
	log4cplus::LogLevel		itsLevel;
	log4cplus::Logger		itsLogger;
	log4cplus::tstring		itsMsg;
	const char*		      	itsFile;
	int	      			itsLine;
};

// The 'mother' of all trace-loggers is stored here.
extern LoggerReference	theirTraceLoggerRef;
// Function to return the 'mother' of all trace-loggers.
inline LoggerReference&	getLogger() { return theirTraceLoggerRef; }

// @}
} // namespace LOFAR

#endif // file read before
