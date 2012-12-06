#ifndef TIMESYNC
#define TIMESYNC

#include <Common/Thread/Mutex.h>
#include <Common/Thread/Condition.h>

namespace LOFAR {

class TimeSync {
public:
  TimeSync();

  // set to `val'
  void set( int64 val );

  // wait for the value to be at least `val'

  // wait for the value to be at least `val' (and
  // return true), or until there is no more data
  // or a timeout (return false).
  bool wait( int64 val );
  bool wait( int64 val, struct timespec &timeout );

  // signal no more data
  void noMoreData();

private:
  bool stop;
  int64 timestamp;
  int64 waitFor;

  Mutex mutex;
  Condition cond;
};

TimeSync::TimeSync()
:
  stop(false),
  timestamp(0),
  waitFor(0)
{
}

void TimeSync::set( int64 val ) {
  ScopedLock sl(mutex);

  timestamp = val;

  if (waitFor != 0 && timestamp > waitFor)
    cond.signal();
}

bool TimeSync::wait( int64 val ) {
  ScopedLock sl(mutex);

  waitFor = val;

  while (timestamp <= val && !stop)
    cond.wait(mutex);

  waitFor = 0;

  return timestamp <= val;
}

bool TimeSync::wait( int64 val, struct timespec &timeout ) {
  ScopedLock sl(mutex);

  waitFor = val;

  while (timestamp <= val && !stop)
    if( !cond.wait(mutex, timeout) )
      break;

  waitFor = 0;

  return timestamp <= val;
}

void TimeSync::noMoreData() {
  ScopedLock sl(mutex);

  stop = true;
  cond.signal();
}

}

#endif
