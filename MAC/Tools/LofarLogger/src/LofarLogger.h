//#  LofarLogger.h: logger wrapper
//#
//#  Copyright (C) 2002-2003
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

#ifndef LOFAR_LOGGER_H
#define LOFAR_LOGGER_H

#include <Common/lofar_string.h>
#include <Common/lofar_map.h>
#include <log4cplus/logger.h>
#include "LofarLoggerNames.h"

// forward declaration
// define convenience macros
#ifndef NOLOG
#define LOFAR_LOG_TRACE_SCOPE(logger,logEvent) \
    LofarScopeTraceLogger _scopeLogger(logger,LofarLogger::formatString logEvent ,__FILE__,__LINE__);
#define LOFAR_LOG_TRACE(logger,logEvent) \
    LofarLogger::getInstance().trace(logger,LofarLogger::formatString logEvent ,__FILE__,__LINE__);
#define LOFAR_LOG_DEBUG(logger,logEvent) \
    LofarLogger::getInstance().debug(logger,LofarLogger::formatString logEvent ,__FILE__,__LINE__);
#define LOFAR_LOG_INFO(logger,logEvent)  \
    LofarLogger::getInstance().info(logger,LofarLogger::formatString logEvent ,__FILE__,__LINE__);
#define LOFAR_LOG_WARN(logger,logEvent)  \
    LofarLogger::getInstance().warn(logger,LofarLogger::formatString logEvent ,__FILE__,__LINE__);
#define LOFAR_LOG_ERROR(logger,logEvent) \
    LofarLogger::getInstance().error(logger,LofarLogger::formatString logEvent ,__FILE__,__LINE__);
#define LOFAR_LOG_FATAL(logger,logEvent) \
    LofarLogger::getInstance().fatal(logger,LofarLogger::formatString logEvent ,__FILE__,__LINE__);
#else    
#define LOFAR_LOG_TRACE_SCOPE(logger,logEvent) \
    {};
#define LOFAR_LOG_TRACE(logger,logEvent) \
    {};
#define LOFAR_LOG_DEBUG(logger,logEvent) \
    {};
#define LOFAR_LOG_INFO(logger,logEvent)  \
    {};
#define LOFAR_LOG_WARN(logger,logEvent)  \
    {};
#define LOFAR_LOG_ERROR(logger,logEvent) \
    {};
#define LOFAR_LOG_FATAL(logger,logEvent) \
    {};
#endif
/**
 *
 * The LofarLogger class wraps the log4cplus::Logger class
 *
 * Usage:
 *
 *    #define EXAMPLE_LOGGER_NAME "ASTRON.LOFAR.MAC.ExampleLoggerName"
 *
 *    LOFAR_LOG_TRACE(EXAMPLE_LOGGER_NAME,("example %d level message %d","trace",15))
 *    LOFAR_LOG_FATAL(EXAMPLE_LOGGER_NAME,("example fatal level message"))
 *
 * NOTE: the second argument of the macro HAS to be surrounded with brackets.
 */
class LofarLogger
{
    public:

        /**
        * destructor
        */
        virtual ~LofarLogger();

        static LofarLogger getInstance();

        /**
        * logging methods4
        */
        void trace(const string& logger,const string& logEvent,const char* file=NULL,int line=-1);
        void debug(const string& logger,const string& logEvent,const char* file=NULL,int line=-1);
        void info(const string& logger,const string& logEvent,const char* file=NULL,int line=-1);
        void warn(const string& logger,const string& logEvent,const char* file=NULL,int line=-1);
        void error(const string& logger,const string& logEvent,const char* file=NULL,int line=-1);
        void fatal(const string& logger,const string& logEvent,const char* file=NULL,int line=-1);

        log4cplus::Logger& getLogger(const string& logger);
        static string formatString(char* format,...);

    protected:
        /**4
        * constructor. Protected because public construction is not allowed
        */
        LofarLogger();
        LofarLogger(const LofarLogger&);

    private:

        /**
        * Don't allow copying of the LofarLogger object.
        */
        LofarLogger& operator=(const LofarLogger&);

        typedef std::map<const std::string,log4cplus::Logger> TLoggerMap;

        /**
        * keep the loggers in a map for re-use
        */
        TLoggerMap         _loggerMap;
        static LofarLogger _sLofarLogger;
};

/**
 *
 * The LofarScopeTraceLogger class logs trace messages in its constructor and destructor
 *
 */
class LofarScopeTraceLogger
{
    public:
       explicit LofarScopeTraceLogger(const std::string& logger, const std::string& logEvent,const char* file=NULL,int line=-1) :
                    _traceLogger(LofarLogger::getInstance().getLogger(logger),logEvent,file,line)
                {
                };
                ~LofarScopeTraceLogger()
                {
                };

    protected:
                /**
                * constructor. Protected because public construction is not allowed
                */
                LofarScopeTraceLogger();
                LofarScopeTraceLogger(const LofarScopeTraceLogger&);
                LofarScopeTraceLogger& operator=(const LofarScopeTraceLogger&);

    private:
                log4cplus::TraceLogger _traceLogger;
};

#endif
