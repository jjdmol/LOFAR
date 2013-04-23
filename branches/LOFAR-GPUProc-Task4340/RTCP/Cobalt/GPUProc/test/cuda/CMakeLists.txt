lofar_add_test(tCUDAException tCUDAException.cc)
cuda_add_executable(t_cuda_complex t_cuda_complex.cu)
add_test(t_cuda_complex t_cuda_complex)
lofar_add_test(tcreateProgram tcreateProgram.cc)
 
