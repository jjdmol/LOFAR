# $Id: CMakeLists.txt 13414 2009-06-16 22:15:37Z loose $

include(LofarCTest)

lofar_add_test(tCorrelateRealData)

lofar_find_package(UnitTest++)

if(UNITTEST++_FOUND)
  set(_src_dir ${PACKAGE_SOURCE_DIR}/src)

  # Add project's source directory to -I path.
  include_directories(${_src_dir})

  # Commented out this test to prevent automated running:
  # new task should be added to have a proper run and sh file with the executable
  # lofar_add_test(RTCP_UnitTest 
    # RTCP_UnitTest.cc
    # UnitTests/new_style_unittest.cc
    # ${_src_dir}/createProgram.cc
    # ${_src_dir}/PerformanceCounter.cc
    # ${_src_dir}/global_defines.cc
    # ${_src_dir}/Kernel.cc
    # ${_src_dir}/UnitTest.cc
    # ${_src_dir}/Kernels/FIR_FilterKernel.cc
    # ${_src_dir}/Kernels/DelayAndBandPassKernel.cc
    # ${_src_dir}/Kernels/CorrelatorKernel.cc
    # ${_src_dir}/Kernels/IntToFloatKernel.cc
    # ${_src_dir}/Kernels/IncoherentStokesKernel.cc
    # ${_src_dir}/Kernels/BeamFormerKernel.cc
    # ${_src_dir}/Kernels/BeamFormerTransposeKernel.cc
    # ${_src_dir}/Kernels/DedispersionChirpKernel.cc
    # ${_src_dir}/Kernels/CoherentStokesKernel.cc
    # ${_src_dir}/Kernels/UHEP_BeamFormerKernel.cc
    # ${_src_dir}/Kernels/UHEP_TransposeKernel.cc
    # ${_src_dir}/Kernels/UHEP_InvFFT_Kernel.cc
    # ${_src_dir}/Kernels/UHEP_InvFIR_Kernel.cc
    # ${_src_dir}/Kernels/UHEP_TriggerKernel.cc   
    # ${_src_dir}/UHEP/InvertedStationPPFWeights.cc
    # )
  #target_link_libraries(RTCP_UnitTest /cm/shared/apps/UnitTest++/libUnitTest++.a)

endif(UNITTEST++_FOUND)
