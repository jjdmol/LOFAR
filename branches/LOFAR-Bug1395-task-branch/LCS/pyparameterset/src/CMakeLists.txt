# $Id$

include(LofarPackageVersion)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

lofar_add_library(_pyparameterset SHARED pyparameterset.cc)
set_target_properties(_pyparameterset PROPERTIES PREFIX "")

lofar_add_bin_program(versionpyparameterset 
  versionpyparameterset.cc
  Package__Version.cc)

# Install Python modules
include(PythonInstall)
python_install(__init__.py DESTINATION lofar/parameterset)
