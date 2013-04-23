#include "Event.h"
#include "Error.h"
#include <boost/noncopyable.hpp>
#include <cuda.h>

struct Event::Impl : boost::noncopyable
{
  Impl(unsigned flags) : _flags(flags)
  {
    checkCudaCall(cuEventCreate(&_event, flags));
  }

  ~Impl()
  {
    checkCudaCall(cuEventDestroy(_event));
  }

  float elapsedTime(const Event& now) const
  {
    float milliseconds;
    checkCudaCall(cuEventElapsedTime(&milliseconds, _event, now._impl->_event));
    return milliseconds;
  }

  CUevent _event;
  unsigned _flags;
};


Event::Event(unsigned flags) :
  _impl(new Impl(flags))
{
}

float Event::elapsedTime() const
{
  return _impl->elapsedTime(Event());
}
