# $Id: CMakeLists.txt 32341 2015-08-28 11:59:26Z schaap $

lofar_add_bin_scripts(resourceassigner)

# supervisord config files
install(FILES
  resourceassigner.ini
  DESTINATION etc/supervisord.d)

