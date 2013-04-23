#include "Function.h"
#include "Error.h"
#include "Module.h"
#include <cuda.h>

#include "Module.cc"

Function::Function(const std::string &name, const Module &module)
{
  checkCudaCall(cuModuleGetFunction(&_function, module._impl->_module, name.c_str()));
}

int Function::getAttribute(CUfunction_attribute attribute) const
{
  int value;
  checkCudaCall(cuFuncGetAttribute(&value, attribute, _function));
  return value;
}

void Function::setSharedMemConfig(CUsharedconfig config)
{
  checkCudaCall(cuFuncSetSharedMemConfig(_function, config));
}
