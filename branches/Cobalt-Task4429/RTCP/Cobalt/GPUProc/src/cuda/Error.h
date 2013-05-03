#pragma once

#include <exception>
#include <cuda.h>  // for CUresult

class Error : public std::exception 
{
public:
  Error(CUresult result);
  virtual const char *what() const throw();
private:
  CUresult _result;
};

void checkCudaCall(CUresult result);
