cmake_minimum_required (VERSION 2.6)

include(LOFARTest)
include_directories ( ${LOFAR_SOURCE_DIR}/LCS/ApplCommon/test )

## ------------------------------------------------------------------------------
## Sourses

file (GLOB tObservation_SRCS tObservation.cc )

## --- Macro(s) -------------------------------------

MACRO (TARGETLINKLIBRARIES_APPLCOMMON_TEST _PROG)
  target_link_libraries (${_PROG} 
                         applcommon
			 common
			 ${Boost_LIBRARIES}
			 ${LOG4CPLUS_LIBRARIES}
			 ${BACKTRACE_LIBRARIES}
			 ${SHMEM_LIBRARIES} )
ENDMACRO (TARGETLINKLIBRARIES_APPLCOMMON_TEST)

## --- Programs to be built -------------------------

set (CHECKPROGS
     tObservation
)

foreach (prog ${CHECKPROGS})
  add_executable (${prog} ${${prog}_SRCS})
  TARGETLINKLIBRARIES_APPLCOMMON_TEST (${prog})
endforeach (prog ${CHECKPROGS})

## --- Programs to be run ---------------------------

set (CHECKTOOLPROGS
     tObservation
)

foreach (prog ${CHECKTOOLPROGS})
  add_test(${prog} ${CMAKE_CURRENT_SOURCE_DIR}/${prog}.sh) 
  set (TEST_PROGRAMS "${TEST_PROGRAMS};${prog}" CACHE STRING "Test programs" FORCE)
endforeach (prog ${CHECKTOOLPROGS})
