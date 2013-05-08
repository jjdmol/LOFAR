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
  Storage/StorageProcesses.cc)

if(USE_CUDA)
  list(APPEND _gpuproc_sources 
    cuda/gpu_wrapper.cc
    cuda/CudaRuntimeCompiler.cc
  )
endif()

if(USE_OPENCL)
  list(APPEND _gpuproc_sources 
    opencl/createProgram.cc
    opencl/gpu_wrapper.cc
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
    opencl/OpenCL_Support.cc
    opencl/PerformanceCounter.cc
    opencl/Pipelines/Pipeline.cc
    opencl/Pipelines/BeamFormerPipeline.cc
    opencl/Pipelines/CorrelatorPipeline.cc
    opencl/WorkQueues/BeamFormerWorkQueue.cc
    opencl/WorkQueues/CorrelatorWorkQueue.cc
    opencl/WorkQueues/WorkQueue.cc)
endif()

lofar_add_library(gpuproc ${_gpuproc_sources})

if(USE_OPENCL)
  lofar_add_bin_program(rtcp rtcp.cc)
endif()
