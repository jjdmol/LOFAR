# $Id$

include(LofarCTest)

# Add path to our CUDA kernels to the -I path for NVCC
cuda_include_directories(${PACKAGE_SOURCE_DIR}/share/gpu/kernels)


if(UNITTEST++_FOUND)
  # CUDA CPU side UnitTest++ tests
  lofar_add_test(tGPUWrapper tGPUWrapper.cc)
  lofar_add_test(tIncoherentStokes tIncoherentStokes.cc)
  # GPU kernel UnitTest++ tests
  lofar_add_test(tDelayAndBandPass tDelayAndBandPass.cc)
  lofar_add_test(tBandPassCorrection tBandPassCorrection.cc)
endif(UNITTEST++_FOUND)

# CUDA CPU side unit tests
lofar_add_test(tKernel tKernel.cc)
lofar_add_test(tKernelOccupancy tKernelOccupancy.cc)
lofar_add_test(tStreamReadBuffer tStreamReadBuffer.cc)
lofar_add_test(tMultiDimArrayHostBuffer tMultiDimArrayHostBuffer.cc)

# GPU kernel unit tests
lofar_add_test(tIntToFloat tIntToFloat.cc)
lofar_add_test(tFIR_Filter tFIR_Filter.cc)
lofar_add_test(tFFT tFFT.cc)
#lofar_add_executable(tFFT_leakage tFFT_leakage.cc)  # not an automatic test, keep buildable (doesn't right now)
lofar_add_test(tCorrelator tCorrelator.cc)
lofar_add_test(tBeamFormer tBeamFormer.cc)
lofar_add_test(tTranspose tTranspose.cc)
lofar_add_test(tTranspose2 tTranspose2.cc)
lofar_add_test(tCoherentStokesTranspose tCoherentStokesTranspose.cc)
lofar_add_test(tIncoherentStokesTranspose tIncoherentStokesTranspose.cc)

# Trick tests that compile kernels that need to be installed into thinking
# that they can find the kernels using the LOFARROOT environment variable.
set_tests_properties(
  tFIR_Filter
  tCorrelator
  tIntToFloat
  tBeamFormer
  tTranspose
  tTranspose2
  tIncoherentStokesTranspose
  tCoherentStokesTranspose
  PROPERTIES ENVIRONMENT "LOFARROOT=${PACKAGE_SOURCE_DIR}"
)

if(UNITTEST++_FOUND)
  set_tests_properties(
    tDelayAndBandPass
    tBandPassCorrection
    tIncoherentStokes
    PROPERTIES ENVIRONMENT "LOFARROOT=${PACKAGE_SOURCE_DIR}"
  )
endif(UNITTEST++_FOUND)


# Tests built by nvcc

# CUDA driver API tests need a .ptx file compiled first and passed to the host code compilation.
# This way we don't need a generated .sh file, nor compile to .ptx from our test code.
cuda_compile_ptx(tcuda_driver_api_ptx tcuda-driver-api.cu)
cuda_add_executable(tcuda-driver-api  tcuda-driver-api.cu  OPTIONS "-DPTX_FILE_CSTR=\"${tcuda_driver_api_ptx}\"")
target_link_libraries(tcuda-driver-api cuda)
add_test(tcuda-driver-api  "${CMAKE_CURRENT_BINARY_DIR}/tcuda-driver-api")

cuda_add_executable(tcuda-runtime-api tcuda-runtime-api.cu)
add_test(tcuda-runtime-api "${CMAKE_CURRENT_BINARY_DIR}/tcuda-runtime-api")

