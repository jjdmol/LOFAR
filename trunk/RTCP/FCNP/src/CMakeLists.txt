find_path (HAVE_LINK_FCNP FCNP ${CMAKE_CURRENT_SOURCE_DIR})
if (HAVE_LINK_FCNP)
  set (HAVE_LINK_FCNP TRUE CACHE BOOL "Have link FCNP?" FORCE)
  execute_process(COMMAND rm -rf ${CMAKE_CURRENT_SOURCE_DIR}/FCNP )
endif (HAVE_LINK_FCNP)

execute_process(COMMAND ln -s ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/FCNP )

## ------------------------------------------------------------------------------
## Build the library

file (GLOB libfcnp_ion_SOURCES fcnp_ion.cc)
add_library (fcnp_ion ${libfcnp_ion_SOURCES})

file (GLOB libfcnp_cn_SOURCES fcnp_cn.cc)
add_library (fcnp_cn ${libfcnp_cn_SOURCES})

## ------------------------------------------------------------------------------
## Installation

install (
  TARGETS fcnp_ion
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )

install (
  TARGETS fcnp_cn
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )
