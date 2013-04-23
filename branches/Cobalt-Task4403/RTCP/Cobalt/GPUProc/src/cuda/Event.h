#pragma once

#include <cuda.h>  // for CU_EVENT_DEFAULT
#include <boost/shared_ptr.hpp>

class Event
{
public:
  Event(unsigned flags = CU_EVENT_DEFAULT);
  float elapsedTime() const;
private:
  friend class Stream;
  class Impl;
  boost::shared_ptr<Impl> _impl;
};

