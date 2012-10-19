# $Id$

if(HAVE_SYS_TIMEPPS_H)
  lofar_add_sbin_program(ppsctl ppsctl.c)
endif(HAVE_SYS_TIMEPPS_H)

install(PROGRAMS plotclock DESTINATION sbin)
