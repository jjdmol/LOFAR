//# PerfProfile.h: profile class used to profile Meq functions.
//#
//# Copyright (C) 2002
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#if !defined(PERFPROFILE_H)
#define PERFPROFILE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_MPI_PROFILE

class PerfProfile
{
 public:
  PerfProfile(int start, int stop, const char* tag) : m_stop(stop)
  {
    MPE_Log_event(start, 0, tag);
  }
  ~PerfProfile()
  {
    MPE_Log_event(m_stop, 0, tag);
  }

  private:
    int m_stop;
};

#define PERFPROFILE(tag) \
  static int _mpe_entry_ = MPE_Log_get_event_number(); \
  static int _mpe_exit_  = MPE_Log_get_event_number(); \
  MPEProfile _mpe_profile_(_mpe_entry_, _mpe_exit_, tag);

#else
#define PERFPROFILE()
#endif

#endif
