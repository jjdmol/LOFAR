# $Id: CMakeLists.txt 33404 2016-01-27 10:21:14Z mol $

lofar_add_bin_scripts(
  rataskspecifiedservice
)

# supervisord config files
install(FILES
  rataskspecifiedservice.ini
  DESTINATION etc/supervisord.d)
