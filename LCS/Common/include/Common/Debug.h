//  Debug.h:
//
//  Copyright (C) 2002
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
//
//  $Log$
//  Revision 1.1  2003/08/21 11:20:32  diepen
//  Moved Common to LCS
//
//  Revision 1.16  2003/05/27 09:09:08  diepen
//  Also print error in AssertMsg
//
//  Revision 1.15  2002/11/26 08:02:33  diepen
//  %[BugId: 76]%
//  Use ostringstream instead of ostrstream
//
//  Revision 1.14  2002/10/30 10:02:56  diepen
//
//  %[BugId: 26]%
//  Fixed incorrect definition of DbgFailWhen
//
//  Revision 1.13  2002/10/29 12:21:30  smirnov
//  %[BugId: 26]%
//  Added a DbgFailWhen() macro.
//  Migrated to new Rose C++ add-in, hence a bunch of changes in the Rose
//  markup comments.
//
//  Revision 1.12  2002/08/28 09:11:16  smirnov
//  %[BugId: 26]%
//  Added CheckConfig class to verify configurations options at run-time.
//  Small fixes to Debug and Thread.
//
//  Revision 1.11  2002/08/06 08:56:53  diepen
//  %[BugId: 76]%
//  Removed.str() in TRACERF macro
//
//  Revision 1.10  2002/08/02 14:42:03  brentjens
//
//  %[BugId: 76]%
//
//  Cancelled previous change...
//
//  Revision 1.9  2002/08/02 10:06:38  brentjens
//
//  %[BugId: 76]%
//
//  TRACERPFN_INTERNAL: added own "{ }" block around the code to
//  prevent error:
//
//  xxx_tmp_tracer_xxx previously defined
//
//  which occurred whenever TRACERPF2 was used more than once in the same
//  {} block.
//
//  Revision 1.8  2002/07/03 14:15:29  smirnov
//  %[BugId: 26]%
//  Various fixes for multithreading and gcc 3.1.
//  Added the Thread package.
//  Added a Stopwatch class.
//
//  Revision 1.7  2002/05/31 15:04:03  smirnov
//  %[BugId: 26]%
//  Fixed default sdebug() to return string instead of const char *
//
//  Revision 1.6  2002/05/15 14:57:19  oms
//  Re-added the Throw1, Assert1, FailWhen1, etc. macros that do not include
//  an sdebug() call. The normal versions were causing trouble inside of static
//  methods of classes that defined a non-static sdebug().
//
//  Revision 1.5  2002/05/14 06:47:18  gvd
//  Fixed ifdef in Debug.h and include in tDebug
//
//  Revision 1.4  2002/05/13 15:01:03  oms
//  Fixed bug in initLevels().
//  Added initialized flag.
//
//  Revision 1.3  2002/05/08 12:26:11  oms
//  Added checking of parent context (where did it go, anyway)?
//
//  Revision 1.2  2002/05/07 11:44:25  gvd
//  Use COMMON_ instead of BASESIM_ in include guards
//  Adapted Debug to Oleg's changes
//
//  Revision 1.1  2002/05/03 10:46:33  gvd
//  Common source files
//
//  Revision 1.2  2002/03/14 14:25:46  wierenga
//  system includes before local includes
//
//  Revision 1.1  2002/03/01 08:27:57  gvd
//  Replaced firewall by Debug and changed code accordingly
//  Added lofar_*.h for correct use of namespaces (for KAI and Intel C++)
//


#ifndef COMMON_DEBUG_H
#define COMMON_DEBUG_H

