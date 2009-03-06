cmake_minimum_required (VERSION 2.6)

include(LOFARTest)
include_directories ( ${CMAKE_CURRENT_SOURCE_DIR} )

find_path (HAVE_LINK_FCNP FCNP ${CMAKE_CURRENT_SOURCE_DIR})
if (HAVE_LINK_FCNP)
  set (HAVE_LINK_FCNP TRUE CACHE BOOL "Have link FCNP?" FORCE)
  execute_process(COMMAND rm -rf ${CMAKE_CURRENT_SOURCE_DIR}/FCNP )
endif (HAVE_LINK_FCNP)

execute_process(COMMAND ln -s ${FCNP_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/FCNP )

## ------------------------------------------------------------------------------
## Sourses

file (GLOB tBandwidth_cn_SRCS            tBandwidth_cn.cc )
file (GLOB tBandwidth_ion_SRCS           tBandwidth_ion.cc )


## --- Macro(s) -------------------------------------

MACRO (TARGETLINKLIBRARIES_FCNP_TEST _PROG)
  target_link_libraries (${_PROG} 
                         fcnp_ion
			 fcnp_cn
 			 common
			 ${BACKTRACE_LIBRARIES} )
ENDMACRO (TARGETLINKLIBRARIES_FCNP_TEST)


## --- Programs to be built -------------------------

set (CHECKPROGS
     tBandwidth_cn
     tBandwidth_ion
)

foreach (prog ${CHECKPROGS})
  add_executable (${prog} ${${prog}_SRCS})
  TARGETLINKLIBRARIES_FCNP_TEST (${prog})
endforeach (prog ${CHECKPROGS})

set_target_properties (tBandwidth_ion
  PROPERTIES
  LINK_FLAGS "-lpthread"
)
