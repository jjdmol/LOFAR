# $Id: CMakeLists.txt 17003 2011-01-06 08:54:59Z romein $

include(LofarPackageVersion)

# Create symbolic link to include directory.
#execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
#  ${CMAKE_CURRENT_SOURCE_DIR}
#  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

# Add current source directory to -I path. This is needed because GPUProc uses
# angle brackets for internal header files, instead of quotes.
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(gpuproc_LIB_SRCS
  #Package__Version.cc
  BandPass.cc
  BeamletBuffer.cc
  BeamletBufferToComputeNode.cc
  Delays.cc
  FilterBank.cc
  InputSection.cc
  InputThread.cc
  ION_Allocator.cc
  LogThread.cc
  OpenCL_Support.cc
  ReaderWriterSynchronization.cc
  RTCP.cc
  
  UHEP/InvertedStationPPFWeights.cc)

lofar_add_library(gpuproc ${gpuproc_LIB_SRCS})

lofar_add_bin_program(RTCP RTCP.cc)
lofar_add_bin_program(RTCP_new RTCP_new.cc
   createProgram.cc
   PerformanceCounter.cc
   global_defines.cc
   UnitTest.cc 
   Kernel.cc
   Kernels/FIR_FilterKernel.cc
   Kernels/DelayAndBandPassKernel.cc
   Kernels/CorrelatorKernel.cc
   Kernels/IntToFloatKernel.cc
   Kernels/IncoherentStokesKernel.cc
   Kernels/BeamFormerKernel.cc
   Kernels/BeamFormerTransposeKernel.cc
   Kernels/DedispersionChirpKernel.cc
   Kernels/CoherentStokesKernel.cc
   Kernels/UHEP_BeamFormerKernel.cc
   Kernels/UHEP_TransposeKernel.cc
   Kernels/UHEP_InvFFT_Kernel.cc
   Kernels/UHEP_InvFIR_Kernel.cc
   )
#lofar_add_bin_program(versiongpuproc versiongpuproc.cc)


