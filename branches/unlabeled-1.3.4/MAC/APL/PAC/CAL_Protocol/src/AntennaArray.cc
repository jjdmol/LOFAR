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
#include "AntennaArrayData.h"

#include <blitz/array.h>
#include <fstream>

using namespace std;
using namespace blitz;
using namespace LOFAR;
using namespace CAL;

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

AntennaArrays::AntennaArrays()
{
}

AntennaArrays::~AntennaArrays()
{
  for (map<string, const AntennaArray*>::const_iterator it = m_arrays.begin();
       it != m_arrays.end(); it++)
  {
    if ((*it).second) delete (*it).second;
  }
}

void AntennaArrays::getAll(std::string url)
{
  AntennaArrayData arraydata;

  while (arraydata.getNextFromFile(url)) {
    AntennaArray* newarray = new AntennaArray(arraydata.getName(),
					      arraydata.getPositions());
    m_arrays[arraydata.getName()] = newarray;
  }
}
