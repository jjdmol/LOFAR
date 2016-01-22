# $Id$

lofar_add_bin_scripts(
  JobsToSchedule.py
)

# supervisord config files
install(FILES
  JobsToSchedule.ini
  DESTINATION etc/supervisord.d)
