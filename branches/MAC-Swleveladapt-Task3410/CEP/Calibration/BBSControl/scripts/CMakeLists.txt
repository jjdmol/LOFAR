# $Id$

include(PythonInstall)

# Python modules.
python_install(
  __init__.py 
  solfetch.py
  solplot.py
  solflag.py
  DESTINATION lofar/solutions)

python_install(
  #__init__.py
  solverquery.py
  plotwindow.py
  plothistogram.py
  plotcorrmatrix.py
  DESTINATION lofar/bbs)

install(PROGRAMS addImagingColumns.py DESTINATION bin)
install(PROGRAMS parmdbplot.py DESTINATION bin)
install(PROGRAMS casapy2bbs.py DESTINATION bin)
install(PROGRAMS solverdialog.py DESTINATION bin)
