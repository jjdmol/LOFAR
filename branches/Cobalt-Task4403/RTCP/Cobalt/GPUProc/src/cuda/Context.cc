#include "Context.h"
#include "Device.h"
#include "Error.h"
#include <boost/noncopyable.hpp>
#include <cuda.h>

struct Context::Impl : boost::noncopyable
{
  Impl(unsigned flags, CUdevice device)
  {
    checkCudaCall(cuCtxCreate(&_context, flags, device));
  }

  ~Impl()
  {
    checkCudaCall(cuCtxDestroy(_context));
  }

  void setCurrent() const
  {
    checkCudaCall(cuCtxSetCurrent(_context));
  }

  CUcontext _context;
};


Context::Context(const Device& device, unsigned flags) :
  _impl(new Impl(flags, device.get()))
{
}

void Context::setCurrent() const
{
  _impl->setCurrent();
}

