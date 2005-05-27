//#  -*- mode: c++ -*-
//#  AntennaArray.cc: implementation of the AntennaArray class.
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

#include "AntennaArray.h"

#include <blitz/array.h>

using namespace CAL;
using namespace std;
using namespace blitz;

AntennaArray::AntennaArray(string                  name,
			   const Array<double, 3>& pos,
			   const Array<int, 2>*    rcuindex) 
  : m_name(name), m_pos(pos), m_rcuindex(0)
{
  m_rcuindex.resize(m_pos.extent(firstDim), m_pos.extent(secondDim));

  if (rcuindex)
    {
      m_rcuindex = *rcuindex;
    }
  else
    {
      firstIndex i;
      m_rcuindex = i;
    }
}

AntennaArray::~AntennaArray()
{
}

AntennaArray* AntennaArrayLoader::loadFromBlitzString(std::string name, std::string array)
{
  Array<double, 3> positions;
  istringstream arraystream(array);

  arraystream >> positions;

  return new AntennaArray(name, positions);
}
