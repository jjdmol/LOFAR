//
// MATLAB Compiler: 2.0.1
// Date: Tue May 09 15:44:02 2000
// Arguments: "-p" "-d" "." "-T" "codegen" ".\beam.m" ".\spectralfft.m" 
//
#ifndef __spectralfft_hpp
#define __spectralfft_hpp 1

#include "matlab.hpp"

extern mwArray spectralfft(mwArray sig = mwArray::DIN,
                           mwArray len = mwArray::DIN);
#ifdef __cplusplus
extern "C"
#endif
void mlxSpectralfft(int nlhs, mxArray * plhs[], int nrhs, mxArray * prhs[]);

#endif
