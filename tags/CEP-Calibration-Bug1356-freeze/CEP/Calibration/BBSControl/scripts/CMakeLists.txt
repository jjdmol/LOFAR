# $Id$

include(PythonInstall)

# Python modules.
python_install(
  __init__.py 
  solfetch.py
  solplot.py
  solflag.py
  DESTINATION lofar/solutions)
