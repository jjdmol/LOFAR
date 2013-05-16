# $Id$

include(LofarCTest)

lofar_add_test(tCudaRuntimeCompiler tCudaRuntimeCompiler.cc)
lofar_add_test(tcreateProgram tcreateProgram.cc)
lofar_add_test(tKernel tKernel.cc)
lofar_add_test(tStreamReadBuffer tStreamReadBuffer.cc)

lofar_add_test(tFIR_Filter tFIR_Filter.cc)
lofar_add_test(tDelayAndBandPass tDelayAndBandPass.cc)

# tests built by nvcc
cuda_add_executable(t_cuda_complex t_cuda_complex.cu)
add_test(t_cuda_complex ${CMAKE_CURRENT_SOURCE_DIR}/t_cuda_complex.sh)
#lofar_add_test(tDelayCompensation tDelayCompensation.cc)

