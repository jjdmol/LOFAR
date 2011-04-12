# $Id$

include(PythonInstall)

# Python modules.
python_install(
  __init__.py
  Observation.py
  sysconfig.py
  DESTINATION lofar/pipeline)

# Python scripts
install(PROGRAMS
  dppp_node.py
  DPPP.py
  MakeVDS.py
  MWImager.py
  StandardImagingPipeline.py
  DESTINATION bin)
