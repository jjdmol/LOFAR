//#  LofarLogger.h: Interface to the log4cplus logging package
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

#if !defined(LOFAR_LOGGER_H)
#define LOFAR_LOGGER_H

#include <lofar_config.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_string.h>
#include <Common/Exception.h>

//# Includes
#include <stdarg.h>				// vsnprintf in formatString
#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>

namespace LOFAR {
extern const std::string formatString(const	char* format, ...);
}

//# -------------------- Initialisation of the logger module -------------------
//#
// Before you can use any function of the logger module you should initialize
// the module with an appropriate properties-file.
// There are two functions available for this:
//  - INIT_LOGGER				Only initializes the logger module.
//  - INIT_LOGGER_AND_WATCH		After initialisation a thread is started to
//								monitor any changes in the properties file.
//								An intervaltime in millisecs must be provided.
#define INIT_LOGGER(filename) \
	log4cplus::PropertyConfigurator::doConfigure(filename); \
	LofarInitTracingModule

#define INIT_LOGGER_AND_WATCH(filename,watchinterval) \
	log4cplus::ConfigureAndWatchThread	tmpWatchThread(filename,watchinterval); \
	LofarInitTracingModule

//# -------------------- Log Levels for the Operator messages ------------------
//#
//# LOG_FATAL(_STR) (logger, message | stream)
//# LOG_ERROR(_STR)	(logger, message | stream)
//# LOG_WARN(_STR)	(logger, message | stream)
//# LOG_INFO(_STR)	(logger, message | stream)
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
// formatString that accepts printf like arguments and returns a string.
//
// The functions LOG_FATAL till LOG_INFO produce messages that are meant for the
// operators. The message may NOT contain any 'inside' information, they should
// tell:
//  - WHAT situation has arisen
//  - WHY this situation is a fault situation
//  - HOW the situation should be solved.

// LOG_FATAL(_STR) should be used when an unrecoverable exception occures.
#define LOG_FATAL(logger,message) \
	log4cplus::Logger::getInstance(logger).log(log4cplus::FATAL_LOG_LEVEL, message);
#define LOG_FATAL_STR(logger,message) { 			\
	std::ostringstream	oss;	\
	oss << message;					\
	log4cplus::Logger::getInstance(logger).log(log4cplus::FATAL_LOG_LEVEL, oss.str());	}


// LOG_ERROR(_STR) should be used in case of recoverable exceptions and illegal
// start parms.
#define LOG_ERROR(logger,message) \
	log4cplus::Logger::getInstance(logger).log(log4cplus::ERROR_LOG_LEVEL, message);
#define LOG_ERROR_STR(logger,message) { 			\
	std::ostringstream	oss;	\
	oss << message;					\
	log4cplus::Logger::getInstance(logger).log(log4cplus::ERROR_LOG_LEVEL, oss.str());	}


// Use LOG_WARN(_STR) when ...
#define LOG_WARN(logger,message) \
	log4cplus::Logger::getInstance(logger).log(log4cplus::WARN_LOG_LEVEL, message);
#define LOG_WARN_STR(logger,message) { 			\
	std::ostringstream	oss;	\
	oss << message;					\
	log4cplus::Logger::getInstance(logger).log(log4cplus::WARN_LOG_LEVEL, oss.str()); }


// LOG_INFO(_STR) should be used to notify operator startup and normal 
// termination of programs. It can also be used for other 'global' actions.
#define LOG_INFO(logger,message) \
	log4cplus::Logger::getInstance(logger).log(log4cplus::INFO_LOG_LEVEL, message);
#define LOG_INFO_STR(logger,message) { 			\
	std::ostringstream	oss;	\
	oss << message;					\
	log4cplus::Logger::getInstance(logger).log(log4cplus::INFO_LOG_LEVEL, oss.str()); }


//# ------------------------- Debug Levels for the Integrator -------------------------
//#
//# LOG_DEBUG(_STR)	(logger, message | stream)
//#
// Debug information is primairy meant for the integrator/user of your moudule. 
// Note that the user of your module is often a developer. The messages contain
// information about the I/O boundaries of a software module. They should be
// clear to someone who does not have detailed inside information.
//
// The debug messages can still be present in the final 'production' version of
// your module, so don't overload your program with it.

#ifdef DISABLE_DEBUG_OUTPUT
#define LOG_DEBUG(logger,message)
#define LOG_DEBUG_STR(logger,stream)
#else

// Use this macro for plain and 'printf' like messages.
#define LOG_DEBUG(logger,message) \
	log4cplus::Logger::getInstance(logger).log(log4cplus::DEBUG_LOG_LEVEL, \
											   message, __FILE__, __LINE__);

