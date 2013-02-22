# $Id: CMakeLists.txt 13414 2009-06-16 22:15:37Z loose $

include(LofarCTest)

# Add project's source directory to -I path.
include_directories(${PACKAGE_SOURCE_DIR}/src)
#include_directories(${PACKAGE_SOURCE_DIR}/test)

include_directories("/cm/shared/apps/UnitTest++/src")

lofar_add_test(tCorrelateRealData)

#lofar_add_test(tDelayCompensation tDelayCompensation.cc)
#target_link_libraries(tDelayCompensation /cm/shared/apps/UnitTest++/libUnitTest++.a)

set(src_dir ${PACKAGE_SOURCE_DIR}/src)

#lofar_add_bin_program( RTCP_UnitTest 
lofar_add_bin_program( RTCP_UnitTest 
   RTCP_UnitTest.cc
   ${src_dir}/createProgram.cc
   ${src_dir}/UHEP/InvertedStationPPFWeights.cc
   ${src_dir}/PerformanceCounter.cc
   ${src_dir}/global_defines.cc
   ${src_dir}/Kernel.cc
   ${src_dir}/Kernels/FIR_FilterKernel.cc
   ${src_dir}/Kernels/DelayAndBandPassKernel.cc
   ${src_dir}/Kernels/CorrelatorKernel.cc
   ${src_dir}/Kernels/IntToFloatKernel.cc
   ${src_dir}/Kernels/IncoherentStokesKernel.cc
   ${src_dir}/Kernels/BeamFormerKernel.cc
   ${src_dir}/Kernels/BeamFormerTransposeKernel.cc
   ${src_dir}/Kernels/DedispersionChirpKernel.cc
   ${src_dir}/Kernels/CoherentStokesKernel.cc
   ${src_dir}/Kernels/UHEP_BeamFormerKernel.cc
   ${src_dir}/Kernels/UHEP_TransposeKernel.cc
   ${src_dir}/Kernels/UHEP_InvFFT_Kernel.cc
   ${src_dir}/Kernels/UHEP_InvFIR_Kernel.cc
   ${src_dir}/Kernels/UHEP_TriggerKernel.cc   
   UnitTests/new_style_unittest.cc
   ${src_dir}/UnitTest.cc
  )
target_link_libraries(RTCP_UnitTest /cm/shared/apps/UnitTest++/libUnitTest++.a)

