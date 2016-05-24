# $Id$

lofar_add_bin_scripts(
  pipelinecontrol
)

python_install(
  PipelineControl.py
  DESTINATION lofar/mac
)

# supervisord config files
install(FILES
  pipelinecontrol.ini
  DESTINATION etc/supervisord.d)
