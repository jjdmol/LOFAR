# $Id$

include(LofarPackageVersion)

set(amcimpl_LIB_SRCS
  Package__Version.cc
  ConverterImpl.cc
  ConverterProcess.cc
  ConverterServer.cc
)

set(amcimpl_PROGRAMS
  versionamcimpl
  AMCServer
  WGS84toITRF)

## Suppress warnings from the Casacore Measures component.
#set_source_files_properties(${amcimpl_LIB_SRCS} ${amcimpl_PROGRAMS}
#  PROPERTIES COMPILE_FLAGS "-Wno-ignored-qualifiers")

lofar_add_library(amcimpl ${amcimpl_LIB_SRCS})

foreach(prog ${amcimpl_PROGRAMS})
  lofar_add_bin_program(${prog} ${prog}.cc)
endforeach(prog ${amcimpl_PROGRAMS})
