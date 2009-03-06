cmake_minimum_required (VERSION 2.6)

include(LOFARTest)
include_directories ( ${LOFAR_SOURCE_DIR}/RTCP/Interface/test )

add_definitions (-DHAVE_PKVERSION)

## ------------------------------------------------------------------------------
## Sourses

file (GLOB tSparseSet_SRCS         tSparseSet.cc )
file (GLOB tRSPTimeStamp_SRCS      tRSPTimeStamp.cc )

## --- Macro(s) -------------------------------------

MACRO (TARGETLINKLIBRARIES_INTERFACE_TEST _PROG)
  target_link_libraries (${_PROG} 
                         interface
			 common
			 ${BACKTRACE_LIBRARIES}
			 ${LOG4CPLUS_LIBRARIES}
			 ${SHMEM_LIBRARIES} )
ENDMACRO (TARGETLINKLIBRARIES_INTERFACE_TEST)


## --- Programs to be built -------------------------

set (CHECKPROGS
     tSparseSet
     tRSPTimeStamp
)

foreach (prog ${CHECKPROGS})
  add_executable (${prog} ${${prog}_SRCS})
  TARGETLINKLIBRARIES_INTERFACE_TEST (${prog})
endforeach (prog ${CHECKPROGS})

## --- Programs to be run ---------------------------

set (CHECKTOOLPROGS
     tSparseSet
     tRSPTimeStamp
)

foreach (prog ${CHECKTOOLPROGS})
  add_test(${prog} ${prog}) 
  set (TEST_PROGRAMS "${TEST_PROGRAMS};${prog}" CACHE STRING "Test programs" FORCE)
endforeach (prog ${CHECKTOOLPROGS})
