# $Id$

lofar_package(Docker 1.0)

include(LofarFindPackage)
lofar_find_package(Subversion REQUIRED)

#
# For given directories, we generate the Dockerfile
# by parsing their Dockerfile.tmpl through "template".
#
# "template" is a script that fills in variables with
# respect to the build info (branch name, build time, etc)
#

# Directories with Dockerfile.tmpl to parse
set(DOCKER_TEMPLATE_DIRS
  lofar-base
  lofar-pipeline
  lofar-outputproc)

# Create our template engine, using build-specific info
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/template.in
  ${CMAKE_CURRENT_BINARY_DIR}/template @ONLY)

# Note: "template" only works as long as the sources are still around,
# since it uses svn to query information from them.
lofar_add_bin_scripts(${CMAKE_CURRENT_BINARY_DIR}/template)
lofar_add_sbin_scripts(docker-build-all.sh)

# Convert Dockerfile.tmpl -> Dockerfile in ${DOCKER_TEMPLATE_DIRS}
foreach(_dir ${DOCKER_TEMPLATE_DIRS})
  # _src -> _dst
  set(_src ${CMAKE_CURRENT_SOURCE_DIR}/${_dir}/Dockerfile.tmpl)
  set(_dst ${CMAKE_CURRENT_BINARY_DIR}/${_dir}_Dockerfile)

  # add generating command, and (any) target to force the generation
  # when "all" is build.
  add_custom_command(
    OUTPUT ${_dst}
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/template < ${_src} > ${_dst}
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/template ${_src}
  )
  add_custom_target(${_dir}_Dockerfile_target ALL DEPENDS ${_dst})

  # install resulting Dockerfile
  install(FILES
    ${_dst}
    DESTINATION share/docker/${_dir}
    RENAME Dockerfile
  )
endforeach()

# Install everything else
install(DIRECTORY
  lofar-base
  lofar-pipeline
  lofar-outputproc
  DESTINATION share/docker
  USE_SOURCE_PERMISSIONS
  PATTERN Dockerfile.tmpl EXCLUDE)
