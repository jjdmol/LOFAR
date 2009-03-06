## ------------------------------------------------------------------------------
## Build the library

file (GLOB libstorage_SOURCES InputThread.cc
			      SubbandWriter.cc
			      MSWriter.cc
			      MSWriterCasa.cc
			      MSWriterNull.cc
			      MSWriterFile.cc)


add_library (storage ${libstorage_SOURCES})

## linker instructions
target_link_libraries (  storage 
                         interface
                         stream
			 amcbase
			 applcommon
                         common
			 ${Boost_LIBRARIES}
			 ${CASA_LIBRARIES}
			 ${LOG4CPLUS_LIBRARIES}
			 ${BACKTRACE_LIBRARIES}
			 ${SHMEM_LIBRARIES}
			 )


add_executable(Storage Storage_main.cc)

target_link_libraries (Storage
                       storage
		      )

## ------------------------------------------------------------------------------
## Installation

install (
  TARGETS storage
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  )
