# $Id: CMakeLists.txt 17003 2011-01-06 08:54:59Z romein $

include(LofarPackageVersion)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

set(_gpuproc_sources
  #Package__Version.cc
  BandPass.cc
  FilterBank.cc
  global_defines.cc
  Input/BeamletBuffer.cc
  Input/BeamletBufferToComputeNode.cc
  Input/CN_Mapping.cc
  Input/Delays.cc
  Input/InputSection.cc
  Input/InputThread.cc
  Input/LogThread.cc
  Input/ReaderWriterSynchronization.cc
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
  PerformanceCounter.cc
  Storage/SSH.cc
  Storage/StorageProcess.cc
  Storage/StorageProcesses.cc
  #UHEP/InvertedStationPPFWeights.cc
  WorkQueues/BeamFormerWorkQueue.cc
  WorkQueues/CorrelatorWorkQueue.cc
  #WorkQueues/UHEP_WorkQueue.cc
  WorkQueues/WorkQueue.cc)

if(USE_CUDA)
#  list(APPEND _gpuproc_sources CUDA_Support.cc)
endif()

if(USE_OPENCL)
  list(APPEND _gpuproc_sources 
    createProgram.cc
    OpenCL_Support.cc
    Pipeline.cc
    Pipelines/BeamFormerPipeline.cc
    Pipelines/CorrelatorPipeline.cc
    #Pipelines/UHEP_Pipeline.cc
    )
endif()

lofar_add_library(gpuproc ${_gpuproc_sources})

if(USE_OPENCL)
  lofar_add_bin_program(rtcp rtcp.cc)
endif()
