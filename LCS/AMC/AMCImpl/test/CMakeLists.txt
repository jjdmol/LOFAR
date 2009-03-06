cmake_minimum_required (VERSION 2.6)

include(LOFARTest)
include_directories ( ${LOFAR_SOURCE_DIR}/LCS/AMC/AMCImpl/test )

## ------------------------------------------------------------------------------
## Sourses

file (GLOB tConverter_SRCS       tConverter.cc )
file (GLOB tConverterStress_SRCS tConverterStress.cc )

## --- Macro(s) -------------------------------------

MACRO (TARGETLINKLIBRARIES_AMCIMPL_TEST _PROG)
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
ENDMACRO (TARGETLINKLIBRARIES_AMCIMPL_TEST)

## --- Programs to be built -------------------------

set (CHECKPROGS
     tConverter
     tConverterStress
)

foreach (prog ${CHECKPROGS})
  add_executable (${prog} ${${prog}_SRCS})
  TARGETLINKLIBRARIES_AMCIMPL_TEST (${prog})
  add_dependencies (${prog} AMCServer)
endforeach (prog ${CHECKPROGS})

## --- Programs to be run ---------------------------

set (CHECKTOOLPROGS
     tConverter
     tConverterStress
)

foreach (prog ${CHECKTOOLPROGS})
  add_test(${prog} ${CMAKE_CURRENT_SOURCE_DIR}/${prog}.sh) 
  set (TEST_PROGRAMS "${TEST_PROGRAMS};${prog}" CACHE STRING "Test programs" FORCE)
endforeach (prog ${CHECKTOOLPROGS})
