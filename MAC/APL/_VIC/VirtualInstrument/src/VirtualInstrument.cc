//#  VirtualInstrument.cc: Implementation of the Virtual VirtualInstrument task
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include "VirtualInstrument.h"


namespace LOFAR
{
  using namespace ACC;
  using namespace APLCommon;
  
namespace AVI
{

VirtualInstrument::VirtualInstrument(const string& taskName, const string& parameterFile) :
  LogicalDevice(taskName,parameterFile)
{
  LOG_DEBUG(formatString("VirtualInstrument(%s)::VirtualInstrument",getName().c_str()));
}


VirtualInstrument::~VirtualInstrument()
{
  LOG_DEBUG(formatString("VirtualInstrument(%s)::~VirtualInstrument",getName().c_str()));
}

::GCFEvent::TResult VirtualInstrument::concrete_initial_state(::GCFEvent& /*e*/, ::GCFPortInterface& /*p*/, TLogicalDeviceState /*newState*/)
{
  ::GCFEvent::TResult status = GCFEvent::HANDLED;
  
  return status;
}

::GCFEvent::TResult VirtualInstrument::concrete_claiming_state(::GCFEvent& /*e*/, ::GCFPortInterface& /*p*/, TLogicalDeviceState /*newState*/)
{
  ::GCFEvent::TResult status = GCFEvent::HANDLED;
  
  return status;
}

::GCFEvent::TResult VirtualInstrument::concrete_preparing_state(::GCFEvent& /*e*/, ::GCFPortInterface& /*p*/, TLogicalDeviceState /*newState*/)
{
  ::GCFEvent::TResult status = GCFEvent::HANDLED;
  
  return status;
}

::GCFEvent::TResult VirtualInstrument::concrete_active_state(::GCFEvent& /*e*/, ::GCFPortInterface& /*p*/, TLogicalDeviceState /*newState*/)
{
  ::GCFEvent::TResult status = GCFEvent::HANDLED;
  
  return status;
}

::GCFEvent::TResult VirtualInstrument::concrete_releasing_state(::GCFEvent& /*e*/, ::GCFPortInterface& /*p*/, TLogicalDeviceState /*newState*/)
{
  ::GCFEvent::TResult status = GCFEvent::HANDLED;
  
  return status;
}

void VirtualInstrument::concreteClaim(::GCFPortInterface& /*port*/)
{
}

void VirtualInstrument::concretePrepare(::GCFPortInterface& /*port*/)
{
}

void VirtualInstrument::concreteResume(::GCFPortInterface& /*port*/)
{
}

void VirtualInstrument::concreteSuspend(::GCFPortInterface& /*port*/)
{
}

void VirtualInstrument::concreteRelease(::GCFPortInterface& /*port*/)
{
}

void VirtualInstrument::concreteParentDisconnected(::GCFPortInterface& /*port*/)
{
}

void VirtualInstrument::concreteChildDisconnected(::GCFPortInterface& /*port*/)
{
}

}; // namespace VIC
}; // namespace LOFAR
