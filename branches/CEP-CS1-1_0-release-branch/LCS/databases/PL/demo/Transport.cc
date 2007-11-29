//#  Transport.cc: implementation of some transportation classes.
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

#include "Transport.h"
#include <iostream>

void Engine::print (std::ostream& os) const 
{
  os << "  " << itsVolume << " l." << std::endl;
}


void GasEngine::setValveCount(int valves)
{
  itsValveCount = valves;
}


void GasEngine::print (std::ostream& os) const 
{
  os << "  " << itsValveCount << " valves (gas)." << std::endl;
  Engine::print (os);
}


void DieselEngine::print (std::ostream& os) const 
{
  os << "  " << itsTurboCharged << " turbo (diesel)." << std::endl;
  Engine::print (os);
}


void Vehicle::print (std::ostream& os) const 
{
  os << "  " << itsMake << std::endl;
  os << "  " << itsWheelCount << " wheels." << std::endl;
}

std::string Vehicle::getMake() const
{
  return itsMake;
}


void MotorCycle::setValveCount(int valves)
{
  itsEngine.setValveCount(valves);
}


void MotorCycle::print (std::ostream& os) const 
{
  os << "Motor Cycle:" << std::endl;
  Vehicle::print (os);
  itsEngine.print (os);
}


void Tractor::print (std::ostream& os) const 
{
  os << "Tractor:" << std::endl;
  Vehicle::print (os);
  itsEngine.print (os);
}

