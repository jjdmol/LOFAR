## ------------------------------------------------------------------------------
## Include files

file (GLOB libamcbase_HEADERS BlobIO.h
			      ConverterClient.h
			      ConverterCommand.h
			      Converter.h
			      ConverterStatus.h
			      Coord3D.h
			      DH_Request.h
			      DH_Result.h
			      Direction.h
			      Epoch.h
			      Exceptions.h
			      Position.h
			      RequestData.h
			      ResultData.h )

## ------------------------------------------------------------------------------
## Installation
   
install (FILES ${libamcbase_HEADERS}
  DESTINATION include/AMCBase
  )
