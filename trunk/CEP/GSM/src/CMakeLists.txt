# $Id$

include(PythonInstall)

# Install Python modules
python_install(
  __init__.py
  gsmutils.py
  gsm_exceptions.py
  DESTINATION lofar/gsm)

lofar_add_bin_scripts(gsm.py)
