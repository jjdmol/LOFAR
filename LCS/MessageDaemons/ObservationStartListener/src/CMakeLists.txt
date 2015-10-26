# $Id$

include(PythonInstall)

python_install(
  __init__.py
  ObservationStartListener.py
  DESTINATION lofar/ObservationStartListener
)

lofar_add_bin_scripts(
  slurm-submit-cobalt-outputproc.sh
)

install(FILES
  lofarlogger.sh
  DESTINATION bin
)
