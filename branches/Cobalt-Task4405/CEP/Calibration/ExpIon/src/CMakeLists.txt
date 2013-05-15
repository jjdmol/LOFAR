# $Id: CMakeLists.txt 14508 2010-03-24 13:18:18Z vdtol $

include(PythonInstall)

# Python modules.
python_install(
  __init__.py
  io.py
  format.py
  ionosphere.py
  acalc.py
  client.py
  sphere.py
  error.py
  mpfit.py
  readms.py
  parmdbmain.py
  DESTINATION lofar/expion)

install(PROGRAMS calibrate-ion readms-part.sh readms-part.py parmdbwriter.py DESTINATION bin)

lofar_add_library(fitting MODULE fitting.cc)

set_target_properties(fitting PROPERTIES
  PREFIX "")
#   LIBRARY_OUTPUT_DIRECTORY ${PYTHON_BUILD_DIR}/lofar/expion)

# This is a quick-and-dirty fix to install the Python binding module in the
# right place. It will now be installed twice, because lofar_add_library()
# will install it in $prefix/$libdir
install(TARGETS fitting
  DESTINATION ${PYTHON_INSTALL_DIR}/lofar/expion)
