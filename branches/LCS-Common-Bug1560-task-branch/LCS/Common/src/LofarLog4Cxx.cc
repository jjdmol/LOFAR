#include <Common/LofarLog4Cxx.h>
#include <Common/LofarLocators.h>
#include <Common/SystemUtil.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/ndc.h>

using namespace log4cxx;

namespace LOFAR
{
  namespace
  {
    // Create an NDC (nested diagnostic context) with the text
    // "application@node" and push it onto the NDC stack.
    void initNDC(void)
    {
      string loggerId(basename(getExecutablePath()) + "@" + myHostname(false));
      log4cxx::NDC::push(loggerId);
    }

    // Function that is used when the TRACE levels are NOT compiled out. 
    //
    // NOT YET IMPLEMENTED!! The implementation of initTraceModule() in
    // LofarLog4Cplus.cc could be used as a starting point.
    void initTraceModule (void)
    {
#ifdef ENABLE_TRACER
#endif
    }
  } // namespace


  // Create the tracelogger
  log4cxx::LoggerPtr theirTraceLoggerRef(log4cxx::Logger::getLogger("TRC"));

  // Initialize Log4cxx. 
  void initLog4Cxx(string propFile, const string& logFile, 
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
      PropertyConfigurator::configure(locatedPropFile);
    } else {
      helpers::LogLog::warn(LOG4CXX_STR("Property configuration file \"") + 
                            propFile +
                            LOG4CXX_STR("\" not found."));
      helpers::LogLog::warn(LOG4CXX_STR("Using basic logging configuration."));
      BasicConfigurator::configure();
    }
  }


#ifdef USE_THREADS
  // Initialize Log4cxx with a watchdog thread for the configuration file.
  void initLog4CxxAndWatch(string propFile, unsigned int watchInterval)
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
    PropertyConfigurator::configureAndWatch(locatedPropFile, watchInterval);
  }
#endif

} // namespace LOFAR
