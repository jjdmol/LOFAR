# $Id$

include(LofarPackageVersion)
include(PythonInstall)

# Add current build directory to the include path. This is needed, because
# pyparmdb.cc #include's Package__Version.cc (yucky!).
include_directories(${CMAKE_CURRENT_BINARY_DIR})

lofar_add_library(_parmdb MODULE pyparmdb.cc)
set_target_properties(_parmdb PROPERTIES 
  PREFIX ""
  LIBRARY_OUTPUT_DIRECTORY ${PYTHON_BUILD_DIR}/lofar/parmdb)

# This is a quick-and-dirty fix to install the Python binding module in the
# right place. It will now be installed twice, because lofar_add_library()
# will install it in $prefix/$libdir
install(TARGETS _parmdb
  DESTINATION ${PYTHON_INSTALL_DIR}/lofar/parmdb)

# We only need this library because dependent packages need to link against
# this package's version info.
lofar_add_library(lofar_pyparmdb Package__Version.cc)

lofar_add_bin_program(versionpyparmdb versionpyparmdb.cc)

# Install Python modules
python_install(__init__.py DESTINATION lofar/parmdb)