#include <Common/lofar_iostream.h>
#include <Common/lofar_string.h>
#include <Common/lofar_map.h>
#include <sstream>
#include <stdexcept>
#include <stdio.h>
#ifdef ENABLE_LATENCY_STATS
#include <sys/time.h>
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// The system supports multiple debugging contexts, implemented as
// namespaces. A context has independent debug message levels.
// You can define the debugging context by
// nesting it inside your package namespace (or even in a class declaration).
// The following macros, when placed in the header _inside_ your class,
// will declare the required Debug context.
//
//    LocalDebugContext;
//
// Then, the .cc file must define an implementation of the context.
// To do so, insert this macro
//
//    InitDebugContext(class,contextname);
//
// This will add the code required to implement the context.
//
// It is possible to declare in a class that the context of another class
// should be used. This can be done by inserting in the class
//
//    LocalDebugAlias(otherclass)
//
// If a specific debug context has to be used in a .cc file, one can do
// so by defining the macro getDebugContext() like:
//
//    #ifdef getDebugContext
//    #undef getDebugContext
//    #endif
//    #define getDebugContext() XX::getDebugContext()



// This macro declares a local debug context within a class. Use public
// or protected access to have subclasses inherit the context, else use
// private.
#define LocalDebugContext \
  private: static ::Debug::Context DebugContext; \
  public: static inline ::Debug::Context & getDebugContext() \
            { return DebugContext; }
#define LocalDebugSubContext LocalDebugContext;
// This macro declares a debug context within this class that is aliased
// from some other class's local context. Use public or protected access to 
// have subclasses inherit the context, else use private.
#define LocalDebugAlias(other) \
  public: static inline ::Debug::Context & getDebugContext() \
            { return other::getDebugContext(); }

// This macro adds necessary implementation of a local debug context. If you
// declare a local context, then it must be present in your class's .cc file. 
#define InitDebugContext(scope,name) \
  ::Debug::Context scope::DebugContext(name)
#define InitDebugSubContext(scope,parent,name) ::Debug::Context scope::DebugContext(name,&(parent::getDebugContext()));

// This macro adds necessary implementation of an aliased debug context. If you
// declare a local context, then it must be present in your class's .cc file. 
#define InitDebugAlias(scope,otherscope) ::Debug::Context & scope##::DebugContext(otherscope::getDebugContext());

// use this to check a numeric debug level
#ifdef DISABLE_DEBUG_OUTPUT
#define Debug(level) (0)
#else
#define Debug(level) getDebugContext().check(level)
#endif

// use this to get the name of the current context
#define DebugName       (getDebugContext().name())
#define DebugLevel      (getDebugContext().level())

namespace Debug
{
  extern ostream & dbg_stream;

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
    dbg_stream<<tv.tv_sec-tv_init.tv_sec<<"."<<tv.tv_usec;
    return 1;
  }
#else
  inline int printf_time () { return 1; };
  inline int stream_time () { return 1; };
#endif
};

// Use this macro to conditionally printf a debugging message.
//    dprintf(1)(format,whatever)
#define dprintf1(level) if( Debug(level) && Debug::printf_time() ) printf
#define dprintf(level) if( Debug(level) && Debug::printf_time() ) printf("%s: ",sdebug(0).c_str()),printf
// use this macro to conditionally stream a debugging message.
//    cdebug(1)<<whatever
#define cdebug1(level)  if( Debug(level) && Debug::stream_time() ) ::Debug::dbg_stream
#define cdebug(level)  cdebug1(level)<<sdebug(0)<<": "


// Use this macro to write trace output.
// Similar as dprintf, but uses iostream instead of printf.
// Optionally it can be compiled out.
// A trace message is printed if its level is <= the runtime trace level.
// Thus the lower the message level, the more often it gets printed.
#ifdef ENABLE_TRACER
# define TRACER(level,stream) cdebug1(level) << "trace" << level << ": " << stream << endl
#else
# define TRACER(level,stream)
#endif
#define TRACER1(stream) TRACER(1,stream)
#define TRACER2(stream) TRACER(2,stream)
#define TRACER3(stream) TRACER(3,stream)
#define TRACER4(stream) TRACER(4,stream)

