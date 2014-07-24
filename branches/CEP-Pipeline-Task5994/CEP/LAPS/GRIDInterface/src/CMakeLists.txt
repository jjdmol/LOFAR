# $Id$

include(PythonInstall)

python_install(
  dpu_xml_interface.py
  pcombine.py
  pipeline_job.py

  DESTINATION LAPS/GridInterface)