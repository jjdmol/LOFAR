# $Id$

include(LofarPackageVersion)

# Create symbolic link in binary include directory to current directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/TestCtlr)

lofar_add_library(testctlr Package__Version.cc)
  
lofar_add_bin_program(versiontestctlr versiontestctlr.cc)

lofar_add_sbin_program(ControllerMenu ControllerMenuMain.cc ControllerMenu.cc)
lofar_add_sbin_program(TestController TestControllerMain.cc TestController.cc)
lofar_add_sbin_program(CodeLoggingTest CodeLoggingTest.cc)
