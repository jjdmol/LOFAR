cmake_minimum_required (VERSION 2.6)

include(LOFARTest)
include_directories ( ${LOFAR_SOURCE_DIR}/RTCP/IONProc/test )

## ------------------------------------------------------------------------------
## Sources

file (GLOB tDelayCompensation_SRCS     tBeamForming.cc
				       )
				       
## --- Macro(s) -------------------------------------

MACRO (TARGETLINKLIBRARIES_IONPROC_TEST _PROG)
  target_link_libraries (${_PROG} 
                         ionproc )
ENDMACRO (TARGETLINKLIBRARIES_IONPROC_TEST)

## --- Programs to be built -------------------------

set (CHECKPROGS
     tDelayCompensation
)

foreach (prog ${CHECKPROGS})
  add_executable (${prog} ${${prog}_SRCS})
  TARGETLINKLIBRARIES_CNPROC_TEST (${prog})
  set_target_properties(${prog} PROPERTIES LINK_FLAGS "-static")
endforeach (prog ${CHECKPROGS})				       

