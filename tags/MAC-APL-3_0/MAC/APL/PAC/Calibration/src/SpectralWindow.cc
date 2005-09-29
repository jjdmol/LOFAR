//#  -*- mode: c++ -*-
//#  SpectralWindow.cc: implementation of the SpectralWindow class
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

#include "SpectralWindow.h"

#include <blitz/array.h>
#include <sstream>

using namespace CAL;
using namespace std;
using namespace blitz;

#define SPW_DIMS    2
#define SPW_NPARAMS 3 /* sampling_freq, nyquist_zone, numsubbands */

SpectralWindow::SpectralWindow() :
  m_sampling_freq(0), m_nyquist_zone(0), m_numsubbands(0)
{
}

SpectralWindow::~SpectralWindow()
{
}

double SpectralWindow::getSubbandFreq(int subband, int pos) const
{
  double freq = -1.0; // invalid

  if (subband < 0) return freq; // invalid index
  
  switch(pos)
  {
    case LOW:
      freq = (subband % m_numsubbands) * getSubbandWidth();
      break;

    case CENTER:
      freq = (((subband + 1) % m_numsubbands) - 0.5) * getSubbandWidth();
      break;

    case HIGH:
      freq = ((subband + 1) % m_numsubbands) * getSubbandWidth();
      break;
  }

  return freq;
}

vector<SpectralWindow> SPWLoader::loadFromBlitzStrings(string params, string names)
{
  vector<SpectralWindow> spws;
  Array<double, SPW_DIMS> paramsArray;
  Array<string, 1>        namesArray;
  
  istringstream streamparams(params);
  istringstream streamnames(names);
  
  streamparams >> paramsArray;
  streamnames  >> namesArray;

  // check for correct extent
  if (paramsArray.extent(secondDim) != SPW_NPARAMS) return spws;
  if (paramsArray.extent(firstDim) != namesArray.extent(firstDim)) return spws;

  for (int i = 0; i < paramsArray.extent(firstDim); i++)
  {
    double sampling_freq = paramsArray(i, 0);
    int    nyquist_zone  = int(floor(paramsArray(i, 1)));
    int    numsubbands   = int(floor(paramsArray(i, 2)));
    
    spws.push_back(SpectralWindow(namesArray(i), sampling_freq, nyquist_zone, numsubbands));
  }
  
  return spws;
}