// Use these macro's for operator<< messages
// Note: the 'printf' counterparts are MUCH faster and produce less code!
#define LOG_DEBUG_STR(logger,message) { 			\
	std::ostringstream	oss;	\
	oss << message;					\
	log4cplus::Logger::getInstance(logger).log(log4cplus::DEBUG_LOG_LEVEL, \
		oss.str(), __FILE__, __LINE__);	}

#endif

//# ----------------------- Trace Levels for the Programmer --------------------
//#
//# LOCAL_TRACER_CONTEXT
//# INIT_TRACER_CONTEXT  (scope, contextname)
//# LOG_TRACE_LOOP(_STR) (stream)
//# LOG_TRACE_VAR(_STR)  (stream)
//# LOG_TRACE_COND(_STR) (stream)
//# LOG_TRACE_OBJ(_STR)  (stream)
//# LOG_TRACE_FLOW(_STR) (stream)
//#
// The trace level is split up into five additive sublevels to be able to
// control the amount of output generated on this trace level. Again there are
// strict guidelines how to use the levels in order to improve maintenance of
// the source (by other developers).
//
// Tracing is implemented different that the other levels to get maximum speed
// for the decision whether of not to log a message. The fact remains though
// that the string-form of the calls stay much faster that there stream-from
// counterparts.
//
// Unlike the other loglevels the trace level will not be present anymore in the
// final production code.
#ifdef ENABLE_TRACER

#define	LOCAL_TRACER_CONTEXT \
	private:	static 			::LOFAR::LoggerReference	TraceLoggerRef;	\
	public:		static inline	::LOFAR::LoggerReference	&getLogger() \
				{ return TraceLoggerRef; }	

#define INIT_TRACER_CONTEXT(scope,contextname) \
	::LOFAR::LoggerReference	scope::TraceLoggerRef(LOFAR::formatString("TRACE.%s",contextname))

// 
#define LOG_TRACE_LOOP(message)		LofarLogTrace(1, message)
#define LOG_TRACE_VAR(message)		LofarLogTrace(2, message)
#define LOG_TRACE_COND(message)		LofarLogTrace(3, message)
#define LOG_TRACE_OBJ(message)		LofarLogTrace(4, message)
#define LOG_TRACE_FLOW(message)		LofarLogTrace(5, message)
#define LOG_TRACE_LOOP_STR(stream)	LofarLogTraceStr(1, stream)
#define LOG_TRACE_VAR_STR(stream)	LofarLogTraceStr(2, stream)
#define LOG_TRACE_COND_STR(stream)	LofarLogTraceStr(3, stream)
#define LOG_TRACE_OBJ_STR(stream)	LofarLogTraceStr(4, stream)
#define LOG_TRACE_FLOW_STR(stream)	LofarLogTraceStr(5, stream)
#define TRACE_LEVEL_LOOP			1
#define TRACE_LEVEL_VAR				2
#define TRACE_LEVEL_COND			3
#define TRACE_LEVEL_OBJ				4
#define TRACE_LEVEL_FLOW			5

// If you like numbers more than names(?)
#define LOG_TRACE1(message)			LofarLogTrace(1, message)
#define LOG_TRACE2(message)			LofarLogTrace(2, message)
#define LOG_TRACE3(message)			LofarLogTrace(3, message)
#define LOG_TRACE4(message)			LofarLogTrace(4, message)
#define LOG_TRACE5(message)			LofarLogTrace(5, message)
#define LOG_TRACE_STR1(stream)		LofarLogTraceStr(1, stream)
#define LOG_TRACE_STR2(stream)		LofarLogTraceStr(2, stream)
#define LOG_TRACE_STR3(stream)		LofarLogTraceStr(3, stream)
#define LOG_TRACE_STR4(stream)		LofarLogTraceStr(4, stream)
#define LOG_TRACE_STR5(stream)		LofarLogTraceStr(5, stream)
#define TRACE_LEVEL1				1
#define TRACE_LEVEL2				2
#define TRACE_LEVEL3				3
#define TRACE_LEVEL4				4
#define TRACE_LEVEL5				5

//# ----------- implementation details -------------
namespace LOFAR {
extern void	initTraceModule(void);
}
#define	LofarInitTracingModule	initTraceModule();

#define LofarLogTrace(level,message) \
	if (getLogger().logger().isEnabledFor(level)) \
		getLogger().logger().forcedLog(level, message, __FILE__, __LINE__)

