# $Id$

# mac_interface
mac_add_protocol(
  "${MACIO_SOURCE_DIR}/SB_Protocol" 
  "${MACIO_SOURCE_DIR}/autogen")
lofar_add_library(_mac_interface MODULE mac_interface.cc)
set_target_properties(_mac_interface PROPERTIES
  PREFIX ""
  LIBRARY_OUTPUT_DIRECTORY ${PYTHON_BUILD_DIR})
install(TARGETS _mac_interface DESTINATION ${PYTHON_INSTALL_DIR})

# mac_control
lofar_add_library(_mac_control MODULE mac_control.cc)
set_target_properties(_mac_control PROPERTIES
  PREFIX ""
  LIBRARY_OUTPUT_DIRECTORY ${PYTHON_BUILD_DIR})
install(TARGETS _mac_control DESTINATION ${PYTHON_INSTALL_DIR})

# Add MAC protocols (generates a .ph and .cc file from a .prot file)
include(MACAddProtocol)

# ep_interface
mac_add_protocol(
  "${MACIO_SOURCE_DIR}/SB_Protocol" 
  "${MACIO_SOURCE_DIR}/autogen")
lofar_add_library(_ep_interface MODULE ep_interface.cc)
set_target_properties(_ep_interface PROPERTIES
  PREFIX ""
  LIBRARY_OUTPUT_DIRECTORY ${PYTHON_BUILD_DIR})
install(TARGETS _ep_interface DESTINATION ${PYTHON_INSTALL_DIR})

# ep_echo
mac_add_protocol(Echo_Protocol "${MACIO_SOURCE_DIR}/autogen")
lofar_add_library(_ep_echo MODULE ep_echo.cc echowrappers.cc Echo_Protocol.cc)
set_target_properties(_ep_echo PROPERTIES
  PREFIX ""
  LIBRARY_OUTPUT_DIRECTORY ${PYTHON_BUILD_DIR})
install(TARGETS _ep_echo DESTINATION ${PYTHON_INSTALL_DIR})

# ep_control
mac_add_protocol(
  "${APLCommon_SOURCE_DIR}/src/Controller_Protocol" 
  "${MACIO_SOURCE_DIR}/autogen")
lofar_add_library(_ep_control MODULE ep_control.cc)
set_target_properties(_ep_control PROPERTIES
  PREFIX ""
  LIBRARY_OUTPUT_DIRECTORY ${PYTHON_BUILD_DIR})
install(TARGETS _ep_control DESTINATION ${PYTHON_INSTALL_DIR})

