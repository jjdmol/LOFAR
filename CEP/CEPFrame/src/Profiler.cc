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
//  $Log$
//  Revision 1.12  2002/12/19 10:25:54  schaaf
//
//  %[BugId: 150]%
//  Modified debug levels
//
//  Revision 1.11  2002/05/16 15:21:01  schaaf
//  added MPE calls for ScaMPI
//
//  Revision 1.10  2002/05/03 11:21:31  gvd
//  Changed for new build environment (mostly added package name to include)
//
//  Revision 1.9  2002/03/01 08:27:57  gvd
//  Replaced firewall by Debug and changed code accordingly
//  Added lofar_*.h for correct use of namespaces (for KAI and Intel C++)
//
//  Revision 1.8  2001/10/26 10:06:27  wierenga
//  Wide spread changes to convert from Makedefs to autoconf/automake/libtool build environment
//
//  Revision 1.7  2001/09/05 08:49:20  wierenga
//  Profiler.cc
//
//  Revision 1.6  2001/03/23 10:00:40  gvd
//  Improved documentation and test programs
//  Added clearEventCount function to Step
//
//  Revision 1.5  2001/03/01 13:15:47  gvd
//  Added type argument in DataHolder constructor which is used in
//  the connect functions to check if the DH types match
//  Improved the simulator parser
//  Improved documentation
//
//  Revision 1.4  2001/02/05 14:53:04  loose
//  Added GPL headers
//

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "BaseSim/Profiler.h"
#include "Common/Debug.h"
#include <Common/lofar_iostream.h>

#ifdef HAVE_MPI_PROFILER

int  Profiler::theirNextFreeState=1001;
bool Profiler::theirIsActive=false;    // start in de-activate state

#include "mpi.h"

#ifdef HAVE_MPI
#include "/opt/scali/contrib/mpe/include/mpe.h"
#endif

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


#endif
