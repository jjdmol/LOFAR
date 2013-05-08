# $Id$

include(LofarCTest)

# To compile .cu files, invoke CUDA's nvcc, but respect the host C/C++ compiler (variants) selection.
set(COMPILER_BINDIR_OPT --compiler-bindir ${CMAKE_CXX_COMPILER})

lofar_add_test(tCudaRuntimeCompiler tCudaRuntimeCompiler.cc)

cuda_add_executable(t_cuda_complex t_cuda_complex.cu OPTIONS ${COMPILER_BINDIR_OPT})
add_test(t_cuda_complex ${CMAKE_CURRENT_SOURCE_DIR}/t_cuda_complex.sh)
 
lofar_add_test(tStreamReadBuffer tStreamReadBuffer.cc)
