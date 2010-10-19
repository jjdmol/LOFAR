
#ifndef __STOPWATCH_H__
#define __STOPWATCH_H__

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
  double elapsed();

 private:
  struct timeval startTime;
  struct timeval stopTime;
};

#endif /* __STOPWATCH_H__ */
