//  LofarLogCout.h: Macro interface to the cout/cerr logging implementation
//
//  Copyright (C) 2004
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$

#if !defined(COMMON_LOFAR_LOGCOUT_H)
#define COMMON_LOFAR_LOGCOUT_H

#include <Common/lofar_iostream.h>
#include <Common/lofar_string.h>
#include <Common/lofar_map.h>
#include <Common/Exception.h>
#include <sstream>
#include <stdexcept>
#include <stdio.h>
#ifdef ENABLE_LATENCY_STATS
#include <sys/time.h>
#endif

//# -------------------- Initialisation of the logger module -------------------
//#
// Initializes debug levels from command line (looks for options of
// the form -dContext=#, or -d# for all levels, or filename.debug
// to load from file, or -dl to load from default progname.debug file)
//	- INIT_LOGGER
//	- INIT_LOGGER_AND_WATCH
//
#define INIT_LOGGER(filename) \
	getDebugContext().setLevel(20); \
	const char*	myargv[2] =  { "", filename }; \
	Debug::initLevels (2, myargv, true)

//# Note: 'watch' functionality not available
#define INIT_LOGGER_AND_WATCH(filename,interval) \
	INIT_LOGGER(filename)

//# -------------------- Log Levels for the Operator messages -----------------
//#
//# LOG_FATAL_(STR) (message|stream)
//# LOG_ERROR_(STR) (message|stream)
//# LOG_WARN_(STR)  (message|stream)
//# LOG_INFO_(STR)  (message|stream)
//#
#define LOG_FATAL(message)			clog(1, "FATAL", message)
#define LOG_FATAL_STR(stream) 		clogstr(1, "FATAL", stream)

#define LOG_ERROR(message) 			clog(2, "ERROR", message)
#define LOG_ERROR_STR(stream) 		clogstr(2, "ERROR", stream)

#define LOG_WARN(message) 			clog(3, "WARN", message)
#define LOG_WARN_STR(stream)		clogstr(3, "WARN", stream)

#define LOG_INFO(message) 			clog(4, "INFO", message)
#define LOG_INFO_STR(stream) 		clogstr(4, "INFO", stream)

//# -------------------- Log Levels for the Integrator -----------------
//#
//# LOG_DEBUG_(STR) (message|stream)
//#
#ifdef DISABLE_DEBUG_OUTPUT
#define LOG_DEBUG(message)
#define LOG_DEBUG_STR(stream)
#else
#define LOG_DEBUG(message) 			cdebug(5, "DEBUG", message)
#define LOG_DEBUG_STR(stream) 		cdebugstr(5, "DEBUG", stream)
#endif	// DISABLE_DEBUG_OUTPUT

//# -------------------- Trace Levels for the Programmer -----------------
//#
//#	ALLOC_TRACER_CONTEXT
//#	INIT_TRACER_CONTEXT		(scope, contextname)
//# ALLOC_TRACER_ALIAS		(parent)
//# LOG_DEBUG_<type>(STR) 	(message|stream)
//#
#ifdef ENABLE_TRACER
#define ALLOC_TRACER_CONTEXT  \
public:	\
  static Debug::Context DebugContext; \
  static inline Debug::Context & getDebugContext() \
            { return DebugContext; }

#define INIT_TRACER_CONTEXT(scope, contextname)  \
  Debug::Context scope::DebugContext(contextname)

#define ALLOC_TRACER_ALIAS(other)  \
  static inline Debug::Context & getDebugContext() \
  { return other::getDebugContext(); }

