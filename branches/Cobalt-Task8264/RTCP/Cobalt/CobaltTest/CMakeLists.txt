# $Id$

lofar_package(CobaltTest 1.0 DEPENDS GPUProc OutputProc)

# Handle options USE_CUDA and USE_OPENCL.
if(USE_CUDA AND NOT USE_OPENCL)
  add_definitions(-DUSE_CUDA)
elseif(USE_OPENCL AND NOT USE_CUDA)
  add_definitions(-DUSE_OPENCL)
endif()

add_subdirectory(test)
