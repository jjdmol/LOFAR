# $Id$

lofar_package(Docker 1.0)

include(LofarFindPackage)
lofar_find_package(Subversion REQUIRED)

# Create our template engine, using build-specific info
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/template.in
  ${CMAKE_CURRENT_BINARY_DIR}/template @ONLY)

# Directories with Dockerfile.tmpl to parse
set(DOCKER_TEMPLATE_DIRS
  lofar-pipeline
  lofar-outputproc)

# Convert Dockerfile.tmpl -> Dockerfile in ${DOCKER_TEMPLATE_DIRS}
foreach(_dir ${DOCKER_TEMPLATE_DIRS})
  set(_src ${CMAKE_CURRENT_SOURCE_DIR}/${_dir}/Dockerfile.tmpl)
  set(_dst ${CMAKE_CURRENT_BINARY_DIR}/${_dir}_Dockerfile)
  add_custom_command(
    OUTPUT ${_dst}
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/template < ${_src} > ${_dst}
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/template ${_src}
  )
  add_custom_target(${_dir}_Dockerfile_target ALL DEPENDS ${_dst})

  install(FILES
    ${_dst}
    DESTINATION share/docker/${_dir}
    RENAME Dockerfile
  )
endforeach()

# Install everything
install(DIRECTORY
  lofar-base
  lofar-pipeline
  lofar-outputproc
  DESTINATION share/docker
  PATTERN Dockerfile.tmpl EXCLUDE)
