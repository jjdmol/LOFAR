# $Id$

include(LofarPackageVersion)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

lofar_add_library(_parmdb SHARED pyparmdb.cc)
set_target_properties(_parmdb PROPERTIES PREFIX "")

# This is a quick-and-dirty fix to install the Python binding module in the
# right place. It will now be installed twice, because lofar_add_library()
# will install it in $prefix/$libdir
install(TARGETS _parmdb
  DESTINATION ${PYTHON_INSTALL_DIR}/lofar/parmdb)

lofar_add_bin_program(versionpyparmdb 
  versionpyparmdb.cc
  Package__Version.cc)

# Install Python modules
include(PythonInstall)
python_install(__init__.py DESTINATION lofar/parmdb)
