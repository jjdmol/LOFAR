#include "lofar_config.h"

#include "Util.h"

#include <stdlib.h>

using namespace qpid::messaging;

namespace LOFAR {
  Duration TimeOutDuration(double secs)
  {
    if (secs > 0.0)
      return (Duration)(static_cast<uint64_t>(1000.0 * secs));

    return Duration::FOREVER;
  }

  std::string queue_prefix()
  {
    char *prefix = getenv("QUEUE_PREFIX");

    if (!prefix)
      return "";

    return prefix;
  }
} // namespace LOFAR

