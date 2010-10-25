# $Id: CMakeLists.txt 14508 2010-03-24 13:18:18Z vdtol $

include(PythonInstall)

# Python modules.
python_install(
  __init__.py
  io.py
  format.py
  ionosphere.py
  acalc.py
  sphere.py
  error.py
  mpfit.py
  readms.py
  parmdbmain.py
  DESTINATION lofar/expion)

install(PROGRAMS calibrate-ion readms-part.sh readms-part.py write_phases_to_parmdb.py DESTINATION bin)
