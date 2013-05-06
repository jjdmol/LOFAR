#pragma once

#include <string>
#include <cuda.h>  // for CUfunction_attribute, CUsharedconfig

class Module;

class Function
{
public:
  Function(const std::string &name, const Module &module);
  int getAttribute(CUfunction_attribute attribute) const;
  void setSharedMemConfig(CUsharedconfig config);
private:
  friend class Stream;
  CUfunction _function;
};

