# $Id$

lofar_add_sbin_scripts(
  flash_images.sh
  restart_images.sh
  view_images.sh
  rsuctl
  rsuctl3
  rsuctl3.pl
  rsuctl3_reset
  flash_tbb.sh)

install(FILES
  README.rsuctl
  DESTINATION sbin)
