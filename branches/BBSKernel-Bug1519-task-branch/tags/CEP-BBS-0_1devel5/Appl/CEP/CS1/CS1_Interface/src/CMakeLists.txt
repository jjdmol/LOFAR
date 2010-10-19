
## ------------------------------------------------------------------------------
## Build the library

if (HAVE_APS)	
  add_definitions (-DHAVE_APS)
endif (HAVE_APS)

file (GLOB libcs1_interface_SOURCES *.cc)

add_library (cs1_interface ${libcs1_interface_SOURCES})

## ------------------------------------------------------------------------------
## Installation

install (
  TARGETS cs1_interface
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )

