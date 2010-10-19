
## ------------------------------------------------------------------------------
## Build the library

file (GLOB libaps_SOURCES *.cc )

add_library (aps ${libaps_SOURCES})

## ------------------------------------------------------------------------------
## Installation

install (
  TARGETS aps
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )

