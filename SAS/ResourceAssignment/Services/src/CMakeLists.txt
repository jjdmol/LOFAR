# $Id$

lofar_add_bin_scripts(
  rataskspecifiedservice
)

python_install(
  RATaskSpecified.py
  RABusListener.py
  config.py
  DESTINATION lofar/sas/resourceassignment/rataskspecified
)

# supervisord config files
install(FILES
  rataskspecifiedservice.ini
  DESTINATION etc/supervisord.d)
