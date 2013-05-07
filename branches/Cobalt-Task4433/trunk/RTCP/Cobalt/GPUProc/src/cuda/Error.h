#pragma once

#include <exception>
#include <cuda.h>  // for CUresult
#include <driver_types.h>

class Error : public std::exception 
{
public:
  Error(CUresult result);
  Error(cudaError error);
  virtual const char *what() const throw();
private:
  enum type
  {
    resultType = 0,
    errorType = 1
  };
  type _type;
  CUresult _result;
  cudaError _error;
};

void checkCudaCall(CUresult result);
void checkCudaCall(cudaError error);

