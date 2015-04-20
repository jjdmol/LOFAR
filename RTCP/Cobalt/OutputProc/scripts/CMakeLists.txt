# $Id$

if(USE_THREADS)
  lofar_add_sbin_program(udp-copy udp-copy.c common.c)
endif(USE_THREADS)

add_executable(tbb-crc-test tbb-crc-test.cc)
add_executable(tbb-dumpframes tbb-dumpframes.cc)
