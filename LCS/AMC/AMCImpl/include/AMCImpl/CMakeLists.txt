## ------------------------------------------------------------------------------
## Include files

file (GLOB libamcimpl_HEADERS ConverterImpl.h
			      ConverterProcess.h
			      ConverterServer.h )

## ------------------------------------------------------------------------------
## Installation
   
install (FILES ${libamcimpl_HEADERS}
  DESTINATION include/AMCImpl
  )
