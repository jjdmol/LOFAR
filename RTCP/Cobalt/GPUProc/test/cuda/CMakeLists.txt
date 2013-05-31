# $Id$

include(LofarCTest)

# Add path to our CUDA kernels to the -I path for NVCC
cuda_include_directories(${PACKAGE_SOURCE_DIR}/share/gpu/kernels)

if(UNITTEST++_FOUND)
  lofar_add_test(tGPUWrapper tGPUWrapper.cc)
endif(UNITTEST++_FOUND)

lofar_add_test(tCudaRuntimeCompiler tCudaRuntimeCompiler.cc)
lofar_add_test(tcreateProgram tcreateProgram.cc)
lofar_add_test(tKernel tKernel.cc)
lofar_add_test(tStreamReadBuffer tStreamReadBuffer.cc)
lofar_add_test(tCorrelator tCorrelator.cc)

lofar_add_test(tFFT tFFT.cc)
lofar_add_test(tFIR_Filter tFIR_Filter.cc)
lofar_add_test(tDelayAndBandPass tDelayAndBandPass.cc)

# Trick tDelayAndBandPass into thinking that it can find the kernels it needs
# using the LOFARROOT environment variable
set_tests_properties(tCorrelator tDelayAndBandPass
  PROPERTIES ENVIRONMENT "LOFARROOT=${PACKAGE_SOURCE_DIR}")

# To compile .cu files, invoke CUDA's nvcc, but respect the host C/C++ compiler (variants) selection.
set(COMPILER_BINDIR_OPT --compiler-bindir ${CMAKE_CXX_COMPILER})

# tests built by nvcc
cuda_add_executable(t_cuda_complex t_cuda_complex.cu 
  OPTIONS ${COMPILER_BINDIR_OPT})

add_test(t_cuda_complex ${CMAKE_CURRENT_SOURCE_DIR}/t_cuda_complex.sh)
#lofar_add_test(tDelayCompensation tDelayCompensation.cc)
