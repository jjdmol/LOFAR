#include <Common/lofar_iostream.h>
#include <string.h>

#include "P2Perf/StopWatch.h"

StopWatch* StopWatch::theirGlobalStopWatch = 0;

StopWatch::StopWatch() : pausedSecs(0), pauseduSecs(0), isPausing (false)
{
  memset(&startTime, 0, sizeof(struct timeval));
  memset(&stopTime,  0, sizeof(struct timeval));
  memset(&pauseStartTime,  0, sizeof(struct timeval));
}

StopWatch::~StopWatch()
{
}

void StopWatch::start()
{
  memset(&pauseStartTime,  0, sizeof(struct timeval));
  timerclear(&stopTime);
  pausedSecs = 0;
  pauseduSecs = 0;
  isPausing = false;
  gettimeofday(&startTime, NULL);
}

void StopWatch::stop()
{
  gettimeofday(&stopTime, NULL);
  if (isPausing)
  {
    // if we were pausing overwrite the stoptime with the time the pause() was called
    memcpy(&stopTime, &pauseStartTime, sizeof(struct timeval));
    isPausing = false;
  }
}

void StopWatch::pause()
{
  gettimeofday(&pauseStartTime, NULL);
  isPausing = true;
}

void StopWatch::resume()
{
  if (isPausing==true)
  {
    isPausing = false;
    pausedSecs -= pauseStartTime.tv_sec;
    pauseduSecs -= pauseStartTime.tv_usec;

    struct timeval pauseStopTime;
    gettimeofday(&pauseStopTime, NULL);
    // the following instructions are executed while the time 
    // is running so this is probably not the best solution
    pausedSecs += pauseStopTime.tv_sec;
    pauseduSecs += pauseStopTime.tv_usec;
  }    
}

double StopWatch::elapsed() const
{
  long secsDiff;
  long usecsDiff;
  secsDiff = stopTime.tv_sec  - startTime.tv_sec - pausedSecs;
  usecsDiff = stopTime.tv_usec - startTime.tv_usec - pauseduSecs;
  double secsElapsed(0);

  if (   timerisset(&startTime)
      && timerisset(&stopTime)
      && timercmp(&startTime, &stopTime, <=))
  {
    secsElapsed = (double) secsDiff + ((double)usecsDiff / 1000000.0);
  }
	
  return secsElapsed;
}

//static
StopWatch * StopWatch::getGlobalStopWatch()
{
  if (theirGlobalStopWatch == 0)
    theirGlobalStopWatch = new StopWatch();
  return theirGlobalStopWatch;    
}
