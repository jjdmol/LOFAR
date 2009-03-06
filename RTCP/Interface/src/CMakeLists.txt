## ------------------------------------------------------------------------------
## Build the library

file (GLOB libinterface_SOURCES Allocator.cc
				CN_Configuration.cc
				CN_Mapping.cc
				Parset.cc
				RSPTimeStamp.cc
				CN_Mode.cc)

add_library (interface ${libinterface_SOURCES})

## ------------------------------------------------------------------------------
## Installation

install (
  TARGETS interface
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )
