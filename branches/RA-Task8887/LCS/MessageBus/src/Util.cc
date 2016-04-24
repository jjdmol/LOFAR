#include "lofar_config.h"
#include <Common/LofarTypes.h>

#include "Util.h"

#include <stdlib.h>

using namespace qpid::messaging;
using namespace std;

namespace {
  string getenv_str(const char *env) {
    char *val = getenv(env);
    return string(val ? val : "");
  }
}

namespace LOFAR {
  Duration TimeOutDuration(double secs)
  {
    if (secs > 0.0)
      return (Duration)(static_cast<uint64>(1000.0 * secs));

    return Duration::FOREVER;
  }

  std::string queue_prefix()
  {
    string lofarenv = getenv_str("LOFARENV");
    string queueprefix = getenv_str("QUEUE_PREFIX");

    if (lofarenv == "PRODUCTION") {
    } else if (lofarenv == "TEST") {
      queueprefix += "test.";
    } else {
      queueprefix += "devel.";
    }

    return queueprefix;
  }
} // namespace LOFAR

