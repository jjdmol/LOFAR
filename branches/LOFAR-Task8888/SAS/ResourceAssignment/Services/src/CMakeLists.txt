# $Id$

lofar_add_bin_scripts(
  jobstoscheduleservice
)

python_install(
  JobsToSchedule.py
  RABusListener.py
  DESTINATION lofar/sas/resourceassignment
)

# supervisord config files
install(FILES
  jobstoscheduleservice.ini
  DESTINATION etc/supervisord.d)
