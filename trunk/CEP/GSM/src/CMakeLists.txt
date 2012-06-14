# $Id$

include(PythonInstall)

# Install Python modules
python_install(
  gsmutils.py
  gsm_exceptions.py
  DESTINATION lofar/gsm)

install(PROGRAMS
  gsm.py
  DESTINATION bin)
