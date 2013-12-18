# $Id$

#set(DOXYGEN_HTML_OUTPUT "${CMAKE_BINARY_DIR}/doc/html/doxygen")
#file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/doc/html/doxygen")

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/quick-guide.cfg.in"
  "${CMAKE_CURRENT_BINARY_DIR}/quick-guide.cfg" @ONLY)

configure_file(
  "${CMAKE_SOURCE_DIR}/CMake/docscripts/MakeDoxyDoc.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/MakeDoxyDoc.cmake" @ONLY)

add_custom_target(doxygen_quick_guide
  COMMAND "${CMAKE_COMMAND}" 
  -D DOXYGEN_CONFIG_FILE="${CMAKE_CURRENT_BINARY_DIR}/quick-guide.cfg"
  -D DOXYGEN_OUTPUT_LOG_FILE="${CMAKE_CURRENT_BINARY_DIR}/quick-guide.log"
  -P "${CMAKE_CURRENT_BINARY_DIR}/MakeDoxyDoc.cmake"
#  DEPENDS quick-guide.dox
#  WORKING_DIRECTORY "${DOXYGEN_OUTPUT_DIRECTORY}"
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  COMMENT "Generating Doxygen Quick Guide ... \n       (see doxygen_quick_guide.log for details)")

add_subdirectory(examples)
