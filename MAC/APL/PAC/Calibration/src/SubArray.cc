//#  -*- mode: c++ -*-
//#  SubArray.cc: implementation of the SubArray class
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

#include "SubArray.h"
#include "CalibrationInterface.h"

using namespace CAL;
using namespace std;
using namespace blitz;
    
SubArray::SubArray(string                 name,
		   const Array<double,3>& pos,
		   const Array<bool, 2>&  select,
		   const SpectralWindow&  spw) : AntennaArray(name, pos), m_spw(spw)
{
  // make array at least big enough
  m_rcuindex.resize(pos.extent(firstDim), pos.extent(secondDim));

  for (int i = 0; i < pos.extent(firstDim); i++)
    {
      for (int pol = 0; pol < pos.extent(secondDim); pol++)
	{
	  if (select(i,pol)) {
	    m_rcuindex(i,pol) = (i * pos.extent(secondDim)) + pol;
	  } else {
	    m_rcuindex(i,pol) = -1;
	  }
	}
    }

  // TODO: compact array by removing antennas of which both polarizations have not been selected

  // create calibration result objects
  m_result[FRONT] = new CalibrationResult(pos.extent(firstDim), spw.getNumSubbands());
  m_result[BACK]  = new CalibrationResult(pos.extent(firstDim), spw.getNumSubbands());
}

SubArray::~SubArray()
{}

void SubArray::startCalibration(CalibrationInterface* cal
				/*, const ACC& acc*/)
{
  if (!cal) return;

  cal->calibrate(*this, *m_result[BACK]);
}

bool SubArray::getCalibration(const CalibrationResult* cal, int buffer)
{
  cal = 0;

  if (buffer >= FRONT && buffer <= BACK
      && m_result[buffer] && m_result[buffer]->isComplete())
    {
      cal = m_result[buffer];
      return true;
    }

  return false;
}

void SubArray::abortCalibration()
{}

const SpectralWindow& SubArray::getSPW() const
{
  return m_spw;
}
