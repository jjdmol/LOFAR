#include "lofar_config.h"

#include <MessageBus/MessageBus.h>
#include "LogSink.h"

namespace LOFAR {
  namespace MessageBus {
    void init() {
      qpidlogsink_init();
    }
  }
} // namespace LOFAR

