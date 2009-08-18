//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/lofar_iostream.h>
#include <string.h>

#include "StopWatch.h"

StopWatch::StopWatch()
{
  memset(&startTime, 0, sizeof(struct timeval));
  memset(&stopTime,  0, sizeof(struct timeval));
}

StopWatch::~StopWatch()
{
}

void StopWatch::start()
{
  timerclear(&stopTime);
  gettimeofday(&startTime, NULL);

  //cout << "start: " << startTime.tv_sec << ":" << startTime.tv_usec << endl;
}

void StopWatch::stop()
{
  gettimeofday(&stopTime, NULL);

  //cout << "stop: " << stopTime.tv_sec << ":" << stopTime.tv_usec << endl;
}

double StopWatch::elapsed()
{
  long   secsDiff  = stopTime.tv_sec  - startTime.tv_sec;
  long   usecsDiff = stopTime.tv_usec - startTime.tv_usec;
  double usecsElapsed = 0.0;

  if (   timerisset(&startTime)
      && timerisset(&stopTime)
      && timercmp(&startTime, &stopTime, <=))
  {
    usecsElapsed = (double)secsDiff + ((double)usecsDiff / 1000000.0);
  }
	
  return usecsElapsed;
}
