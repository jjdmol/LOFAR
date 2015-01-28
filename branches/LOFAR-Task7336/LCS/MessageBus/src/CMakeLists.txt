# $Id$

include(LofarPackageVersion)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

set(messagebus_LIB_SRCS
  Package__Version.cc
  MsgBus.cc)

set(messagebus_PROGRAMS
)

lofar_add_library(messagebus ${messagebus_LIB_SRCS})

foreach(prog ${messagebus_PROGRAMS})
  lofar_add_bin_program(${prog} ${prog}.cc)
endforeach(prog ${messagebus_PROGRAMS})
