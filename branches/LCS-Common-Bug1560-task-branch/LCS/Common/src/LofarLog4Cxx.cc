#include <Common/LofarLog4Cxx.h>

using namespace log4cxx;

// Create tracer logger
LoggerPtr theirTraceLoggerRef(Logger::getLogger("TRC"));

namespace LOFAR
{

#if 0
  void initLog4Cxx(string file)
  {
    // Add extension ".log_prop" to <file>, if necessary.
    if(file.find(".log4cxx_prop") == string::npos) file += ".log4cxx_prop";

    // Try to locate <file> using the configuration file locator.
    string cfgFile = ConfigLocator().locate(file);

    // Use the property configurator if we found a configuration file,
    // otherwise use the basic configurator.
    if(!cfgFile.empty()) {
      PropertyConfigurator::doConfigure(File(cfgFile));
    } else {
      helpers::LogLog::getLogLog()->
        warn(LOG4CPLUS_TEXT("Property configuration file \"") + file + 
             LOG4CPLUS_TEXT("\" not found."));
      helpers::LogLog::getLogLog()->
        warn(LOG4CPLUS_TEXT("Using basic logging configuration."));
      BasicConfigurator::doConfigure();
    }

  }
#endif
}
