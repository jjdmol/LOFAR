//# VirtualMachine.h: Class to hold a collection of Step objects
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

#ifndef CEPFRAME_VIRTUALMACHINE_H
#define CEPFRAME_VIRTUALMACHINE_H

#include <lofar_config.h>

#include <Common/lofar_string.h>

namespace LOFAR
{

class VirtualMachine
{
 public:
  VirtualMachine();
  virtual ~VirtualMachine();
  
  // Enumerator used to describe the State of the Virtual Machine.
  enum State{Idle,pause,running,aborting};

  // Enumerator used to describe the Status of the Virtual Machine.
  enum Status{unknown,alive,degraded,dying};

  // Enumerator used to describe triggers sent to the Virtual Machine.
  enum Trigger{start,stop,abort};

  // Access the Virtual Machine State.
  State getState() const;

  // Access the Virtual Machine Status.
  Status getStatus() const;

  // Send a trigger to the Virtual Machine; the VM state will be updated.
  void trigger (Trigger);

  string asString(Trigger aTrig) const;
  string asString(State   aState) const;
  string asString(Status  aStatus) const;

private:
  State   itsState;
  Status itsStatus;
};


inline VirtualMachine::State VirtualMachine::getState() const
  { return itsState; }

inline VirtualMachine::Status VirtualMachine::getStatus() const
  { return itsStatus; }

}

#endif
