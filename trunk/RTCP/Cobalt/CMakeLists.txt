# $Id: CMakeLists.txt 24129 2013-03-08 14:48:23Z mol $

# We need at least CMake 2.8 for CUDA support
cmake_minimum_required(VERSION 2.8)

lofar_add_package(InputProc)   # GPU cluster: Station Input Retrieval and Redistribution applications 
lofar_add_package(OutputProc)  # GPU cluster: Data Storage applications
lofar_add_package(GPUProc)     # GPU computation applications
lofar_add_package(OpenCL_FFT)  # Apple's FFT library for OpenCL
lofar_add_package(CoInterface) # CoInterface code between Cobalt packages
