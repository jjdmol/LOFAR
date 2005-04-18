//# Profiler.h:
//#
//# Copyright (C) 2000, 2001
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

#ifndef BASESIM_PROFILER_H
#define BASESIM_PROFILER_H

#include <lofar_config.h>

namespace LOFAR
{

/**
   The Profiler class is used to profile the programme during execution.
   When HAVE_MPI_PROFILER is not defined the class is de-activated by using
   dummy inlined methods.
   If active, the MPE library is used to track wall-clock time spent
   in a particular user defined state.
*/

class Profiler
{
public:

  /// Initialise the Profiler; Go into default de-activated state.
  static void init();

  /** Define a state with name and a colour for the graphical
      representation.
      An identifier for the state is returned. This identified must be 
      passed with the enter/leave State() calls.
  */
  static int defineState (const char* name="noname",
			  const char* color="yellow");

  /// Flag entering a particular state.
  static void enterState (int astate);

  /// Flag leaving a particular state.
  static void leaveState (int astate);

  /// Switch on the Profiler.
  static void activate();

  /// Switch off the Profiler.
  static void deActivate();

private:
  static int  theirNextFreeState;
  static bool theirIsActive;       
  
};

class ProfilingState
{
public:
  ProfilingState();
  
  void enter();
  void leave();
  void init(const char* name, const char* color);

private:
  bool itsInState;
  int itsState;
};

#ifndef HAVE_MPI_PROFILER
inline void Profiler::init() {};
inline int  Profiler::defineState (const char*, const char*) {return 0;}
inline void Profiler::enterState (int) {};
inline void Profiler::leaveState (int) {};
inline void Profiler::activate() {};
inline void Profiler::deActivate() {};

inline void ProfilingState::enter() {};
inline void ProfilingState::leave() {};

#else

inline void ProfilingState::enter() {
  if (!itsInState){
    Profiler::enterState(itsState);
    itsInState = true;
  }
};
inline void ProfilingState::leave() {
  if (itsInState) {
    Profiler::leaveState(itsState);
    itsInState = false;
  }
};

#endif

}

#endif
