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

#if !defined(MEQ_PERFPROFILE_H)
#define MEQ_PERFPROFILE_H

#include <Common/lofar_string.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if defined(HAVE_MPI_PROFILER) && defined(HAVE_MPICH)

#define MPICH_SKIP_MPICXX
#include <mpe.h>

#include <stdio.h>

class PerfProfile
{
public:
  inline PerfProfile(int start, int stop) : m_stop(stop)
  {
    MPE_Start_log();
    MPE_Log_event(start, start, (char*)0);
  }

  inline ~PerfProfile()
  {
    MPE_Log_event(m_stop, m_stop, (char*)0);
  }

  static int get_second_event_number(int start, const char* tag)
  {
    static int cur_color_index = 0;
    int stop = MPE_Log_get_event_number();
    MPE_Describe_state(start, stop, (char*)tag,
		       (char*)PerfProfile::m_colors[cur_color_index]);
    cur_color_index = (cur_color_index + 1 ) % PerfProfile::m_nr_colors;

    return stop;
  }

private:
  int m_stop;
  string m_tag;

  static const char* const m_colors[];
  static int m_nr_colors;
};

#define PERFPROFILE(tag) \
  static int _mpe_entry_ = MPE_Log_get_event_number(); \
  static int _mpe_exit_  = PerfProfile::get_second_event_number(_mpe_entry_, tag); \
  PerfProfile _mpe_profile_(_mpe_entry_, _mpe_exit_);

#else
#define PERFPROFILE(tag)
#endif

#endif