#define LofarLogTraceStr(level,stream) \
	if (getLogger().logger().isEnabledFor(level)) { \
		std::ostringstream	oss;	\
		oss << stream;					\
		getLogger().logger().forcedLog(level, oss.str(), __FILE__, __LINE__); }

#else	// ENABLE_TRACER
//# Define dummies if tracing is disabled.
#define LOG_TRACE_LOOP(message)
#define LOG_TRACE_VAR(message)
#define LOG_TRACE_COND(message)
#define LOG_TRACE_OBJ(message)
#define LOG_TRACE_FLOW(message)
#define LOG_TRACE_LOOP_STR(stream)
#define LOG_TRACE_VAR_STR(stream)
#define LOG_TRACE_COND_STR(stream)
#define LOG_TRACE_OBJ_STR(stream)
#define LOG_TRACE_FLOW_STR(stream)
#define LOG_TRACE1(message)
#define LOG_TRACE2(message)
#define LOG_TRACE3(message)
#define LOG_TRACE4(message)
#define LOG_TRACE5(message)
#define LOG_TRACE_STR1(stream)
#define LOG_TRACE_STR2(stream)
#define LOG_TRACE_STR3(stream)
#define LOG_TRACE_STR4(stream)
#define LOG_TRACE_STR5(stream)
#define TRACE_LEVEL_LOOP	0
#define TRACE_LEVEL_VAR		0
#define TRACE_LEVEL_COND	0
#define TRACE_LEVEL_OBJ		0
#define TRACE_LEVEL_FLOW	0

#define	LofarInitTracingModule
#define	LOCAL_TRACER_CONTEXT 
#define INIT_TRACER_CONTEXT(scope,contextname) 

#endif	// ENABLE_TRACER

//#
//# AUTO_FUNCTION_NAME
//#
// Define a the macro AUTO_FUNCTION_NAME that will be resolved by the
// (pre)compiler to hold the name of the function the macro was used in.
#if defined(HAVE_PRETTY_FUNCTION)
#	define AUTO_FUNCTION_NAME		__PRETTY_FUNCTION__
#elif defined(HAVE_FUNCTION)
#	define AUTO_FUNCTION_NAME		__FUNCTION__
#else
#	define AUTO_FUNCTION_NAME		"??"
#endif

//#
//# LOG_TRACE_LIFETIME(_STR) (level,message | stream)
//#
#define LOG_TRACE_LIFETIME(level,message) \
	log4cplus::TraceLogger		_tmpLifetimeTraceObj(getLogger().logger(), \
			LOFAR::formatString("%s:%s", AUTO_FUNCTION_NAME, message), \
			__FILE__, __LINE__); 

#define LOG_TRACE_LIFETIME_STR(level,stream) \
	ostringstream	oss; \
	oss << AUTO_FUNCTION_NAME << ":" << stream; \
	log4cplus::TraceLogger		_tmpLifetimeTraceObj(getLogger().logger(), \
								oss.str(), __FILE__, __LINE__); \
	}

	
//------------------------- LoggerReference class ---------------------------------

namespace LOFAR {

// The LoggerReference class is used for implementing faster logging
// for the trace levels. The class holds a Logger which is in fact a
// pointer to a shared Object.
class	LoggerReference {
public:
	LoggerReference(const std::string&	LoggerName) : 
			itsLogger(log4cplus::Logger::getInstance(LoggerName)) {	 }
	~LoggerReference() { }
	log4cplus::Logger&	logger()	{ return itsLogger; }

private:
	log4cplus::Logger			itsLogger;
};

extern LoggerReference	TraceLoggerRef;
inline LoggerReference&	getLogger() { return TraceLoggerRef; }

} // namespace LOFAR
using LOFAR::getLogger;


//#------------------------ Assert en FailWhen -------------------------------
//#
//# THROW_EXC (exception,stream)
//# ASSERT	  (condition,stream)
//# FAILWHEN  (condition,stream)
//#

namespace LOFAR {
	EXCEPTION_CLASS(AssertError,Exception)
}

#define THROW_EXC(exc,stream) { \
	std::ostringstream	oss;	\
	oss << stream;				\
	log4cplus::Logger::getInstance(PACKAGE ".EXCEPTION").log( \
					log4cplus::DEBUG_LOG_LEVEL, oss.str(), __FILE__, __LINE__); \
	} \
	throw (exc(stream, __HERE__))

#define ASSERT(cond,stream) \
	if (!(cond))  THROW_EXC(LOFAR::AssertError, "Assertion: " #cond)

#define FAILWHEN(cond,stream) \
	if (cond)  THROW_EXC(LOFAR::AssertError, "Failtest: " #cond)

#endif // file read before