// Use this macro to write file and line (and possibly function) as well.
#ifdef ENABLE_TRACER
# if defined(HAVE_PRETTY_FUNCTION)
#  define TRACERF(level,stream) cdebug1(level) << "trace" << level << ' ' << __FILE__ << ':' << __LINE__ << '(' << __PRETTY_FUNCTION__ << "): " << stream << endl
# elif defined(HAVE_FUNCTION)
#  define TRACERF(level,stream) cdebug1(level) << "trace" << level << ' ' << __FILE__ << ':' << __LINE__ << '(' << __FUNCTION__ << "): " << stream << endl
# else
#  define TRACERF(level,stream) cdebug1(level) << "trace" << level << ' ' << __FILE__ << ':' << __LINE__ << ": " << stream << endl
# endif
#else
# define TRACERF(level,stream)
#endif
#define TRACERF1(stream) TRACERF(1,stream)
#define TRACERF2(stream) TRACERF(2,stream)
#define TRACERF3(stream) TRACERF(3,stream)
#define TRACERF4(stream) TRACERF(3,stream)

// This macro creates a Tracer object, so you get an automatic trace
// message at the end of a scope.
// objname gives the name of the Tracer object to create.
// Usually only one Tracer object will be used in a scope. In such a case
// the macro TRACERPF can be used (which uses a standard object name).
#ifdef ENABLE_TRACER
# define TRACERPFN_INTERNAL(objname,level,funcName,objPtr,stream) \
    ::Debug::Tracer objname; \
    if( Debug(level) ) { \
      std::ostringstream trace_oss; \
      trace_oss << stream; \
      objname.startMsg (level, __FILE__, __LINE__, \
                        funcName, objPtr, trace_oss.str().c_str()); \
    }
#else
# define TRACERPFN_INTERNAL(objname,level,funcName,objPtr,stream)
#endif

#ifdef HAVE___FUNCTION__
# define TRACERPFN(objname,level,stream) \
     TRACERPFN_INTERNAL(objname,level,__FUNCTION__,0,stream)
#else
# define TRACERPFN(objname,level,stream) \
     TRACERPFN_INTERNAL(objname,level,0,0,stream)
#endif

#define TRACERPFN1(objname,stream) TRACERPFN(objname,1,stream)
#define TRACERPFN2(objname,stream) TRACERPFN(objname,2,stream)
#define TRACERPFN3(objname,stream) TRACERPFN(objname,3,stream)
#define TRACERPFN4(objname,stream) TRACERPFN(objname,4,stream)

#define TRACERPF(level,stream) TRACERPFN(xxx_tmp_tracer_xxx,level,stream)
#define TRACERPF1(stream) TRACERPF(1,stream)
#define TRACERPF2(stream) TRACERPF(2,stream)
#define TRACERPF3(stream) TRACERPF(3,stream)
#define TRACERPF4(stream) TRACERPF(4,stream)


// The SourceFileLine macro creates a string containing the current
// filename and line number.
// The CodeStatus1 macro adds the debug context, plus a message. This is 
// handy for forming error and exception messages.
// If your class defines a "sdebug()" method returning the current object
// status (as string or char *), then use CodeStatus instead, to include 
// sdebug() info in your message.
#ifdef HAVE___FUNCTION__
# define SourceFileLine ::Debug::ssprintf("%s:%d(%s)",__FILE__,__LINE__,__FUNCTION__)
#else 
# define SourceFileLine ::Debug::ssprintf("%s:%d",__FILE__,__LINE__)
#endif
#define CodeStatus1(msg) ((msg)+string(" (context: ")+DebugName+", at "+ SourceFileLine + ")")
#define CodeStatus(msg) ("["+(string)sdebug()+("] ")+CodeStatus1(msg))

