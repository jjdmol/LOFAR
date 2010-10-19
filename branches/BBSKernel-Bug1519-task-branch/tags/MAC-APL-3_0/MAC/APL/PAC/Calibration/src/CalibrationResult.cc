//#  -*- mode: c++ -*-
//#  CalibrationResult.cc: implementation of the CalibrationResult class.
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

#include "CalibrationResult.h"

using namespace CAL;
using namespace std;
using namespace blitz;

CalibrationResult::CalibrationResult(int nantennas, int nsubbands) : m_complete(false)
{
  if (nantennas < 0 || nsubbands < 0)
    {
      nantennas = 0;
      nsubbands = 0;
    }
   
  m_gains.resize(nantennas, 2, nsubbands);
  m_gains = 1;

  m_quality.resize(nsubbands);
  m_quality = 1;
}

CalibrationResult::~CalibrationResult()
{}

