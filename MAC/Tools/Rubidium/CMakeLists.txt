# $Id$

if(HAVE_SYS_TIMEPPS_H)
  lofar_add_sbin_program(tci tci.c)
endif(HAVE_SYS_TIMEPPS_H)

install(PROGRAMS 
  plotrubidium 
  filter.py 
  rr.py 
  rubidium_logger.py 
  rlp.py 
  DESTINATION sbin)

