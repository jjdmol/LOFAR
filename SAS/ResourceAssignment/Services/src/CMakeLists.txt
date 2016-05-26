# $Id$

lofar_add_bin_scripts(
  rataskspecifiedservice
)

python_install(
  RATaskSpecified.py
  RABusListener.py
  DESTINATION lofar/sas/resourceassignment
)

# supervisord config files
install(FILES
  rataskspecifiedservice.ini
  DESTINATION etc/supervisord.d)