#define LOG_TRACE_LOOP(message)		ctrace(TRACE_LEVEL_LOOP, message)
#define LOG_TRACE_VAR(message)		ctrace(TRACE_LEVEL_VAR, message)
#define LOG_TRACE_CALC(message)		ctrace(TRACE_LEVEL_CALC, message)
#define LOG_TRACE_COND(message)		ctrace(TRACE_LEVEL_COND, message)
#define LOG_TRACE_STAT(message)		ctrace(TRACE_LEVEL_STAT, message)
#define LOG_TRACE_OBJ(message)		ctrace(TRACE_LEVEL_OBJ, message)
#define LOG_TRACE_RTTI(message)		ctrace(TRACE_LEVEL_RTTI, message)
#define LOG_TRACE_FLOW(message)		ctrace(TRACE_LEVEL_FLOW, message)
#define LOG_TRACE_LOOP_STR(stream)	ctracestr(TRACE_LEVEL_LOOP, stream)
#define LOG_TRACE_VAR_STR(stream)	ctracestr(TRACE_LEVEL_VAR, stream)
#define LOG_TRACE_CALC_STR(stream)	ctracestr(TRACE_LEVEL_CALC, stream)
#define LOG_TRACE_COND_STR(stream)	ctracestr(TRACE_LEVEL_COND, stream)
#define LOG_TRACE_STAT_STR(stream)	ctracestr(TRACE_LEVEL_STAT, stream)
#define LOG_TRACE_OBJ_STR(stream)	ctracestr(TRACE_LEVEL_OBJ, stream)
#define LOG_TRACE_RTTI_STR(stream)	ctracestr(TRACE_LEVEL_RTTI, stream)
#define LOG_TRACE_FLOW_STR(stream)	ctracestr(TRACE_LEVEL_FLOW, stream)
#define TRACE_LEVEL_LOOP			18
#define TRACE_LEVEL_VAR				17
#define TRACE_LEVEL_CALC			16
#define TRACE_LEVEL_COND			15
#define TRACE_LEVEL_STAT			14
#define TRACE_LEVEL_OBJ				13
#define TRACE_LEVEL_RTTI			12
#define TRACE_LEVEL_FLOW			11

//#
//# LOG_TRACE_LIFETIME(_STR) (level, message|stream)
//#
#define LOG_TRACE_LIFETIME_STR(level, stream) \
	Debug::Tracer objname; \
    if( Debug(level) ) { \
		constructStream(stream) \
		objname.startMsg (level, __FILE__, __LINE__, \
                        __PRETTY_FUNCTION__, 0, oss.str().c_str()); \
    }
#define LOG_TRACE_LIFETIME(level,message) \
	LOG_TRACE_LIFETIME_STR(level, message)

//# ---------- implementation details tracer part ----------
#define ctrace(level, message)		cdebug(level, "TRACE" << level, message)
#define ctracestr(level,stream) 	cdebugstr(level, "TRACE" << level, stream)

#else	// ENABLE_TRACER
//# define dummies if tracing is disabled
#define ALLOC_TRACER_CONTEXT 
#define INIT_TRACE_CONTEXT(scope, contextname) 
#define ALLOC_TRACER_ALIAS(other) 

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
#define TRACE_LEVEL_LOOP			0
#define TRACE_LEVEL_VAR				0
#define TRACE_LEVEL_CALC			0
#define TRACE_LEVEL_COND			0
#define TRACE_LEVEL_STAT			0
#define TRACE_LEVEL_OBJ				0
#define TRACE_LEVEL_RTTI			0
#define TRACE_LEVEL_FLOW			0

#define LOG_TRACE_LIFETIME_STR(level, stream)
#define LOG_TRACE_LIFETIME(level,message)

#endif	// ENABLE_TRACER

//# -------------------- Assert and FailWhen --------------------
//#
//# THROW			(exception,stream)
//# (DBG)ASSERT		(condition,stream)
//# (DBG)FAILWHEN	(condition,stream)
//#
//# Note: only THROW needs to be defines here, the others are buiold on THROW
//# in the LofarLogger.h file.

#undef THROW
// possible object debug status. 
#define THROW(exc,msg)  { \
	constructStream(msg) \
	clog(1, "EXCEPTION", oss.str()); \
	throw(exc(oss.str(), __HERE__)); \
	}

//# ---------- implementation details generic part ----------

#define Debug(level)	getDebugContext().check(level)

#define DebugTestAndLog(level)	if (Debug(level) && Debug::stream_time()) \
									Debug::getDebugStream()
#define	constructStream(stream) \
	std::ostringstream	oss; \
	oss << stream;

#define	clog(level,levelname,message) \
	DebugTestAndLog(level) << levelname << "-[" << LOFARLOGGER_PACKAGE << "] " << \
								  message << endl;

