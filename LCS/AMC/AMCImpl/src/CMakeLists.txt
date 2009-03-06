## --- Macro(s) -------------------------------------

MACRO (TARGETLINKLIBRARIES_AMCIMPL_SRC _PROG)
  target_link_libraries (${_PROG} 
                	 amcimpl
			 amcbase
			 transport
			 blob
			 common
			 ${CASACORE_LIBRARIES}
	        	 ${LOG4CPLUS_LIBRARIES}
			 ${BACKTRACE_LIBRARIES}
			 ${SHMEM_LIBRARIES} )
ENDMACRO (TARGETLINKLIBRARIES_AMCIMPL_SRC)

## ------------------------------------------------------------------------------
## Build the library

file (GLOB libamcimpl_SOURCES ConverterImpl.cc
			      ConverterProcess.cc
			      ConverterServer.cc )

add_library (amcimpl ${libamcimpl_SOURCES})

## ------------------------------------------------------------------------------
## Build Programs

add_executable (AMCServer AMCServer.cc)
TARGETLINKLIBRARIES_AMCIMPL_SRC (AMCServer)

add_executable (WGS84toITRF WGS84toITRF.cc)
TARGETLINKLIBRARIES_AMCIMPL_SRC (WGS84toITRF)

## ------------------------------------------------------------------------------
## Installation

install (
  TARGETS amcimpl
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )
