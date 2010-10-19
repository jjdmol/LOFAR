
## ------------------------------------------------------------------------------
## Build the library

file (GLOB libamcimpl_SOURCES *.cc)

add_library (amcimpl ${libamcimpl_SOURCES})

## ------------------------------------------------------------------------------
## Installation

install (
  TARGETS amcimpl
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )

