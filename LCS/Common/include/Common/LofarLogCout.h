//# LofarLogCout.h: Macro interface to the cout/cerr logging implementation
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

//# No include guard is used, because it should only be included indirectly
//# by LofarLogger.h (and by LofarLogCout.cc).

// \file
// Macro interface to the cout/cerr logging implementation.


#include <Common/lofar_iostream.h>
#include <Common/lofar_sstream.h>
#include <Common/lofar_iomanip.h>
#include <Common/lofar_string.h>
#include <Common/lofar_map.h>

#include <time.h>
#include <sys/time.h>
#include <cstdio>

//# This might be undefined if used by an external package like ASKAP.
#ifndef AUTO_FUNCTION_NAME
# define AUTO_FUNCTION_NAME __FUNCTION__
#endif


#ifndef DOXYGEN_SHOULD_SKIP_THIS

//# -------------------- Initialisation of the logger module -------------------
//#
// Initializes debug levels the file filename.debug
//	- INIT_LOGGER
//	- INIT_LOGGER_AND_WATCH
//
#define INIT_LOGGER(filename) \
	::LOFAR::LFDebug::initLevels (::LOFAR::string(filename) + ".debug")

//# Note: 'var' logger functionality not available
#define INIT_VAR_LOGGER(filename,logfile) \
	INIT_LOGGER(filename)

//# Note: 'watch' functionality not available
#define INIT_LOGGER_AND_WATCH(filename,interval) \
	INIT_LOGGER(filename)

#define INIT_LOGGER_WITH_SYSINFO(sinfo) \
        getLFDebugContext().initialize(); \
	::LOFAR::LFDebug::setLevel("Global",8); \
	::LOFAR::LFDebug::sysInfo = sinfo;

#define LOGCOUT_SETLEVEL(level) \
	::LOFAR::LFDebug::setLevel("Global",level);


//# -------------------- Log Levels for the Operator messages -----------------
//#
//# LOG_FATAL_(STR) (message|stream)
//# LOG_ERROR_(STR) (message|stream)
//# LOG_WARN_(STR)  (message|stream)
//# LOG_INFO_(STR)  (message|stream)
//#
//# levelnames need to be of the same length and contain a trailing space -- that way it can
//# be inserted into log lines immediately without formatting
#define LOG_FATAL(message)			cLog(1, "FATAL ", message)
#define LOG_FATAL_STR(stream) 		cLogstr(1, "FATAL ", stream)

#define LOG_ERROR(message) 			cLog(2, "ERROR ", message)
#define LOG_ERROR_STR(stream) 		cLogstr(2, "ERROR ", stream)

#define LOG_WARN(message) 			cLog(3, "WARN  ", message)
#define LOG_WARN_STR(stream)		cLogstr(3, "WARN  ", stream)

#define LOG_INFO(message) 			cLog(4, "INFO  ", message)
#define LOG_INFO_STR(stream) 		cLogstr(4, "INFO  ", stream)

//# -------------------- Log Levels for the Integrator -----------------
//#
//# LOG_DEBUG_(STR) (message|stream)
//#
#ifdef DISABLE_DEBUG_OUTPUT
#define LOG_DEBUG(message)
#define LOG_DEBUG_STR(stream)
#else
#define LOG_DEBUG(message) 			cDebug(5, "DEBUG ", message)
#define LOG_DEBUG_STR(stream) 		cDebugstr(5, "DEBUG ", stream)
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
  static LFDebug::Context DebugContext; \
  static inline LFDebug::Context & getLFDebugContext() \
            { return DebugContext; }

#define INIT_TRACER_CONTEXT(scope, contextname)  \
  LFDebug::Context scope::DebugContext(contextname)

#define ALLOC_TRACER_ALIAS(other)  \
public: \
	static inline LFDebug::Context & getLFDebugContext() \
		{ return other::getLFDebugContext(); }

