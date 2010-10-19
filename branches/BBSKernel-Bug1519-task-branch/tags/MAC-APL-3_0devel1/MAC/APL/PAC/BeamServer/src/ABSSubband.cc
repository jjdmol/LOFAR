//#  ABSSubband.h: implementation of the Subband class
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

#include <ABSSubband.h>

#include <iostream>

using namespace ABS;
using namespace std;

int      Subband::m_ninstances = 0;
Subband* Subband::m_subbands   = 0;

Subband::Subband()
{}

Subband::~Subband()
{
  if (m_subbands) delete [] m_subbands;
  m_ninstances = 0;
}

Subband* Subband::getInstance(int subband)
{
  // if not yet initialised, just return 0
  if (!m_subbands) return 0;

  // only return beam if subband is a valid index
  if (subband >= 0 && subband < m_ninstances)
  {
      // initialize center frequency of the subband
//      spw.getFrequency(m_subbands[subband].m_centerfreq, subband);
      return &m_subbands[subband];
  }

  // otherwise return 0
  return 0;
}

int Subband::setNInstances(int nspws,
			   int ninstances)
{
  // If already initialised just return
  // failure.
  // Only one initialisation is allowed.
  if (m_subbands) return -1;

  m_ninstances = ninstances;
  m_subbands = new Subband[nspws][m_ninstances];

  if (!m_subbands) return -1;

  // set index for the subbands
  for (int i = 0; i < m_nswps; i++)
  {
      for (int j = 0; j < m_ninstances; j++)
      {
	  m_subbands[i][j].m_index = j;
      }
  }

  return 0;
}
