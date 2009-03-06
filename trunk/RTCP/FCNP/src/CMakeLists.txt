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
