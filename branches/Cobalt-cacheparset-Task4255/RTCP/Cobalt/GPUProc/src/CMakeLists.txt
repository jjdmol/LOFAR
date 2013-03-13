# $Id: CMakeLists.txt 17003 2011-01-06 08:54:59Z romein $

include(LofarPackageVersion)

# Add current source directory to -I path. This is needed because GPUProc uses
# angle brackets for internal header files, instead of quotes.
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

lofar_add_library(gpuproc
  #Package__Version.cc
  BandPass.cc
  FilterBank.cc
  OpenCL_Support.cc
  PerformanceCounter.cc
  Input/BeamletBuffer.cc
  Input/BeamletBufferToComputeNode.cc
  Input/CN_Mapping.cc
  Input/Delays.cc
  Input/InputSection.cc
  Input/InputThread.cc
  Input/LogThread.cc
  Input/ReaderWriterSynchronization.cc
  Storage/SSH.cc
  Storage/StorageProcess.cc
  Storage/StorageProcesses.cc
  UHEP/InvertedStationPPFWeights.cc)

lofar_add_bin_program(RTCP RTCP.cc)
lofar_add_bin_program(RTCP_new 
   RTCP_new.cc
   createProgram.cc
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
   Kernels/UHEP_TriggerKernel.cc
   Kernels/FFT_Kernel.cc
   Kernels/FFT_Plan.cc
   Kernels/Filter_FFT_Kernel.cc
   Kernels/DedispersionForwardFFTkernel.cc
   Kernels/DedispersionBackwardFFTkernel.cc
   Pipeline.cc
   Pipelines/CorrelatorPipeline.cc
   Pipelines/BeamFormerPipeline.cc
   Pipelines/UHEP_Pipeline.cc
   WorkQueues/WorkQueue.cc
   WorkQueues/CorrelatorWorkQueue.cc
   WorkQueues/BeamFormerWorkQueue.cc
   WorkQueues/UHEP_WorkQueue.cc)
