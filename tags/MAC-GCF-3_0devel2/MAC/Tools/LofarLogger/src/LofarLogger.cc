//#  LofarLogger.cc: logger wrapper
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

#include "LofarLogger.h"
#include <stdarg.h>
#include <stdio.h>
#include <log4cplus/configurator.h>
#include <log4cplus/loggingmacros.h>

// namespaces
using namespace std;

// static member initialization
LofarLogger LofarLogger::_sLofarLogger;

/**
 * LofarLogger constructor
 */
LofarLogger::LofarLogger() :
    _loggerMap()
{
    log4cplus::PropertyConfigurator::doConfigure("log4cplus.properties");
}

LofarLogger::LofarLogger(const LofarLogger& rhs) :
    _loggerMap(rhs._loggerMap)
{
}

/**
 * ~LofarLogger destructor
 */
LofarLogger::~LofarLogger()
{
}

/**
 * getInstance - returns the static instance of the LofarLogger
 */
LofarLogger LofarLogger::getInstance()
{
  return _sLofarLogger;
}

/**
 * trace - logs TRACE level messages
 */
void LofarLogger::trace(const string& logger,const string& logEvent,const char* file, int line)
{
    log4cplus::Logger log4cplusLogger=getLogger(logger);

    if(log4cplusLogger.isEnabledFor(log4cplus::TRACE_LOG_LEVEL))
    {
        log4cplus::tostringstream _log4cplus_buf;
        _log4cplus_buf << logEvent;
        const char* shortFile = strrchr(file, '/');
        if (shortFile == 0) shortFile = file; else shortFile++;
        log4cplusLogger.forcedLog(log4cplus::TRACE_LOG_LEVEL,_log4cplus_buf.str(),shortFile,line);
    }
}

/**
 * debug - logs DEBUG level messages
 */
void LofarLogger::debug(const string& logger,const string& logEvent,const char* file, int line)
{
    log4cplus::Logger log4cplusLogger=getLogger(logger);

    if(log4cplusLogger.isEnabledFor(log4cplus::DEBUG_LOG_LEVEL))
    {
        log4cplus::tostringstream _log4cplus_buf;
        _log4cplus_buf << logEvent;
        const char* shortFile = strrchr(file, '/');
        if (shortFile == 0) shortFile = file; else shortFile++;
        log4cplusLogger.forcedLog(log4cplus::DEBUG_LOG_LEVEL, _log4cplus_buf.str(), shortFile,line);
    }
}

/**
 * info - logs INFO level messages
 */
void LofarLogger::info(const string& logger,const string& logEvent,const char* file, int line)
{
    log4cplus::Logger log4cplusLogger=getLogger(logger);

    if(log4cplusLogger.isEnabledFor(log4cplus::INFO_LOG_LEVEL))
    {
        log4cplus::tostringstream _log4cplus_buf;
        _log4cplus_buf << logEvent;
        const char* shortFile = strrchr(file, '/');
        if (shortFile == 0) shortFile = file; else shortFile++;
        log4cplusLogger.forcedLog(log4cplus::INFO_LOG_LEVEL, _log4cplus_buf.str(), shortFile,line);
    }
}

/**
 * warn - logs WARN level messages
 */
void LofarLogger::warn(const string& logger,const string& logEvent,const char* file, int line)
{
    log4cplus::Logger log4cplusLogger=getLogger(logger);

    if(log4cplusLogger.isEnabledFor(log4cplus::WARN_LOG_LEVEL))
    {
        log4cplus::tostringstream _log4cplus_buf;
        _log4cplus_buf << logEvent;
        const char* shortFile = strrchr(file, '/');
        if (shortFile == 0) shortFile = file; else shortFile++;
        log4cplusLogger.forcedLog(log4cplus::WARN_LOG_LEVEL, _log4cplus_buf.str(), shortFile,line);
    }
}

/**
 * error - logs ERROR level messages
 */
void LofarLogger::error(const string& logger,const string& logEvent,const char* file, int line)
{
    log4cplus::Logger log4cplusLogger=getLogger(logger);

    if(log4cplusLogger.isEnabledFor(log4cplus::ERROR_LOG_LEVEL))
    {
        log4cplus::tostringstream _log4cplus_buf;
        _log4cplus_buf << logEvent;
        const char* shortFile = strrchr(file, '/');
        if (shortFile == 0) shortFile = file; else shortFile++;
        log4cplusLogger.forcedLog(log4cplus::ERROR_LOG_LEVEL, _log4cplus_buf.str(), shortFile,line);
    }
}

/**
 * fatal - logs FATAL level messages
 */
void LofarLogger::fatal(const string& logger,const string& logEvent,const char* file, int line)
{
    log4cplus::Logger log4cplusLogger=getLogger(logger);

    if(log4cplusLogger.isEnabledFor(log4cplus::FATAL_LOG_LEVEL)) 
    {
        log4cplus::tostringstream _log4cplus_buf;
        _log4cplus_buf << logEvent;
        const char* shortFile = strrchr(file, '/');
        if (shortFile == 0) shortFile = file; else shortFile++;
        log4cplusLogger.forcedLog(log4cplus::FATAL_LOG_LEVEL, _log4cplus_buf.str(), shortFile,line);
    }
}

/**
 * formatString - creates a formatted string
 */
string LofarLogger::formatString(char* format,...)
{
    va_list args;
    char buffer[1000];
    va_start(args,format);
    vsnprintf(buffer,1000,format,args);
    return string(buffer);
}

/**
 * getLogger - finds a logger in the logger map or creates a new logger
 */
log4cplus::Logger& LofarLogger::getLogger(const string& logger)
{
    TLoggerMap::iterator loggerMapIterator=_loggerMap.find(logger);
    if(loggerMapIterator==_loggerMap.end())
    {
        _loggerMap.insert(TLoggerMap::value_type(logger,log4cplus::Logger::getInstance(logger)));
    }
    loggerMapIterator=_loggerMap.find(logger);
    return (loggerMapIterator->second);
}
