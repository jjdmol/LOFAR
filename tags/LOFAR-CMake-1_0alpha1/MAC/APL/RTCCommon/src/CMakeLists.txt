#  $Id$
#
#  Copyright (C) 2008-2009
#  ASTRON (Netherlands Foundation for Research in Astronomy)
#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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
