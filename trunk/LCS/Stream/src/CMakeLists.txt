## ------------------------------------------------------------------------------
## Build the library

file (GLOB libstream_SOURCES FileDescriptorBasedStream.cc
			     FileStream.cc
			     NullStream.cc
			     SocketStream.cc
			     Stream.cc
			     SystemCallException.cc)

add_library (stream ${libstream_SOURCES})

## ------------------------------------------------------------------------------
## Installation

install (
  TARGETS stream
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )

