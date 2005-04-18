//  Profiler.cc:
//
//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//
//////////////////////////////////////////////////////////////////////////////

#include <lofar_config.h>

#include "tinyCEP/Profiler.h"
#include <Common/Debug.h>
#include <Common/lofar_iostream.h>

#ifdef HAVE_MPI_PROFILER

#include "mpi.h"

#ifdef HAVE_MPI
#include "mpe.h"
#endif

namespace LOFAR
{

int  Profiler::theirNextFreeState=1001;
bool Profiler::theirIsActive=false;    // start in de-activate state

void Profiler::init() {
  MPE_Init_log();
  MPI_Pcontrol(0);        // switch off MPI build-in profiling
  theirIsActive = false;   // start in de-activate state
}

int Profiler::defineState (const char* name, const char* color)
{
  int startstate = theirNextFreeState++; // the start state
  int endstate   = theirNextFreeState++; // the end state
  MPE_Describe_state (startstate, endstate, (char*)name, (char*)color);
  cdebug(3) << "Defined State " << name
	    << " start=" << startstate
	    << "  end=" << endstate << endl;
  return startstate;
}

void Profiler::enterState (int astate)
{
  if (theirIsActive) {
    MPE_Log_event (astate, 1, (char*)0);
  }
  return;
}

void Profiler::leaveState (int astate)
{
  if (theirIsActive) {
    MPE_Log_event (astate+1, 1, (char*)0);
  }
  return;
}


void Profiler::activate()
{
  MPE_Start_log();
  // switch on the Profiler
  MPI_Pcontrol (1);         // switch on MPI build-in profiling
  theirIsActive = true;
}


void Profiler::deActivate()
{
  MPE_Finish_log("MPIlog");
  // switch off the Profiler
  MPI_Pcontrol (0);         // switch off MPI build-in profiling
  theirIsActive = false;
}

}

#endif // HAVE_MPI_PROFILER is defined

namespace LOFAR {

ProfilingState::ProfilingState():
  itsInState(false),
  itsState(0)
{
}

void ProfilingState::init(const char* name, const char* color)
{
  if (itsState==0)
    itsState = Profiler::defineState(name, color);
}

} //namespace LOFAR

