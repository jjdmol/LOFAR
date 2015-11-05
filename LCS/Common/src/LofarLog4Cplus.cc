#include <Common/LofarLog4Cplus.h>
#include <Common/LofarLocators.h>
#include <Common/SystemUtil.h>
#include <log4cplus/configurator.h>
#include <log4cplus/helpers/loglog.h>
#include <cstdlib>

using namespace log4cplus;

namespace LOFAR
{
  namespace
  {
    const string gExecutablePath = getExecutablePath();

    // Define the eight trace log levels
    const LogLevel TRACE1_LOG_LEVEL = 1;
    const LogLevel TRACE2_LOG_LEVEL = 2;
    const LogLevel TRACE3_LOG_LEVEL = 3;
    const LogLevel TRACE4_LOG_LEVEL = 4;
    const LogLevel TRACE5_LOG_LEVEL = 5;
    const LogLevel TRACE6_LOG_LEVEL = 6;
    const LogLevel TRACE7_LOG_LEVEL = 7;
    const LogLevel TRACE8_LOG_LEVEL = 8;

    const tstring _TRACE1_STRING = LOG4CPLUS_TEXT("TRACE1");
    const tstring _TRACE2_STRING = LOG4CPLUS_TEXT("TRACE2");
    const tstring _TRACE3_STRING = LOG4CPLUS_TEXT("TRACE3");
    const tstring _TRACE4_STRING = LOG4CPLUS_TEXT("TRACE4");
    const tstring _TRACE5_STRING = LOG4CPLUS_TEXT("TRACE5");
    const tstring _TRACE6_STRING = LOG4CPLUS_TEXT("TRACE6");
    const tstring _TRACE7_STRING = LOG4CPLUS_TEXT("TRACE7");
    const tstring _TRACE8_STRING = LOG4CPLUS_TEXT("TRACE8");

    // Convert trace log level to string
    tstring traceLevel2String(LogLevel ll) {
      switch (ll) {
      case TRACE1_LOG_LEVEL: return _TRACE1_STRING;
      case TRACE2_LOG_LEVEL: return _TRACE2_STRING;
      case TRACE3_LOG_LEVEL: return _TRACE3_STRING;
      case TRACE4_LOG_LEVEL: return _TRACE4_STRING;
      case TRACE5_LOG_LEVEL: return _TRACE5_STRING;
      case TRACE6_LOG_LEVEL: return _TRACE6_STRING;
      case TRACE7_LOG_LEVEL: return _TRACE7_STRING;
      case TRACE8_LOG_LEVEL: return _TRACE8_STRING;
      }
      return tstring();  // not found
    }

    // Convert string to trace log level
    LogLevel string2TraceLevel (const tstring& lname) {
      if (lname == _TRACE1_STRING) return TRACE1_LOG_LEVEL;
      if (lname == _TRACE2_STRING) return TRACE2_LOG_LEVEL;
      if (lname == _TRACE3_STRING) return TRACE3_LOG_LEVEL;
      if (lname == _TRACE4_STRING) return TRACE4_LOG_LEVEL;
      if (lname == _TRACE5_STRING) return TRACE5_LOG_LEVEL;
      if (lname == _TRACE6_STRING) return TRACE6_LOG_LEVEL;
      if (lname == _TRACE7_STRING) return TRACE7_LOG_LEVEL;
      if (lname == _TRACE8_STRING) return TRACE8_LOG_LEVEL;
      return NOT_SET_LOG_LEVEL;   // not found
    }

