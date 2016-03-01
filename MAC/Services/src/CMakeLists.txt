# $Id$

lofar_add_bin_scripts(
  pipelinestarter
)

python_install(
  PipelineStarter.py
  DESTINATION lofar/mac
)

# supervisord config files
install(FILES
  pipelinestarter.ini
  DESTINATION etc/supervisord.d)
