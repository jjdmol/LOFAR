add_custom_target(dqg_example_group
  COMMAND ${CMAKE_COMMAND} -E
    make_directory "${CMAKE_BINARY_DIR}/doc/html/doxygen/examples/group"
  COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/group.cfg)

add_custom_target(dqg_example_memgrp
  COMMAND ${CMAKE_COMMAND} -E
    make_directory "${CMAKE_BINARY_DIR}/doc/html/doxygen/examples/memgrp"
  COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/memgrp.cfg)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/group.cfg
  ${CMAKE_CURRENT_BINARY_DIR}/group.cfg
  @ONLY)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/memgrp.cfg
  ${CMAKE_CURRENT_BINARY_DIR}/memgrp.cfg
  @ONLY)