    // Function that is used when the TRACE levels are NOT compiled out. It
    // registers the TRACEn management routines at the Log4Cplus
    // LogLevelManager and sets up the global trace-logger named "TRC", with
    // the additivity set to false.  Attached to the trace-logger is one
    // Appender that logs to stderr.
    void initTraceModule (void)
    {
#ifdef ENABLE_TRACER
      // Register our own loglevels
      getLogLevelManager().pushToStringMethod(traceLevel2String);
      getLogLevelManager().pushFromStringMethod(string2TraceLevel);
      
      // Setup a property object to initialise the TRACE Logger
      helpers::Properties  prop;
      prop.setProperty("log4cplus.logger.TRC", "DEBUG, STDERR");
      prop.setProperty("log4cplus.additivity.TRC", "false");
      prop.setProperty("log4cplus.appender.STDERR", 
                       "log4cplus::ConsoleAppender");
      prop.setProperty("log4cplus.appender.STDERR.logToStdErr", "true");
      prop.setProperty("log4cplus.appender.STDERR.ImmediateFlush", "true");
      prop.setProperty("log4cplus.appender.STDERR.layout",
                       "log4cplus::PatternLayout");
      prop.setProperty("log4cplus.appender.STDERR.layout.ConversionPattern",
                       "%D{%y%m%d %H%M%S,%q} [%i] %-6p %c{3} [%b:%L] - %m%n");
      PropertyConfigurator(prop).configure();
      Logger::getInstance("TRC").forcedLog(0, "TRACE module activated");
#endif
    }
  } // namespace


  // Create an NDC (nested diagnostic context) with the text
  // "application@node" and push it onto the NDC stack.
  void initNDC(void)
  {
    string loggerId(basename(gExecutablePath) + "@" + myHostname(false));
    log4cplus::getNDC().push(loggerId);
  }

  // Destroy the NDC (nested diagnostic context) when we're done
  // with this thread.
  void destroyNDC(void)
  {
    log4cplus::getNDC().remove();
  }

  // Create the tracelogger
  LOFAR::LoggerReference theirTraceLoggerRef("TRC");


  // Initialize Log4cplus. 
  void initLog4Cplus(string propFile, const string& logFile, 
                     const string& envVar)
  {
    // Assign the contents of \a logFile to environment variable \a envVar.
    if(!envVar.empty()) {
      setenv(envVar.c_str(), logFile.c_str(), true);  // set overwrite 'true'
    }

    // Initialize NDC (nested diagnostic context).
    initNDC();

    // Initialize tracing module.
    initTraceModule();

    // Add extension ".log_prop" to \a propFile, if necessary.
    if(propFile.find(".log_prop") == string::npos) {
      propFile += ".log_prop";
    }

    // Try to locate \a propFile using the configuration file locator.
    string locatedPropFile = ConfigLocator().locate(propFile);

    // Use the property configurator if we found a configuration file,
    // otherwise use the basic configurator.
    if(!locatedPropFile.empty()) {
      PropertyConfigurator::doConfigure(locatedPropFile);
    } else {
      helpers::LogLog::getLogLog()->
        warn(LOG4CPLUS_TEXT("Property configuration file \"") + propFile +
             LOG4CPLUS_TEXT("\" not found."));
      helpers::LogLog::getLogLog()->
        warn(LOG4CPLUS_TEXT("Using basic logging configuration."));
      BasicConfigurator::doConfigure();
    }
  }


#ifdef USE_THREADS
  // Initialize Log4cplus with a watchdog thread for the configuration file.
  void initLog4CplusAndWatch(string propFile, unsigned int watchInterval)
  {
    // Initialize NDC (nested diagnostic context).
    initNDC();

    // Initialize tracing module.
    initTraceModule();

    // Add extension ".log_prop" to \a propFile, if necessary.
    if(propFile.find(".log_prop") == string::npos) {
      propFile += ".log_prop";
    }

    // Try to locate \a propFile using the configuration file locator.
    string locatedPropFile = ConfigLocator().locate(propFile);

    // Use the configure-and-watch-thread property configurator, even if we
    // didn't find a properties file. It might be created later and will then
    // be picked up by the watchdog.
    ConfigureAndWatchThread(locatedPropFile, watchInterval);
  }
#endif

} // namespace LOFAR
