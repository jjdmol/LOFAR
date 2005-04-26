//#  -*- mode: c++ -*-
//#  ACC.cc: implementation of the Auto Correlation Cube class
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

#include "ACC.h"
#include <blitz/array.h>
#include <fstream>

using namespace CAL;
using namespace blitz;
using namespace LOFAR::RSP_Protocol;
using namespace std;

ACC::ACC(Array<complex<double>, 5>& acc) : m_acc(acc), m_time(acc.extent(thirdDim))
{
}

ACC::~ACC()
{
}

const Array<complex<double>, 4> ACC::getACM(int subband, Timestamp& timestamp) const
{
  timestamp = Timestamp(0,0);

  // check range of subband argument
  if (subband < 0 || subband > m_acc.extent(thirdDim)) return Array<complex<double>,4>();

  timestamp = m_time(subband);

#if 0
  Array<complex<double>, 4> slice(m_acc(Range::all(), Range::all(),
					subband, Range::all(), Range::all()));
#endif

  return m_acc(Range::all(), Range::all(),
	       subband, Range::all(), Range::all());
}

const ACC* ACCLoader::loadFromFile(string filename)
{
  ACC* acc = 0;
  Array<complex<double>, 5> acc_array;

  ifstream accstream(filename.c_str());

  if (accstream.is_open())
    {
      accstream >> acc_array;
      acc = new ACC(acc_array);
    }

  return acc;
}



