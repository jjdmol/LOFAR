# $Id$

configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/cuda_config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cuda_config.h"  # no need to install this header
  )

lofar_add_library(gpuproc_cuda

)
