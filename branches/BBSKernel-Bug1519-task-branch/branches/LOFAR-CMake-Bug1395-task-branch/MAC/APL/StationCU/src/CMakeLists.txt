# $Id$

include(LofarPackageVersion)

# Add definition of include path suffix definition, because the header files
# reside in a non-standard location.
set(${PACKAGE_NAME}_INCLUDE_PATH_SUFFIX APL/ClockProtocol
  CACHE INTERNAL "Include path suffix for package \"${PACKAGE_NAME}\"")

# Create symbolic link in binary include directory to current directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/StationCU)

lofar_add_library(stationcu Package__Version.cc)
lofar_add_bin_program(versionstationcu versionstationcu.cc)

add_subdirectory(BeamControl)
add_subdirectory(CalibrationControl)
add_subdirectory(ClockControl)
add_subdirectory(ClockProtocol)
add_subdirectory(HardwareMonitor)
add_subdirectory(StationControl)
add_subdirectory(TBBControl)
add_subdirectory(TempControl)
