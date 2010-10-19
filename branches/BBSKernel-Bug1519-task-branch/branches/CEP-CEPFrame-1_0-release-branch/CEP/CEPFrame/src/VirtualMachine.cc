//  VirtualMachine.cc:
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
//////////////////////////////////////////////////////////////////////

#include <CEPFrame/VirtualMachine.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{

VirtualMachine::VirtualMachine():
  itsState(running),
  itsStatus(unknown)
{
  LOG_TRACE_FLOW("VirtualMachine Constructor");
}

VirtualMachine::~VirtualMachine() 
{
  LOG_TRACE_FLOW("VirtualMachine Destructor");
}

void VirtualMachine::trigger (VirtualMachine::Trigger aTrig)
{
  State oldState = itsState;

  switch (itsState) {
  case Idle:
    switch (aTrig) {
    case start:
      itsState = running;
      break;
    case stop:
      itsState = pause;
      break;
    case abort:
      itsState = aborting;
      break;
    default:
      THROW(LOFAR::Exception, "Unknown Trigger");
      break;
    }
    break;
    
  case pause:
    switch (aTrig) {
    case start:
      itsState = running;
      break;
    case stop:
      itsState = pause;
      break;
    case abort:
      itsState = aborting;
      break;
    default:
      THROW(LOFAR::Exception, "Unknown Trigger");
      break;
    }
    break;
    
  case running:
    switch (aTrig) {
    case start:
      itsState = running;
      break;
    case stop:
      itsState = pause;
      break;
    case abort:
      itsState = aborting;
      break;
    default:
      THROW(LOFAR::Exception, "Unknown Trigger");
      break;
    }
    break;
    
  case aborting:
    switch (aTrig) {
    case start:
      itsState = aborting;
      break;
    case stop:
      itsState = aborting;
      break;
    case abort:
      itsState = aborting;
      break;
    default:
      THROW(LOFAR::Exception, "Unknown Trigger");
      break;
    }
    break;
    
  default:
    THROW(LOFAR::Exception, "Unknown State");
    break;

  }
  // complete output
  LOG_TRACE_FLOW_STR ("Called VM::trigger(" 
	   << asString(oldState) << '/'
	   << asString(aTrig)    << '/' 
	   << asString(itsState) << ')');
  return;
}


string VirtualMachine::asString (VirtualMachine::Trigger aTrig) const
{
  switch (aTrig) {
  case start:
    return "Start";
  case stop:
    return "Stop ";
  case abort:
    return "Abort";
  default:
    return "unknown";
  }
}

string VirtualMachine::asString (VirtualMachine::State aState) const
{
  switch (aState) {
  case Idle:
    return "Idle    ";
  case pause:
    return "Pause   ";
  case running:
    return "Running ";
  case aborting:
    return "Aborting";
  default:
    return "unknown";
  }
}

string VirtualMachine::asString (VirtualMachine::Status aStatus) const
{
  switch (aStatus) {
  case alive:
    return "Alive   ";
  case degraded:
    return "Degraded";
  case dying:
    return "Dying   ";
  default:
    return "unknown";
  }
}

}
