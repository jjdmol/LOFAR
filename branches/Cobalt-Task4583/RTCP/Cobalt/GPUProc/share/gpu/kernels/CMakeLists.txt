# $Id$

set(_kernel_sources
  gpu_math.cuh
  IntToFloat.cuh
  BeamFormer.cu
  Correlator.cu
  DelayAndBandPass.cu
  FIR_Filter.cu
  IntToFloat.cu
  Transpose.cu
)

install(FILES ${_kernel_sources} DESTINATION share/gpu/kernels)


# Verify whether all .cu .cuh files in this dir are in the install list.
# (Do not build the list automatically; then cmake cannot see changes (not strictly needed now).)
file(GLOB _found_kernel_sources "*.cu" "*.cuh")
set(_missing_kernel_sources )
foreach(path ${_found_kernel_sources})
  # Found filenames are full path, so strip to compare. (Assume the basename contains no '/'.)
  get_filename_component(filename "${path}" NAME)

  list(FIND _kernel_sources "${filename}" index)
  if(index EQUAL -1)
    list(APPEND _missing_kernel_sources "${filename}")
  endif()
endforeach()
list(LENGTH _missing_kernel_sources _len)
if(_len GREATER 0)
  message(SEND_ERROR "Missing CUDA kernel sources in CMakeFile.txt install list: ${_missing_kernel_sources}")
endif()

