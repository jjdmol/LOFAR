#include "lofar_config.h"

#include "LogSink.h"
#include <Common/LofarLogger.h>

#include <Common/SystemUtil.h>
#include <Common/StringUtil.h>
#include <string>
#include <sstream>

#ifdef HAVE_QPID
#include <qpid/messaging/Logger.h>

using namespace qpid::messaging;
#endif

namespace LOFAR {

#ifdef HAVE_QPID
  class QpidLogSink: public LoggerOutput {
  public:
    QpidLogSink() {
    }

    virtual void log(Level level, bool user, const char* file, int line, const char* function, const std::string& message) {
      (void)user;

      // shorten filename to reduce spam
      std::string filename = basename(file);

      // remove trailing \n from message (and other white space while we're at it)
      std::string trimmed_message = message;
      rtrim(trimmed_message, " \t\n");

      // construct log message
      std::stringstream msg;
      msg << function << "(): " << trimmed_message << " (" << filename << ":" << line << ")";

      // emit at the right level
      switch(level) {
        case trace:
        case debug:
          LOG_DEBUG(msg.str());
          break;

        case info:
        case notice:
          LOG_INFO(msg.str());
          break;

        case warning:
          LOG_WARN(msg.str());
          break;

        case error:
          LOG_ERROR(msg.str());
          break;

        case critical:
          LOG_FATAL(msg.str());
          break;
      }
    }
  };

  static QpidLogSink qpidLogSink;

  void qpidlogsink_init() {
    const char *argv[] = {
      // Name of program (dummy)
      "a.out",

      // QPID logger configuration parameters (see Logger::configure)

      // Disable all default QPID sinks
      "--log-to-stdout", "off",
      "--log-to-stderr", "off",

      // Trim the log messages
      "--log-time", "off"
    };
    int argc = sizeof argv / sizeof argv[0];

    Logger::configure(argc, argv);

    // Add the LOFAR logger as a sink for QPID
    Logger::setOutput(qpidLogSink);
  }

#else
  void qpidlogsink_init() {
    LOG_WARN("QPID support NOT enabled! Messaging will NOT be functional!");
  }
#endif

} // namespace LOFAR