// This inserts declarations of the sdebug() and debug() methods into your class.
// Use DeclareDebugInfo(virtual) to declare a virtual sdebug().
// Else use DeclareDebugInfo().
// The following method declarations are inserted:
//    <qualifiers> string sdebug ( int detail = 1,const string &prefix = "",
//              const char *name = 0 ) const;
//    const char * debug ( int detail = 1,const string &prefix = "",
//                         const char *name = 0 ) const
//    { return Debug::staticBuffer(sdebug(detail,prefix,name)); }
//
#define Declare_sdebug(qualifiers) qualifiers string sdebug ( int detail = 1,const string &prefix = "",const char *name = 0 ) const; 
#define Declare_debug(qualifiers) qualifiers const char * debug ( int detail = 1,const string &prefix = "",const char *name = 0 ) const { return Debug::staticBuffer(sdebug(detail,prefix,name)); }

// this global definition of sdebug allows the use of "non-1" macros everywhere
inline string sdebug(int=0) { return ""; };

// The Throw macro throws an exception, using
// CodeStatus to add on filename, line, current debugging 
// context, and possible object debug status. 
// The default exception type is Debug::Error which is currently
// typdef-ed as std::logic_error.

const char exception_message[] = "\n==================================== EXCEPTION ================================\n\n";
#define Throw(msg)  { ::Debug::dbg_stream<<exception_message<<CodeStatus(msg)<<"\n"; throw(Debug::Error(CodeStatus(msg))); }
#define Throw1(msg)  { ::Debug::dbg_stream<<exception_message<<CodeStatus1(msg)<<"\n"; throw(Debug::Error(CodeStatus1(msg))); }

// The Assert macro will do a Throw if condition is FALSE.
#define Assert(cond)  { if( !(cond) ) Throw("Assert failed: " #cond); }
#define Assert1(cond)  { if( !(cond) ) Throw1("Assert failed: " #cond); }

// The FailWhen macro will Throw a message if condition is TRUE
// Always defined (even with debugging off)
#define FailWhen(cond,msg)  { if( cond ) Throw(msg); }
#define FailWhen1(cond,msg)  { if( cond ) Throw1(msg); }

// The DbgFailWhen macro is like FailWhen, but
// defined to do nothing if debugging is off.
#ifdef ENABLE_DBGASSERT
# define DbgFailWhen(cond,msg)  FailWhen(cond,msg)
# define DbgFailWhen1(cond,msg)  FailWhen1(cond,msg)
#else
# define DbgFailWhen(cond,msg)  
# define DbgFailWhen1(cond,msg)  
#endif

// The DbgAssert macro is like Assert, but
// defined to do nothing if debugging is off.
#ifdef ENABLE_DBGASSERT
# define DbgAssert(cond) { if( !(cond) ) Throw("DbgAssert failed: " #cond); }
# define DbgAssert1(cond) { if( !(cond) ) Throw1("DbgAssert failed: " #cond); }
#else
# define DbgAssert(cond)
# define DbgAssert1(cond)
#endif

