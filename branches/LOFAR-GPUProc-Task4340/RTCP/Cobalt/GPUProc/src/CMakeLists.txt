# $Id: CMakeLists.txt 17003 2011-01-06 08:54:59Z romein $

include(LofarPackageVersion)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

set(GPUPROC_SOURCES
  #Package__Version.cc
  BandPass.cc
  FilterBank.cc
  global_defines.cc
  Kernel.cc
  Kernels/BeamFormerKernel.cc
  Kernels/BeamFormerTransposeKernel.cc
  Kernels/CoherentStokesKernel.cc
  Kernels/CorrelatorKernel.cc
  Kernels/DedispersionBackwardFFTkernel.cc
  Kernels/DedispersionChirpKernel.cc
  Kernels/DedispersionForwardFFTkernel.cc
  Kernels/DelayAndBandPassKernel.cc
  Kernels/FFT_Kernel.cc
  Kernels/FFT_Plan.cc
  Kernels/Filter_FFT_Kernel.cc
  Kernels/FIR_FilterKernel.cc
  Kernels/IncoherentStokesKernel.cc
  Kernels/IntToFloatKernel.cc
  #Kernels/UHEP_BeamFormerKernel.cc
  #Kernels/UHEP_InvFFT_Kernel.cc
  #Kernels/UHEP_InvFIR_Kernel.cc
  #Kernels/UHEP_TransposeKernel.cc
  #Kernels/UHEP_TriggerKernel.cc
  OpenCL_Support.cc
  PerformanceCounter.cc
  Pipeline.cc
  Pipelines/BeamFormerPipeline.cc
  Pipelines/CorrelatorPipeline.cc
  #Pipelines/UHEP_Pipeline.cc
  Storage/SSH.cc
  Storage/StorageProcess.cc
  Storage/StorageProcesses.cc
  #UHEP/InvertedStationPPFWeights.cc
  WorkQueues/BeamFormerWorkQueue.cc
  WorkQueues/CorrelatorWorkQueue.cc
  #WorkQueues/UHEP_WorkQueue.cc
  WorkQueues/WorkQueue.cc)

if(USE_CUDA)
  lofar_add_library(gpuproc ${GPUPROC_SOURCES})
  set_target_properties(gpuproc PROPERTIES DEFINE_SYMBOL "USE_CUDA")
  lofar_add_bin_program(rtcp rtcp.cc)
  #cuda_add_executable(rtcp rtcp.cc)
  #cuda_add_cufft_to_target(rtcp)
  target_link_libraries(gpuproc cufft)

  install(FILES
    rtcp
    DESTINATION bin)
endif(USE_CUDA)

if(USE_OPENCL)
  lofar_add_library(gpuproc_opencl ${GPUPROC_SOURCES})
  set_target_properties(gpuproc PROPERTIES DEFINE_SYMBOL "USE_OPENCL")
  lofar_add_bin_program(rtcp_opencl rtcp.cc)

  install(FILES
    rtcp_opencl
    DESTINATION bin)
endif(USE_OPENCL)
