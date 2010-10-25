# $Id$

lofar_add_sbin_program(tci tci.c)

install(PROGRAMS 
  plotrubidium 
  filter.py 
  rr.py 
  rubidium_logger.py 
  rlp.py 
  DESTINATION sbin)

