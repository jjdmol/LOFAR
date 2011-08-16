# $Id$

install(PROGRAMS
  flash_images.sh
  restart_images.sh
  view_images.sh
  rsuctl
  rsuctl3
  rsuctl3.pl
  rsuctl3_reset
  flash_tbb.sh
  DESTINATION sbin)

install(FILES
  README.rsuctl
  DESTINATION sbin)
