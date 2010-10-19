# $Id$

include(LofarPackageVersion)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

lofar_add_library(_pyparameterset MODULE pyparameterset.cc)
set_target_properties(_pyparameterset PROPERTIES 
  PREFIX ""
  LIBRARY_OUTPUT_DIRECTORY ${PYTHON_BUILD_DIR}/lofar/parameterset)

# This is a quick-and-dirty fix to install the Python binding module in the
# right place. It will now be installed twice, because lofar_add_library()
# will install it in $prefix/$libdir
install(TARGETS _pyparameterset 
  DESTINATION ${PYTHON_INSTALL_DIR}/lofar/parameterset)

lofar_add_library(lofar_pyparameterset Package__Version.cc)

lofar_add_bin_program(versionpyparameterset 
  versionpyparameterset.cc)

# Install Python modules
include(PythonInstall)
python_install(__init__.py DESTINATION lofar/parameterset)
