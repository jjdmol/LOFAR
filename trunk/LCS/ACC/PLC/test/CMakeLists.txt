cmake_minimum_required (VERSION 2.6)

include(LOFARTest)
include_directories ( ${LOFAR_SOURCE_DIR}/LCS/ACC/PLC/test )

## ------------------------------------------------------------------------------
## Sourses

file (GLOB tPCCmd_SRCS tPCCmd.cc )

## --- Macro(s) -------------------------------------

MACRO (TARGETLINKLIBRARIES_PLC_TEST _PROG)
  target_link_libraries (${_PROG} 
                         plc
			 ${LOG4CPLUS_LIBRARIES}
			 ${BACKTRACE_LIBRARIES}
			 ${SHMEM_LIBRARIES} )
ENDMACRO (TARGETLINKLIBRARIES_PLC_TEST)

## --- Programs to be built -------------------------

set (CHECKPROGS
     tPCCmd
)

foreach (prog ${CHECKPROGS})
  add_executable (${prog} ${${prog}_SRCS})
  TARGETLINKLIBRARIES_PLC_TEST (${prog})
endforeach (prog ${CHECKPROGS})

## --- Programs to be run ---------------------------
