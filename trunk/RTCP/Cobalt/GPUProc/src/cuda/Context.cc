#include "Context.h"
#include "Device.h"
#include "Error.h"
#include <boost/noncopyable.hpp>
#include <cuda.h>

struct Context::Impl : boost::noncopyable
{
  //# Context Management
  Impl(unsigned flags, CUdevice device)
  {
    checkCudaCall(cuCtxCreate(&_context, flags, device));
  }

  //# Context Management
  ~Impl()
  {
    checkCudaCall(cuCtxDestroy(_context));
  }

  //# Context Management
  void setCurrent() const
  {
    checkCudaCall(cuCtxSetCurrent(_context));
  }

  CUcontext _context;
};


Context::Context(const Device& device, unsigned flags) :
  _impl(new Impl(flags, device._device))
{
}

void Context::setCurrent() const
{
  _impl->setCurrent();
}

