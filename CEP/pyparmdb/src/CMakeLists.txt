# $Id$

include(LofarPackageVersion)

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

# Dummy library, needed because lofar_add_executable() takes its dependencies
# from libraries added with lofar_add_library() (see bug #1430)
lofar_add_library(lofar_pyparmdb Package__Version.cc)

lofar_add_bin_program(versionpyparmdb versionpyparmdb.cc)

# Install Python modules
include(PythonInstall)
python_install(__init__.py DESTINATION lofar/parmdb)
