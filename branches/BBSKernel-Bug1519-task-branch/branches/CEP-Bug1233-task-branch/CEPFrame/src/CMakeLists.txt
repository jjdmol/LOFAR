
## ------------------------------------------------------------------------------
## Build the library

file (GLOB libcepframe_SOURCES *.cc)

add_library (cepframe ${libcepframe_SOURCES})

## ------------------------------------------------------------------------------
## Installation

install (
  TARGETS cepframe
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )

