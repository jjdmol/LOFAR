
## ------------------------------------------------------------------------------
## Build the library

file (GLOB liblofar_impl_SOURCES *.cc)

add_library (lofar_impl SHARED ${liblofar_impl_SOURCES})

target_link_libraries (lofar_impl
  amcimpl
  amcbase
  cepframe
  cs1_interface
  tinycep
  plc
  transport
  applcommon
  blob
  aps
  common
  )

set_target_properties (lofar_impl
  PROPERTIES
  LINK_FLAGS "-fPIC --whole-archive"
)

## ------------------------------------------------------------------------------
## Installation

install (
  TARGETS lofar_impl
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )

