//  Sel_RoundRobin.cc:
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
//////////////////////////////////////////////////////////////////////


#include "tinyCEP/Sel_RoundRobin.h"

namespace LOFAR
{

Sel_RoundRobin::Sel_RoundRobin(unsigned int noOptions)
  :  Selector(noOptions)
{}

Sel_RoundRobin::~Sel_RoundRobin()
{}

Sel_RoundRobin::Sel_RoundRobin(const Sel_RoundRobin& that)
  : Selector(that)
{}

Selector* Sel_RoundRobin::clone() const
{
  return new Sel_RoundRobin(*this);
}

unsigned int Sel_RoundRobin::getNext()
{
  itsCurrentSelection++;
  if (itsCurrentSelection >= itsNOptions)
  {
    itsCurrentSelection = 0;
  }
  return itsCurrentSelection;
}

}
