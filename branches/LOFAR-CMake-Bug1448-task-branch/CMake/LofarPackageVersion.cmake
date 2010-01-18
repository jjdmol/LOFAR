configure_file(
  ${LOFAR_ROOT}/CMake/MakePackageVersion.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/MakePackageVersion.cmake @ONLY)

#add_custom_target(${PACKAGE_NAME}_Version ALL
#  COMMAND ${CMAKE_COMMAND} 
#  ARGS -P ${CMAKE_CURRENT_BINARY_DIR}/MakePackageVersion.cmake)

include(${CMAKE_CURRENT_BINARY_DIR}/MakePackageVersion.cmake)

if(0)
# Mark files Package__Version.cc and version<pkg>.cc as generated;
# otherwise CMake will complain it cannot find the source files.
string(TOLOWER ${PACKAGE_NAME} _lpkg)
message(STATUS "_lpkg=${_lpkg}")
set(_generated 
  ${CMAKE_CURRENT_BINARY_DIR}/Package__Version.cc 
  ${CMAKE_CURRENT_BINARY_DIR}/version${_lpkg}.cc)
message(STATUS "set_source_files_properties(${_generated} PROPERTIES GENERATED ON)")
#set_source_files_properties(${_generated} PROPERTIES GENERATED ON)
set_source_files_properties(Package__Version.cc version${_lpkg}.cc
  PROPERTIES GENERATED ON)
endif(0)

include_directories(${CMAKE_BINARY_DIR}/include/Package__Version)
