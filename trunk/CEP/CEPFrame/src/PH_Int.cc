//#  PH_Int.cc: A simple ParamHolder containing an integer
//#
//#  Copyright (C) 2002-2003
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
////////////////////////////////////////////////////////////////////////////

#include "CEPFrame/PH_Int.h"

namespace LOFAR
{

PH_Int::PH_Int(const string& name)
  : ParamHolder(name, "PH_Int"),
    itsParamPacket(0)
{
}

PH_Int::~PH_Int()
{
  delete [] (char*)(itsParamPacket);
}

PH_Int::PH_Int(const PH_Int& that)
  : ParamHolder(that),
    itsParamPacket(0)
{
}

ParamHolder* PH_Int::clone() const
{
  return new PH_Int(*this);
}

void PH_Int::preprocess()
{
  // Determine the number of bytes needed for DataPacket and buffer.
  unsigned int size = sizeof(ParamPacket);
  char* ptr = new char[size];
  // Fill in the data packet pointer.
  itsParamPacket = (ParamPacket*)(ptr);
  *itsParamPacket = ParamPacket();
  // Initialize value
  itsParamPacket->itsValue = 0;
  // Initialize base class.
  setParamPacket(itsParamPacket, size);
}

}
