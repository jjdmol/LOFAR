## ------------------------------------------------------------------------------
## Include files

file (GLOB libstorage_HEADERS InputThread.h
			      SubbandWriter.h
			      MSWriter.h
			      MSWriterCasa.h
			      MSWriterNull.h
			      MSWriterFile.h)
   
## ------------------------------------------------------------------------------
## Installation

install (FILES ${libstorage_HEADERS}
  DESTINATION include/Storage
  )
