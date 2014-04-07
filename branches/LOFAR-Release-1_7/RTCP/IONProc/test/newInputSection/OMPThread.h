#ifndef __OMPTHREAD__
#define __OMPTHREAD__

#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <Common/LofarLogger.h>
#include <Common/SystemCallException.h>

namespace LOFAR {

class OMPThread {
public:
  OMPThread(): id(0), stopped(false) {}

  void start() {
    id = pthread_self();
  }

  void stop() {
    id = 0;
    stopped = true;
  }

  void kill() {
    while (!stopped) {
      // interrupt blocking system calls (most notably, read())
      // note that the thread will stick around until the end
      // of pragma parallel, so the thread id is always valid
      // once it has been set.
      pthread_t oldid = id;

      if (oldid > 0)
        if (pthread_kill(oldid, SIGHUP) < 0)
          throw SystemCallException("pthread_kill", errno, THROW_ARGS);

      // sleep for 100ms - do NOT let us get killed here,
      // because we're maintaining integrity
      const struct timespec ts = { 1, 200*1000 };
      while (nanosleep( &ts, NULL ) == -1 && errno == EINTR)
        ;
    }
  }

  class ScopedRun {
  public:
    ScopedRun( OMPThread &thread ): thread(thread) {
      thread.start();
    }

    ~ScopedRun() {
      thread.stop();
    }

  private:
    OMPThread &thread;
  };

private:
  volatile pthread_t id;
  volatile bool stopped;
};

}

#endif

