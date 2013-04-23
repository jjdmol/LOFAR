#pragma once

#include <string>
#include <vector>
#include <cuda.h>  // unfortunately necessary for CUjit_option
#include <boost/shared_ptr.hpp>

class Module
{
public:
  Module(const std::string &file_name);
  Module(const void *data);
  Module(const void *data, 
         std::vector<CUjit_option>& options, 
         std::vector<void*> optionValues);
  CUmodule operator()() const;
private:
  friend class Function;
  struct Impl;
  boost::shared_ptr<Impl> _impl;
};

