# $Id: CMakeLists.txt 13414 2009-06-16 22:15:37Z loose $

include(LofarCTest)

# Add project's source directory to -I path.
# This is needed, because the way that header files are currently included by
# the different test programs is broken.
include_directories(${PACKAGE_SOURCE_DIR}/src)

lofar_add_test(tBestEffortQueue tBestEffortQueue.cc)
lofar_add_test(tContext tContext.cc)
lofar_add_test(tPerformanceCounter tPerformanceCounter.cc)
lofar_add_test(tCorrelate_1sec_1st_5sb_noflagging)
lofar_add_test(tCorrelate_3sec_1st_5sb)
lofar_add_test(tCorrelate_3sec_2st_5sb)

lofar_add_test(tSSH tSSH.cc)
lofar_add_bin_program(DummyStorage DummyStorage.cc)
lofar_add_test(tStorageProcesses tStorageProcesses.cc)
#lofar_add_test(tDelayCompensation tDelayCompensation.cc)
lofar_add_test(RTCP_UnitTest RTCP_UnitTest.cc)

lofar_find_package(UnitTest++)

if(UNITTEST++_FOUND)
  set(_src_dir ${PACKAGE_SOURCE_DIR}/src)

  # Add project's source directory to -I path.
  include_directories(${_src_dir})

  lofar_add_test(tCorrelatorWorkQueue 
                 UnitTests/WorkQueues/CorrelatorWorkQueue.cc
				 UnitTests/new_style_unittest.cc
				 )

  #target_link_libraries(RTCP_UnitTest /cm/shared/apps/UnitTest++/libUnitTest++.a)

endif(UNITTEST++_FOUND)
