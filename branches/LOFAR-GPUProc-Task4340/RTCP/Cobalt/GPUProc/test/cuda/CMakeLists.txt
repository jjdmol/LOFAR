# $Id$

# To compile .cu files, invoke CUDA's nvcc, but respect the host C/C++ compiler (variants) selection.
if(LOFAR_COMPILER_SUITES STREQUAL "GNU")
  set(COMPILER_BINDIR_OPT "--compiler-bindir ${GNU_CXX}")
elseif(LOFAR_COMPILER_SUITES STREQUAL "CLANG")
  set(COMPILER_BINDIR_OPT "--compiler-bindir ${CLANG_CXX}")
endif()

lofar_add_test(tCUDAException tCUDAException.cc)
cuda_add_executable(t_cuda_complex t_cuda_complex.cu OPTIONS ${COMPILER_BINDIR_OPT})
add_test(t_cuda_complex t_cuda_complex)
lofar_add_test(tcreateProgram tcreateProgram.cc)
 
