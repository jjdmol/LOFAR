# $Id$

# Add path to generated ClockProtocol files to -I path.
include_directories(${PACKAGE_BINARY_DIR}/src/ClockProtocol)

lofar_add_bin_program(ClockControl ClockControlMain.cc ClockControl.cc)
target_link_libraries(ClockControl clock_protocol)

lofar_add_bin_program(clkctl clkctl.cc)
target_link_libraries(clkctl clock_protocol)
