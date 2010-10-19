
## ------------------------------------------------------------------------------
## Build the library

file (GLOB libtransport_SOURCES *.cc)

add_library (transport ${libtransport_SOURCES})

## ------------------------------------------------------------------------------
## Installation

install (
  TARGETS transport
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )

