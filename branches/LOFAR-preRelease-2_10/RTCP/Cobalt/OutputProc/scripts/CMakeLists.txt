# $Id$

if(USE_THREADS)
  lofar_add_sbin_program(udp-copy udp-copy.c common.c)
endif(USE_THREADS)
