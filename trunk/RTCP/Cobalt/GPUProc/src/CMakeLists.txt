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
  Storage/SSH.cc
  Storage/StorageProcess.cc
  Storage/StorageProcesses.cc
)

if(USE_CUDA)
  list(APPEND _gpuproc_sources 
    cuda/gpu_wrapper.cc
    cuda/gpu_utils.cc
    cuda/CudaRuntimeCompiler.cc
    cuda/PerformanceCounter.cc
    cuda/Kernels/Kernel.cc
    cuda/Kernels/BeamFormerKernel.cc
    cuda/Kernels/BeamFormerTransposeKernel.cc
    cuda/Kernels/CoherentStokesKernel.cc
    cuda/Kernels/CorrelatorKernel.cc
    cuda/Kernels/DedispersionBackwardFFTkernel.cc
    cuda/Kernels/DedispersionChirpKernel.cc
    cuda/Kernels/DedispersionForwardFFTkernel.cc
    cuda/Kernels/DelayAndBandPassKernel.cc
    cuda/Kernels/FFT_Kernel.cc
    cuda/Kernels/FFT_Plan.cc
    cuda/Kernels/Filter_FFT_Kernel.cc
    cuda/Kernels/FIR_FilterKernel.cc
    cuda/Kernels/IncoherentStokesKernel.cc
    cuda/Kernels/IntToFloatKernel.cc
    #cuda/Kernels/UHEP_BeamFormerKernel.cc
    #cuda/Kernels/UHEP_InvFFT_Kernel.cc
    #cuda/Kernels/UHEP_InvFIR_Kernel.cc
    #cuda/Kernels/UHEP_TransposeKernel.cc
    #cuda/Kernels/UHEP_TriggerKernel.cc
#    cuda/Pipelines/Pipeline.cc
#    cuda/Pipelines/BeamFormerPipeline.cc
#    cuda/Pipelines/CorrelatorPipeline.cc
#    cuda/Pipelines/UHEP_Pipeline.cc
#    cuda/WorkQueues/WorkQueue.cc
#    cuda/WorkQueues/BeamFormerWorkQueue.cc
#    cuda/WorkQueues/CorrelatorWorkQueue.cc
#    cuda/WorkQueues/UHEP_WorkQueue.cc
  )
  add_subdirectory(cuda)

  lofar_add_library(gpuproc ${_gpuproc_sources})
  if(CUDA_cufft_LIBRARY)
    target_link_libraries(gpuproc ${CUDA_cufft_LIBRARY})
  endif()
  # Compiling rtcp.cc doesn't work yet for CUDA
  #lofar_add_bin_program(rtcp rtcp.cc)
endif()

if(USE_OPENCL)
  list(APPEND _gpuproc_sources 
    opencl/gpu_wrapper.cc
    opencl/gpu_utils.cc
    opencl/PerformanceCounter.cc
    opencl/Kernels/Kernel.cc
    opencl/Kernels/BeamFormerKernel.cc
    opencl/Kernels/BeamFormerTransposeKernel.cc
    opencl/Kernels/CoherentStokesKernel.cc
    opencl/Kernels/CorrelatorKernel.cc
    opencl/Kernels/DedispersionBackwardFFTkernel.cc
    opencl/Kernels/DedispersionChirpKernel.cc
    opencl/Kernels/DedispersionForwardFFTkernel.cc
    opencl/Kernels/DelayAndBandPassKernel.cc
    opencl/Kernels/FFT_Kernel.cc
    opencl/Kernels/FFT_Plan.cc
    opencl/Kernels/Filter_FFT_Kernel.cc
    opencl/Kernels/FIR_FilterKernel.cc
    opencl/Kernels/IncoherentStokesKernel.cc
    opencl/Kernels/IntToFloatKernel.cc
    #opencl/Kernels/UHEP_BeamFormerKernel.cc
    #opencl/Kernels/UHEP_InvFFT_Kernel.cc
    #opencl/Kernels/UHEP_InvFIR_Kernel.cc
    #opencl/Kernels/UHEP_TransposeKernel.cc
    #opencl/Kernels/UHEP_TriggerKernel.cc
    opencl/Pipelines/Pipeline.cc
    opencl/Pipelines/BeamFormerPipeline.cc
    opencl/Pipelines/CorrelatorPipeline.cc
    #opencl/Pipelines/UHEP_Pipeline.cc
    opencl/WorkQueues/WorkQueue.cc
    opencl/WorkQueues/BeamFormerWorkQueue.cc
    opencl/WorkQueues/CorrelatorWorkQueue.cc
    #opencl/WorkQueues/UHEP_WorkQueue.cc
  )
  add_subdirectory(opencl)

  lofar_add_library(gpuproc_opencl ${_gpuproc_sources})
  lofar_add_bin_program(rtcp_opencl rtcp.cc)
endif()
