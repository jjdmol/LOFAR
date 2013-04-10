lofar_add_test(tCUDAException tCUDAException.cc)
message(STATUS "CUDA_LIBRARIES = ${CUDA_LIBRARIES}")
cuda_add_executable(t_kernel t_kernel.cu)
#add_test(t_kernel t_kernel)