#define LOG_TRACE_LOOP(message)		cTrace(TRACE_LEVEL_LOOP, message)
#define LOG_TRACE_VAR(message)		cTrace(TRACE_LEVEL_VAR, message)
#define LOG_TRACE_CALC(message)		cTrace(TRACE_LEVEL_CALC, message)
#define LOG_TRACE_COND(message)		cTrace(TRACE_LEVEL_COND, message)
#define LOG_TRACE_STAT(message)		cTrace(TRACE_LEVEL_STAT, message)
#define LOG_TRACE_OBJ(message)		cTrace(TRACE_LEVEL_OBJ, message)
#define LOG_TRACE_RTTI(message)		cTrace(TRACE_LEVEL_RTTI, message)
#define LOG_TRACE_FLOW(message)		cTrace(TRACE_LEVEL_FLOW, message)
#define LOG_TRACE_LOOP_STR(stream)	cTracestr(TRACE_LEVEL_LOOP, stream)
#define LOG_TRACE_VAR_STR(stream)	cTracestr(TRACE_LEVEL_VAR, stream)
#define LOG_TRACE_CALC_STR(stream)	cTracestr(TRACE_LEVEL_CALC, stream)
#define LOG_TRACE_COND_STR(stream)	cTracestr(TRACE_LEVEL_COND, stream)
#define LOG_TRACE_STAT_STR(stream)	cTracestr(TRACE_LEVEL_STAT, stream)
#define LOG_TRACE_OBJ_STR(stream)	cTracestr(TRACE_LEVEL_OBJ, stream)
#define LOG_TRACE_RTTI_STR(stream)	cTracestr(TRACE_LEVEL_RTTI, stream)
#define LOG_TRACE_FLOW_STR(stream)	cTracestr(TRACE_LEVEL_FLOW, stream)
#define TRACE_LEVEL_LOOP			18
#define TRACE_LEVEL_VAR				17
#define TRACE_LEVEL_CALC			16
#define TRACE_LEVEL_COND			15
#define TRACE_LEVEL_STAT			14
#define TRACE_LEVEL_OBJ				13
#define TRACE_LEVEL_RTTI			12
#define TRACE_LEVEL_FLOW			11

//# The numbering of trace levels in log4cplus is reversed compared to Debug.
//# This macro converts the Debug trace level to the log4cplus trace level.
#define LOG4CPLUS_LEVEL(level) (TRACE_LEVEL_LOOP-(level)+1)

//#
//# LOG_TRACE_LIFETIME(_STR) (level, message|stream)
//#
#define LOG_TRACE_LIFETIME_STR(level, stream) \
	if( LFDebugCheck(level) ) { \
		::LOFAR::LFDebug::Tracer objname; \
	        std::ostringstream lfr_log_oss; \
	        lfr_log_oss << stream; \
		objname.startMsg (LOG4CPLUS_LEVEL(level), __FILE__, __LINE__, \
                        AUTO_FUNCTION_NAME, lfr_log_oss.str().c_str(), 0); \
	}

#define LOG_TRACE_LIFETIME(level,message) \
	LOG_TRACE_LIFETIME_STR(level, message)


#else	// ENABLE_TRACER
//# define dummies if tracing is disabled
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

#define LofarInitTracingModule
#define ALLOC_TRACER_CONTEXT 
#define ALLOC_TRACER_ALIAS(other)
#define INIT_TRACER_CONTEXT(scope,contextname) 
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
#define THROW(exc,msg) do { \
	  std::ostringstream lfr_log_oss; \
	  lfr_log_oss << msg; \
	  throw(exc(lfr_log_oss.str(), THROW_ARGS)); \
	} while(0)

//# ---------- implementation details generic part ----------

#define LFDebugCheck(level)	getLFDebugContext().check(level)

#define	thread_unsafe_cLog(level,levelname,message) \
	do { \
	  if (::LOFAR::LFDebug::LFDebugCheck(level)) \
	      ::LOFAR::LFDebug::getDebugStream() << ::LOFAR::LFDebug::sysInfo << ::LOFAR::LFDebug::spaced_time_string << levelname \
			<< message << std::endl; \
	} while (0)

