## ------------------------------------------------------------------------------
## Build the library

file (GLOB libapplcommon_SOURCES Observation.cc )

add_library (applcommon ${libapplcommon_SOURCES})

## ------------------------------------------------------------------------------
## Installation

install (
  TARGETS applcommon
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )

