# $Id$

lofar_package(CUDASupport 0.3 DEPENDS Common)

lofar_find_package(CUDA REQUIRED)

add_subdirectory(src)
#add_subdirectory(test)
