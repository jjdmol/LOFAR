## ------------------------------------------------------------------------------
## Include files

file (GLOB libstream_HEADERS FileDescriptorBasedStream.h
			     FileStream.h
			     NullStream.h
			     SocketStream.h
			     Stream.h
			     SystemCallException.h )

## ------------------------------------------------------------------------------
## Installation
   
install (FILES ${libstream_HEADERS}
  DESTINATION include/Stream
  )
