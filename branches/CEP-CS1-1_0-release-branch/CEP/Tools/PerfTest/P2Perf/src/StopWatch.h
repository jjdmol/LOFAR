
#ifndef P2PERF_STOPWATCH_H__
#define P2PERF_STOPWATCH_H__

#include <sys/time.h>
#include <unistd.h>

/// StopWatch class
class StopWatch
{
 public:
  StopWatch();
  virtual ~StopWatch();

  void start();
  void stop();
  void pause();
  void resume();
  double elapsed() const;

  // also provide a global stopwatch
  // this can be used as a singleton
  static StopWatch* getGlobalStopWatch();

 private:
  struct timeval startTime;
  struct timeval stopTime;

  struct timeval pauseStartTime;
  long pausedSecs;
  long pauseduSecs;
  bool isPausing;

  static StopWatch* theirGlobalStopWatch;
};

#endif /* __STOPWATCH_H__ */
