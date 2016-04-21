# $Id$

lofar_package(Docker 1.0 DEPENDS Common)

include(LofarPackageVersion)

# Build version info
set(docker_LIB_SRCS
  Package__Version.cc
)

lofar_add_library(docker ${docker_LIB_SRCS})

lofar_add_bin_program(versiondocker versiondocker.cc)

#
# For given directories, we generate the Dockerfile
# by parsing their Dockerfile.tmpl through "docker-template".
#
# "docker-template" is a script that fills in variables with
# respect to the build info (branch name, build time, etc)
#

# Directories with Dockerfile.tmpl to parse
set(DOCKER_TEMPLATE_DIRS
  lofar-base
  lofar-pipeline
  lofar-outputproc)

# Note: "docker-template" only works as long as the sources are still around,
# since it uses svn to query information from them.
lofar_add_bin_scripts(docker-template)
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
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/docker-template < ${_src} > ${_dst}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/docker-template ${_src} ${CMAKE_CURRENT_BINARY_DIR}/versiondocker
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
  lofar-tbbwriter
  DESTINATION share/docker
  USE_SOURCE_PERMISSIONS
  PATTERN Dockerfile.tmpl EXCLUDE)
