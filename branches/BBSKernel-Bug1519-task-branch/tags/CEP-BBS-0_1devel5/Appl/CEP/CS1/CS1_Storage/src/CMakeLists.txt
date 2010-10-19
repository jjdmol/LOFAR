
## ------------------------------------------------------------------------------
## Build the library

file (GLOB libstorage_SOURCES *.cc)

add_library (storage ${libstorage_SOURCES})

## linker instructions
target_link_libraries (  storage 
                         amcbase
                         cepframe
                         cs1_interface
                         tinycep
                         plc
                         transport
                         applcommon
                         blob
                         aps
                         common                          
			 ${LOG4CPLUS_LIBRARIES}
			 ${CASACORE_LIBRARIES}
			 ${BOOST_LIBRARIES}
			 )

add_executable(CS1_Storage CS1_Storage_main.cc)

target_link_libraries (CS1_Storage
                       storage
		      )

## ------------------------------------------------------------------------------
## Installation

install (
  TARGETS storage CS1_Storage
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  )  
