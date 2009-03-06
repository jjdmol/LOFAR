cmake_minimum_required (VERSION 2.6)

include(LOFARTest)
include_directories ( ${LOFAR_SOURCE_DIR}/LCS/AMC/AMCBase/test )

## ------------------------------------------------------------------------------
## Sourses

file (GLOB tCoord3D_SRCS          tCoord3D.cc )
file (GLOB tDirection_SRCS        tDirection.cc )
file (GLOB tPosition_SRCS         tPosition.cc )
file (GLOB tEpoch_SRCS            tEpoch.cc )
file (GLOB tConverterCommand_SRCS tConverterCommand.cc )
file (GLOB tConverterStatus_SRCS  tConverterStatus.cc )
file (GLOB tBlobIO_SRCS           tBlobIO.cc )
file (GLOB tDH_Request_SRCS       tDH_Request.cc )
file (GLOB tDH_Result_SRCS        tDH_Result.cc )

## --- Macro(s) -------------------------------------

MACRO (TARGETLINKLIBRARIES_AMCBASE_TEST _PROG)
  target_link_libraries (${_PROG} 
                         amcbase
			 transport
			 blob
			 common
			 ${LOG4CPLUS_LIBRARIES}
			 ${BACKTRACE_LIBRARIES}
			 ${SHMEM_LIBRARIES} )
ENDMACRO (TARGETLINKLIBRARIES_AMCBASE_TEST)

## --- Programs to be built -------------------------

set (CHECKPROGS
     tCoord3D
     tDirection
     tPosition
     tEpoch
     tConverterCommand
     tConverterStatus
     tBlobIO
     tDH_Request
     tDH_Result
)

foreach (prog ${CHECKPROGS})
  add_executable (${prog} ${${prog}_SRCS})
  TARGETLINKLIBRARIES_AMCBASE_TEST (${prog})
endforeach (prog ${CHECKPROGS})

## --- Programs to be run ---------------------------

set (CHECKTOOLPROGS
     tCoord3D
     tDirection
     tPosition
     tEpoch
     tConverterCommand
     tConverterStatus
     tBlobIO
     tDH_Request
     tDH_Result
)

foreach (prog ${CHECKTOOLPROGS})
  add_test(${prog} ${CMAKE_CURRENT_SOURCE_DIR}/${prog}.sh) 
  set (TEST_PROGRAMS "${TEST_PROGRAMS};${prog}" CACHE STRING "Test programs" FORCE)
endforeach (prog ${CHECKTOOLPROGS})