#define clogstr(level,levelname,stream) { \
		constructStream(stream) \
		clog(level,levelname,oss.str().c_str()) \
	}

#define	cdebug(level,levelname,message) \
	DebugTestAndLog(level) << levelname << "-[" << LOFARLOGGER_PACKAGE << "] " << \
								  message << \
								  ", File:" << __FILE__ << \
								  ", Line:" << __LINE__ << endl;

#define cdebugstr(level,levelname,stream)  { \
		constructStream(stream) \
		cdebug(level,levelname,oss.str().c_str()) \
	}


//#-------------------- END OF MACRO DEFINITIONS --------------------#//

namespace LOFAR
{

  namespace Debug
  {
    extern ostream * dbg_stream_p;
  
    inline ostream & getDebugStream () { return *dbg_stream_p; }

#ifdef ENABLE_LATENCY_STATS
    extern struct timeval tv_init;
    inline int printf_time ()
    { 
      struct timeval tv; gettimeofday(&tv,0);
      printf("%ld.%06ld ",tv.tv_sec-tv_init.tv_sec,tv.tv_usec);
      return 1;
    }
    inline int stream_time ()
    {
      struct timeval tv; gettimeofday(&tv,0);
      getDebugStream()<<tv.tv_sec-tv_init.tv_sec<<"."<<tv.tv_usec;
      return 1;
    }
#else
    inline int printf_time () { return 1; };
    inline int stream_time () { return 1; };
#endif
  };

  namespace Debug
  {
    // Typedef the exception type, so we can change whenever needed.
    EXCEPTION_CLASS(Fail,LOFAR::Exception);

    // sets level of given context
    bool setLevel (const string &context,int level);
     
    void	initLevels (int argc,const char *argv[], bool save=true);
    // saves debug to file (default: progname.debug) 
    bool saveLevels ( string fname = "" );
    // loads debug levels from file (default: progname.debug) 
    void loadLevels ( string fname = "" );
    // redirects debug output to file
    int redirectOutput (const string &fname);

    // copies string into static buffer. Thread-safe (i.e. each thread
    // has its own buffer)
    const char * staticBuffer( const string &str );

    // appends strings and inserts a space, if needed
    string& append( string &str,const string &str2,const string &sep = " " );
    // sprintfs to a string object, returns it
    const string ssprintf( const char *format,... );
    // sprintfs to a string, with append & insertion of spaces
    int appendf( string &str,const char *format,... );
  
    class Context 
    {
    public:
      Context (const string &name, Context *parent_ = 0);
      ~Context();
    
      bool check (int level) const;
      int level () const;
      int setLevel (int value);
      const string& name () const;
      static void initialize ();
        
    private:
      static bool initialized;
      int debug_level;
      string context_name;
      Context *parent;
    };

    //## Other Operations (inline)
    inline bool Context::check (int level) const
    {
      if( !initialized )
        return false;
      if( parent && parent->check(level) )
        return true;
      return level <= debug_level;
    }

    inline int Context::level () const
    {
      return debug_level;
    }

    inline int Context::setLevel (int value)
    {
      debug_level = value;
      return value;
    }

    inline const string& Context::name () const
    {
      return context_name;
    }
  
    inline void Context::initialize ()
    {
      initialized = true;
    }

#ifdef ENABLE_TRACER
    class Tracer
    {
    public:
      Tracer() : itsDo(false) {}

      void startMsg (int level,
                     const char* file, int line, const char* func,
                     const char* msg, const void* objPtr);

      ~Tracer()
      { if (itsDo) endMsg(); }

    private:
      Tracer(const Tracer&);
      Tracer& operator= (const Tracer&);
      void endMsg();

      bool    itsDo;
      int     itsLevel;
      string  itsMsg;
    };
#endif

  } // namespace Debug


  namespace Debug {
    extern Context DebugContext;
    inline Context & getDebugContext ()  { return DebugContext; }
  }

  // Default DebugContext is the one in Debug.
  using Debug::getDebugContext;

} // namespace LOFAR

#endif	// file read before
