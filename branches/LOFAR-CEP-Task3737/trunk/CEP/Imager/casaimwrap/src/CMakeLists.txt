# $Id$

include(LofarPackageVersion)

lofar_add_library(_casaimwrap MODULE casaimwrap.cc)
set_target_properties(_casaimwrap PROPERTIES 
  PREFIX ""
  LIBRARY_OUTPUT_DIRECTORY ${PYTHON_BUILD_DIR}/lofar/casaimwrap)

# This is a quick-and-dirty fix to install the Python binding module in the
# right place. It will now be installed twice, because lofar_add_library()
# will install it in $prefix/$libdir
install(TARGETS _casaimwrap
  DESTINATION ${PYTHON_INSTALL_DIR}/lofar/casaimwrap)

# Install Python modules
include(PythonInstall)
python_install(__init__.py DESTINATION lofar/casaimwrap)

# Install Python scripts
install(PROGRAMS pyimager.py DESTINATION bin)

