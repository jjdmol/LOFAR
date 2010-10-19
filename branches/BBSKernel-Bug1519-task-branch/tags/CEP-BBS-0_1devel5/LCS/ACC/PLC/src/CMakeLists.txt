
## ------------------------------------------------------------------------------
## Build the library

file (GLOB libplc_SOURCES *.cc )

add_library (plc ${libplc_SOURCES})

## ------------------------------------------------------------------------------
## Installation

install (
  TARGETS plc
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )

