#include "Module.h"
#include "Error.h"
#include <boost/noncopyable.hpp>
#include <cuda.h>

struct Module::Impl : boost::noncopyable
{
  //# Module Management
  Impl(const char *fname)
  {
    checkCudaCall(cuModuleLoad(&_module, fname));
  }

  //# Module Management
  Impl(const void *image)
  {
    checkCudaCall(cuModuleLoadData(&_module, image));
  }

  //# Module Management
  Impl(const void *image, unsigned numOptions, 
       CUjit_option *options, void **optionValues)
  {
    checkCudaCall(cuModuleLoadDataEx(&_module, image, numOptions, 
                                     options, optionValues));
  }

  //# Module Management
  ~Impl()
  {
    checkCudaCall(cuModuleUnload(_module));
  }

  CUmodule _module;

};


Module::Module(const std::string &file_name) :
  _impl(new Impl(file_name.c_str()))
{
}

Module::Module(const void *data) :
  _impl(new Impl(data))
{
}

Module::Module(const void *data, 
               std::vector<CUjit_option>& options, 
               std::vector<void*> optionValues) :
  _impl(new Impl(data, options.size(), &options[0], &optionValues[0]))
{
}

CUmodule Module::operator()() const
{
  return _impl->_module;
}

CUfunction Module::getKernelEntryPoint(const char* functionName)
{
    CUfunction   hKernel; 
    checkCudaCall(cuModuleGetFunction(&hKernel,
                  _impl->_module,
                  functionName));
    return hKernel;
}