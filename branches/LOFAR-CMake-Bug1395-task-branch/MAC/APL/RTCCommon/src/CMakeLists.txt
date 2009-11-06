# $Id$

include(LofarPackageVersion)

# Search for a number of required system header files
#include(CheckIncludeFile)
#check_include_file(string.h       HAVE_STRING_H)
#check_include_file(sys/time.h     HAVE_SYS_TIME_H)
#check_include_file(unistd.h       HAVE_UNISTD_H)
#check_include_file(sys/types.h    HAVE_SYS_TYPES_H)
#check_include_file(sys/stat.h     HAVE_SYS_STAT_H)
#check_include_file(sys/resource.h HAVE_SYS_RESOURCE_H)
#check_include_file(signal.h       HAVE_SIGNAL_H)

lofar_add_library(rtccommon
  Package__Version.cc
  Timestamp.cc
  Subject.cc
  daemonize.c
  RegisterState.cc)

lofar_add_library(gnuplot_i gnuplot_i.cc)

lofar_add_bin_program(versionrtccommon versionrtccommon.cc)

install(FILES
  rc.status
  DESTINATION etc)
