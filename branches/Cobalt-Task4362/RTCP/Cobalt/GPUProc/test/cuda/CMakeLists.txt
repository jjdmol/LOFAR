lofar_add_test(tCUDAException tCUDAException.cc)
cuda_add_executable(t_kernel t_kernel.cu)
target_link_libraries(t_kernel ${${PACKAGE_NAME}_LINK_LIBRARIES})
#add_test(t_kernel t_kernel)

