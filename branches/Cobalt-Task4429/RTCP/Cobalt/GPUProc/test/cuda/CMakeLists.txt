include(LofarCTest)

lofar_add_test(tCUDAException tCUDAException.cc)
lofar_add_test(tCudaRuntimeCompiler tCudaRuntimeCompiler.cc)

cuda_add_executable(t_cuda_complex t_cuda_complex.cu)
add_test(t_cuda_complex ${CMAKE_CURRENT_SOURCE_DIR}/t_cuda_complex.sh)
 
lofar_add_test(tStreamReadBuffer tStreamReadBuffer.cc)
