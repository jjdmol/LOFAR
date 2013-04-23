#pragma once

#include "Module.h"
#include "Error.h"
#include <boost/noncopyable.hpp>
#include <cuda.h>

struct Module::Impl : boost::noncopyable
{
  Impl(const char *fname)
  {
    checkCudaCall(cuModuleLoad(&_module, fname));
  }

  Impl(const void *image)
  {
    checkCudaCall(cuModuleLoadData(&_module, image));
  }

  Impl(const void *image, unsigned numOptions, 
       CUjit_option *options, void **optionValues)
  {
    checkCudaCall(cuModuleLoadDataEx(&_module, image, numOptions, 
                                     options, optionValues));
  }

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

