# $Id$

lofar_find_package(Log4Cxx REQUIRED)
lofar_find_package(Casacore REQUIRED COMPONENTS casa)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

lofar_add_library(askap_askap
  src/AskapError.cc
  src/AskapUtil.cc
  src/ISignalHandler.cc
  src/Log4cxxLogSink.cc
  src/SignalCounter.cc
  src/SignalManagerSingleton.cc)
