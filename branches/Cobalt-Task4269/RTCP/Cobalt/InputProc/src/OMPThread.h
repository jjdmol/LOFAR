#ifndef __OMPTHREAD__
#define __OMPTHREAD__

#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <Common/LofarLogger.h>
#include <Common/SystemCallException.h>

namespace LOFAR {

/*
 * Represents an OpenMP thread. To use,
 * call start() and stop() at the beginning and end
 * of an OpenMP thread. The kill() command can then
 * be used by another thread to kill the OpenMP thread.
 *
 * The thread is killed by sending SIGHUP to it until
 * stop() is called by the thread.
 *
 * To be able to use this class properly, please call
 * OMPThread::init() to clear the SIGHUP handler.
 *
 * Note: do NOT use this on threads which continue operating
 * through omp 'nowait' parallelism, due to race conditions.
 */
class OMPThread {
public:
  OMPThread(): id(0), stopped(false) {}

  // Register the current thread as killable
  void start() {
    id = pthread_self();
  }

  // Unregister the current thread
  void stop() {
    id = 0;
    stopped = true;
  }

  // Kill the registered thread. If no thread is registered,
  // kill() will wait.
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

  static void init() {
    signal(SIGHUP, sighandler);
    siginterrupt(SIGHUP, 1);
  }

private:
  volatile pthread_t id;
  volatile bool stopped;

  static void sighandler(int) {
    /* no-op. We use SIGHUP only
     * to interrupt system calls.
     */
  }
};

}

#endif

