cmake_minimum_required (VERSION 2.6)

include(LOFARTest)
include_directories ( ${LOFAR_SOURCE_DIR}/LCS/Transport/test )

## ------------------------------------------------------------------------------
## Sourses

file (GLOB Example_SRCS            Example.cc DH_Example.cc )
file (GLOB ExampleMem_SRCS         ExampleMem.cc DH_Example.cc DH_ExampleExtra.cc )
file (GLOB ExampleBlMem_SRCS       ExampleBlMem.cc DH_Example.cc DH_ExampleExtra.cc )
file (GLOB ExampleNonBlocking_SRCS ExampleBlMem.cc DH_Example.cc DH_ExampleExtra.cc )
file (GLOB ExampleVarBuf_SRCS      ExampleVarBuf.cc DH_VarBuf.cc )
file (GLOB ExampleShMem_SRCS       ExampleShMem.cc DH_Example.cc DH_ExampleExtra.cc )
file (GLOB ExampleMPI_SRCS         ExampleMPI.cc DH_Example.cc DH_ExampleExtra.cc )
file (GLOB TestBidirectional_SRCS  TestBidirectional.cc DH_Example.cc )
file (GLOB ExampleSocket_SRCS      ExampleSocket.cc DH_Socket.cc )
file (GLOB ExampleEthernet_SRCS    ExampleEthernet.cc DH_Ethernet.cc )
file (GLOB tServer_SRCS            tServer.cc DH_Socket.cc )
file (GLOB tClient_SRCS            tClient.cc DH_Socket.cc )
file (GLOB tCSConnection_SRCS      tCSConnection.cc )

## --- Macro(s) -------------------------------------

MACRO (TARGETLINKLIBRARIES_TRANSPORT_TEST _PROG)
  target_link_libraries (${_PROG} 
                         transport
			 blob
			 common
			 ${LOG4CPLUS_LIBRARIES}
			 ${BACKTRACE_LIBRARIES}
			 ${SHMEM_LIBRARIES} )
ENDMACRO (TARGETLINKLIBRARIES_TRANSPORT_TEST)


## --- Programs to be built -------------------------

set (CHECKPROGS
     Example
     TestBidirectional
     ExampleMem
     ExampleBlMem
     ExampleNonBlocking
     ExampleVarBuf
     ExampleShMem
     ExampleMPI
     ExampleSocket
     ExampleEthernet
     tServer
     tClient
     tCSConnection
)

foreach (prog ${CHECKPROGS})
  add_executable (${prog} ${${prog}_SRCS})
  TARGETLINKLIBRARIES_TRANSPORT_TEST (${prog})
endforeach (prog ${CHECKPROGS})

## --- Programs to be run ---------------------------

set (CHECKTOOLPROGS
     Example
     TestBidirectional
     ExampleMem
     ExampleBlMem
     ExampleNonBlocking
     ExampleVarBuf
     ExampleSocket
)

foreach (prog ${CHECKTOOLPROGS})
  add_test(${prog} ${CMAKE_CURRENT_SOURCE_DIR}/${prog}_test.sh) 
  set (TEST_PROGRAMS "${TEST_PROGRAMS};${prog}" CACHE STRING "Test programs" FORCE)
endforeach (prog ${CHECKTOOLPROGS})

## --- ExampleShMem ---------------------------------

if ( MPI_FOUND )
  add_test (ExampleShMem ${CMAKE_CURRENT_SOURCE_DIR}/ExampleShMem_test.sh)
  set (TEST_PROGRAMS "${TEST_PROGRAMS};ExampleShMem" CACHE STRING "Test programs" FORCE)
else ( MPI_FOUND )
  message (STATUS "Warning: ExampleShMem.run skipped: probably MPI is not configured in!")
endif ( MPI_FOUND ) 

## --- ExampleMPI -----------------------------------

if ( MPI_FOUND )
  add_test (ExampleMPI ${CMAKE_CURRENT_SOURCE_DIR}/ExampleMPI_test.sh)
  set (TEST_PROGRAMS "${TEST_PROGRAMS};ExampleMPI" CACHE STRING "Test programs" FORCE) 
else ( MPI_FOUND )
  message (STATUS "Warning: ExampleMPI.run skipped: probably MPI is not configured in!")
endif ( MPI_FOUND ) 
