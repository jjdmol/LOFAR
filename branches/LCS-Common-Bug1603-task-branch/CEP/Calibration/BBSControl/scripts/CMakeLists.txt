# $Id$

include(PythonInstall)

# Python modules.
python_install(
  __init__.py 
  solfetch.py
  solplot.py
  solflag.py
  DESTINATION lofar/solutions)

install(PROGRAMS addImagingColumns.py DESTINATION bin)
install(PROGRAMS parmdbplot.py DESTINATION bin)
install(PROGRAMS casapy2bbs.py DESTINATION bin)
