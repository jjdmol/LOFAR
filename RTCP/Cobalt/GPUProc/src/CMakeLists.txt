# $Id$

include(LofarPackageVersion)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

set(_gpuproc_sources
  #Package__Version.cc
  BandPass.cc
  CommandThread.cc
  cpu_utils.cc
  FilterBank.cc
  global_defines.cc
  MPIReceiver.cc
  Package__Version.cc
  SysInfoLogger.cc
  Station/StationNodeAllocation.cc
  Station/StationInput.cc
  Station/StationTranspose.cc
  Storage/StorageProcess.cc
  Storage/StorageProcesses.cc
)

if(USE_CUDA)
  list(APPEND _gpuproc_sources 
    cuda/gpu_wrapper.cc
    cuda/gpu_utils.cc
    cuda/KernelFactory.cc
    cuda/PerformanceCounter.cc
    cuda/Kernels/Kernel.cc
    cuda/Kernels/BeamFormerKernel.cc
    cuda/Kernels/BeamFormerTransposeKernel.cc
    cuda/Kernels/CoherentStokesTransposeKernel.cc
    cuda/Kernels/CoherentStokesKernel.cc
    cuda/Kernels/CorrelatorKernel.cc
    #cuda/Kernels/DedispersionBackwardFFTkernel.cc
    #cuda/Kernels/DedispersionChirpKernel.cc
    #cuda/Kernels/DedispersionForwardFFTkernel.cc
    cuda/Kernels/DelayAndBandPassKernel.cc
    cuda/Kernels/BandPassCorrectionKernel.cc
    cuda/Kernels/FFT_Kernel.cc
    cuda/Kernels/FFT_Plan.cc
    cuda/Kernels/FIR_FilterKernel.cc
    cuda/Kernels/IncoherentStokesKernel.cc
    cuda/Kernels/IncoherentStokesTransposeKernel.cc
    cuda/Kernels/IntToFloatKernel.cc
    cuda/Kernels/FFTShiftKernel.cc
    #cuda/Kernels/UHEP_BeamFormerKernel.cc
    #cuda/Kernels/UHEP_InvFFT_Kernel.cc
    #cuda/Kernels/UHEP_InvFIR_Kernel.cc
    #cuda/Kernels/UHEP_TransposeKernel.cc
    #cuda/Kernels/UHEP_TriggerKernel.cc
    cuda/Pipelines/Pipeline.cc
#    cuda/Pipelines/UHEP_Pipeline.cc
    cuda/SubbandProcs/SubbandProc.cc
    cuda/SubbandProcs/SubbandProcInputData.cc
    cuda/SubbandProcs/SubbandProcOutputData.cc
    cuda/SubbandProcs/KernelFactories.cc
    cuda/SubbandProcs/CorrelatorStep.cc
    cuda/SubbandProcs/BeamFormerPreprocessingStep.cc
    cuda/SubbandProcs/BeamFormerCoherentStep.cc
    cuda/SubbandProcs/BeamFormerIncoherentStep.cc
    
#    cuda/SubbandProcs/UHEP_SubbandProc.cc
  )
  add_subdirectory(cuda)

  lofar_add_library(gpuproc ${_gpuproc_sources})
  if(CUDA_cufft_LIBRARY)
    target_link_libraries(gpuproc ${CUDA_cufft_LIBRARY})
  endif()

  lofar_add_bin_program(rtcp rtcp.cc)
  lofar_add_bin_program(getOutputProcHosts getOutputProcHosts.cc)
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
    opencl/SubbandProcs/SubbandProc.cc
    opencl/SubbandProcs/BeamFormerSubbandProc.cc
    opencl/SubbandProcs/CorrelatorSubbandProc.cc
    #opencl/SubbandProcs/UHEP_SubbandProc.cc
  )
  add_subdirectory(opencl)

  lofar_add_library(gpuproc_opencl ${_gpuproc_sources})
  #lofar_add_bin_program(rtcp_opencl rtcp.cc)
endif()

lofar_add_bin_program(mpi_node_list Station/mpi_node_list.cc)
lofar_add_bin_program(station_stream Station/station_stream.cc)
lofar_add_bin_program(send_state send_state.cc)
lofar_add_bin_program(gpu_load gpu_load.cc)

# install scripts used to run an observation under bin
lofar_add_bin_scripts(
  scripts/cobalt_functions.sh
  scripts/CobaltControl.sh
  scripts/generate_globalfs_locations.py
  scripts/runObservation.sh
  scripts/startstopkeys.sh
  scripts/startBGL.sh
  scripts/stopBGL.sh
  scripts/watchlogs.sh)

# install support scripts (install, roll-out, debugging) into sbin
lofar_add_sbin_scripts(
  scripts/bw_monitor.sh
  scripts/cobaltswitch
  scripts/Cobalt_build.sh
  scripts/Cobalt_install.sh
  scripts/CobaltVersions.sh
  scripts/setloglevels.sh)

# install logprop files
install(FILES
  rtcp.log_prop
  getOutputProcHosts.log_prop
  Station/mpi_node_list.log_prop
  Station/station_stream.log_prop
  scripts/watchlogs-multitail.conf
  DESTINATION etc)