// The AssertStr macro makes it possible to put arbitrary data in
// the exception message.
// E.g.
//   AssertStr (n < 10, "value " << n << " exceeds maximum");
#define AssertStr(cond,stream) \
 { if( !(cond) ) { \
     std::ostringstream oss; \
     oss << stream; \
     Throw("Assertion `" #cond "' failed: " + oss.str()); \
 }}

// The DbgAssertStr macro is like AssertStr, but
// defined to do nothing if debugging is off.
#ifdef ENABLE_DBGASSERT
# define DbgAssertStr(cond,stream) \
 { if( !(cond) ) { \
     std::ostringstream oss; \
     oss << stream; \
     Throw("DbgAssert `" #cond "' failed: " + oss.str()); \
 }}
#else
# define DbgAssertStr(cond,stream)
#endif

// The AssertMsg macro is similar to AssertStr, but it does not
// include source and line number in the message.
// It can be used for generating 'normal' error messages.
#define AssertMsg(cond,stream) \
 { if( !(cond) ) { \
     std::ostringstream oss; \
     oss << stream; \
     ::Debug::dbg_stream << oss.str() << std::endl; \
     throw Debug::Error(oss.str()); \
 }}



namespace Debug
{
  // Typedef the exception type, so we can change whenever needed.
//##ModelId=3DB9546401F6
  typedef std::logic_error Error;

  // sets level of given context
  bool setLevel (const string &context,int level);
      
  // initializes debug levels from command line (looks for options of
  // the form -dContext=#, or -d# for all levels, or filename.debug
  // to load from file, or -dl to load from default progname.debug file)
  void initLevels   (int argc,const char *argv[],bool save=true);
  // saves debug to file (default: progname.debug) 
  bool saveLevels ( string fname = "" );
  // loads debug levels from file (default: progname.debug) 
  void loadLevels ( string fname = "" );

  // copies string into static buffer. NB: current implementation is not
  // thread-safe, but can be made so.
  const char * staticBuffer( const string &str );

  // appends strings and inserts a space, if needed
  string& append( string &str,const string &str2,const string &sep = " " );
  // sprintfs to a string object, returns it
  const string ssprintf( const char *format,... );
  // sprintfs to a string, with append & insertion of spaces
  int appendf( string &str,const char *format,... );
  
  // helper functions and declarations
  int dbg_printf( const char *format,... );


//##ModelId=3C21B55E02FC
  class Context 
  {
  public:
    //##ModelId=3C21B594005B
    Context (const string &name, Context *parent_ = 0);

    //##ModelId=3DB95464028D
    ~Context();
    
    //##ModelId=3C21B9750352
    bool check (int level) const;

    //##ModelId=3DB954640293
    int level () const;
    //##ModelId=3DB95464029E
    int setLevel (int value);

    //##ModelId=3DB9546402B5
    const string& name () const;

    //##ModelId=3DB9546402C2
    static void initialize ();
        
  private:
        
    //##ModelId=3DB954640264
    static bool initialized;
    //##ModelId=3C21B57C027D
    int debug_level;
    //##ModelId=3C21B5800193
    string context_name;
    //##ModelId=3CD68637038B
    Context *parent;
  };


//##ModelId=3C21B9750352
  //## Other Operations (inline)
  inline bool Context::check (int level) const
  {
    if( !initialized )
      return false;
    if( parent && parent->check(level) )
      return true;
    return level <= debug_level;
  }

//##ModelId=3DB954640293
  inline int Context::level () const
  {
    return debug_level;
  }

//##ModelId=3DB95464029E
  inline int Context::setLevel (int value)
  {
    debug_level = value;
    return value;
  }

//##ModelId=3DB9546402B5
  inline const string& Context::name () const
  {
    return context_name;
  }
  
//##ModelId=3DB9546402C2
  inline void Context::initialize ()
  {
    initialized = true;
  }

#ifdef ENABLE_TRACER
//##ModelId=3DB954640201
  class Tracer
  {
  public:
    //##ModelId=3DB9546402E2
    Tracer() : itsDo(false) {}

    //##ModelId=3DB9546402E3
    void startMsg (int level,
		   const char* file, int line, const char* func,
		   const char* msg, const void* objPtr);

    //##ModelId=3DB9546402EA
    ~Tracer()
          { if (itsDo) endMsg(); }

  private:
    //##ModelId=3DB9546402EB
    Tracer(const Tracer&);
    //##ModelId=3DB9546402ED
    Tracer& operator= (const Tracer&);
    //##ModelId=3DB9546402EF
    void endMsg();

    //##ModelId=3DB9546402DE
    bool    itsDo;
    //##ModelId=3DB9546402DF
    int     itsLevel;
    //##ModelId=3DB9546402E1
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


// inline functions for converting scalars to strings
inline string num2str (int x)
{
  return Debug::ssprintf("%d",x);
}
inline string num2str (double x)
{
  return Debug::ssprintf("%f",x);
}



#endif