#if defined USE_THREADS
// make sure that the logging is 
//   a) thread safe, using a mutex, which is unlocked even if thread_unsafe_cLog throws
//   b) does not trigger on a pthread_cancel, because stl will cause an abort() due to
//      not rethrowing after a catch(...) in ostream::put(char) (fixed in GCC 4.4+, maybe in 4.3 as well).
//   c) will not cancel halfway through printing a log line when using pthread_cancel in GCC 4.4+,
//      as that would absorb the next log line since both will be put on the same line.

// an example crash test will reveal the problem (GCC <=4.2):
/*
#include <iostream>
#include <pthread.h>

void *thread(void*)
{
  // ostream::put(char) will do a catch(...), discarding the pthread cancel
  // "exception", which is fatal.
  for(;;) std::cout << "crash" << std::endl;
  return 0;
}

int main() {
  pthread_t t;
  pthread_create( &t, 0, &thread, 0 );

  pthread_cancel( t );
  pthread_join( t, 0 );
}
*/
#define	cLog(level,levelname,message) \
	do { \
                struct D { \
                   D() { pthread_mutex_lock(&::LOFAR::LFDebug::mutex); \
                         pthread_setcancelstate( PTHREAD_CANCEL_DISABLE, &oldCancelState ); \
                       } \
                  ~D() { \
                         pthread_setcancelstate( oldCancelState, 0 ); \
                         pthread_mutex_unlock(&::LOFAR::LFDebug::mutex); \
                       } \
                   int oldCancelState; \
                } pthread_control; \
                (void)pthread_control; \
                \
		thread_unsafe_cLog(level,levelname,message); \
	} while(0)
#else
#define	cLog(level,levelname,message) \
	do { \
		thread_unsafe_cLog(level,levelname,message); \
	} while(0)
#endif

// thread_unsafe_cLog can handle both strings and streams
#define cLogstr cLog

#define	cDebug cLog
#define	cDebugstr cDebug

#define thread_unsafe_cTrace(level,message) \
	do { \
	  if (::LOFAR::LFDebug::LFDebugCheck(level)) \
	      ::LOFAR::LFDebug::getDebugStream() << ::LOFAR::LFDebug::sysInfo << ::LOFAR::LFDebug::spaced_time_string << "TRACE" << LOG4CPLUS_LEVEL(level) \
			<< " TRC." << getLFDebugContext().name() << " " \
			<< message << std::endl; \
	} while(0)

#if defined USE_THREADS
#define cTrace(level,message) \
	do { \
		pthread_mutex_lock(&::LOFAR::LFDebug::mutex); \
		thread_unsafe_cTrace(level,message); \
		pthread_mutex_unlock(&::LOFAR::LFDebug::mutex); \
	} while(0)
#else
#define cTrace(level,message) \
	do { \
		thread_unsafe_cTrace(level,message); \
	} while(0)
#endif

#define cTracestr cTrace

//#-------------------- END OF MACRO DEFINITIONS --------------------#//

namespace LOFAR
{
  namespace LFDebug
  {
    extern string sysInfo;

#if defined USE_THREADS
    extern pthread_mutex_t mutex;
#endif

    extern std::ostream * dbg_stream_p;
  
    inline std::ostream & getDebugStream () { return *dbg_stream_p; }

    // sets level of given context
    bool setLevel (const string &context,int level);
     
    void initLevels (const string& fname);
    // loads debug levels from file.
    void loadLevels (const string& fname);

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

    std::ostream& spaced_time_string (std::ostream &str);

    extern Context DebugContext;
    inline Context & getLFDebugContext ()  { return DebugContext; }

  } // namespace LFDebug


  // Default DebugContext is the one in LFDebug.
  using LFDebug::getLFDebugContext;

} // namespace LOFAR

#endif // DOXYGEN_SHOULD_SKIP_THIS
