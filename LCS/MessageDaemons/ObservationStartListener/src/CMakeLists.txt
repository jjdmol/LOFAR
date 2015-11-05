# $Id$

include(PythonInstall)

python_install(
  __init__.py
  ObservationStartListener.py
  DESTINATION lofar/ObservationStartListener
)

lofar_add_bin_scripts(
  lofarlogger.sh
  slurm-submit-cobalt-outputproc.sh
)
