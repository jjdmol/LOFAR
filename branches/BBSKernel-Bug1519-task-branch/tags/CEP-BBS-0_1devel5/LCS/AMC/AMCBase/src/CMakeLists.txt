
## ------------------------------------------------------------------------------
## Build the library

file (GLOB libamcbase_SOURCES *.cc)

add_library (amcbase ${libamcbase_SOURCES})

## ------------------------------------------------------------------------------
## Installation

install (
  TARGETS amcbase
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )

