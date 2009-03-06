## ------------------------------------------------------------------------------
## Build the library

file (GLOB libamcbase_SOURCES BlobIO.cc
			      ConverterClient.cc
			      ConverterCommand.cc
			      ConverterStatus.cc
			      Coord3D.cc
			      DH_Request.cc
			      DH_Result.cc
			      Direction.cc
			      Epoch.cc
			      Position.cc )

add_library (amcbase ${libamcbase_SOURCES})

## ------------------------------------------------------------------------------
## Installation

install (
  TARGETS amcbase
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )

