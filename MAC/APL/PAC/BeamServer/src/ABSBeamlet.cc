//#  ABSBeamlet.h: implementation of the Beamlet class
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

#include <ABSBeamlet.h>
#include <ABSBeam.h>

#include <iostream>
#include <queue>

#include <blitz/array.h>
using namespace blitz;

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>
using namespace LOFAR;

using namespace ABS;
using namespace std;

int      Beamlet::m_ninstances = 0;
Beamlet* Beamlet::m_beamlets   = 0;

Beamlet::Beamlet() :
    m_spw(0), m_subband(0), m_index(-1), m_beam(0)
{}

Beamlet::~Beamlet()
{
  if (m_beamlets) delete [] m_beamlets;
  m_ninstances = 0;
}

Beamlet* Beamlet::getInstance()
{
  // if not yet initialised, just return 0
  if (!m_beamlets) return 0;

  for (int i = 0; i < m_ninstances; i++)
  {
      if (!m_beamlets[i].allocated())
	  return &m_beamlets[i];
  }

  // otherwise return 0
  return 0;
}

int Beamlet::init(int ninstances)
{
  // if already initialised just return
  // only one initialisation is allowed
  if (m_beamlets) return -1;

  m_beamlets = new Beamlet[ninstances];
  if (!m_beamlets) return -1;
  m_ninstances = ninstances;

  for (int i = 0; i < m_ninstances; i++)
  {
      m_beamlets[i].m_index = i;
  }

  return (m_beamlets ? 0 : -1);
}

int Beamlet::allocate(const Beam& beam,
		      SpectralWindow const& spw, int subband)
{
  // don't allow second allocation
  if (m_beam) return -1;

  // check that the subband is within the spectral window
  if ((subband >= spw.nsubbands()) 
      || (subband < 0) ) return -1;

  m_spw     = &spw;
  m_subband = subband;
  m_beam = &beam;

  return 0;
}

int Beamlet::deallocate()
{
  if (!m_beam) return -1;

  m_spw     = 0;
  m_subband = -1;
  m_beam    = 0;

  return 0;
}

const Beam* Beamlet::getBeam() const
{
  return m_beam;
}

void Beamlet::calculate_weights()
{
  priority_queue<Pointing> coords;
  const Array<double,2>* track = 0;

  for (int i = 0; i < m_ninstances; i++)
  {
      Beamlet* beamlet = &m_beamlets[i];
      if (beamlet->allocated())
      {
	  const Beam* beam = beamlet->getBeam();

	  // get coordinates from beam
	  if (beam) track = &beam->getCoordinates();
      }
  }

  // calculating weights for the following coordinates
  LOG_INFO(formatString("Calculating weights for %d beamlets: %d coordinates.",
			m_ninstances, coords.size()));
  if (track)
      LOG_INFO(formatString("Storing in Array<double,2> track(%d,%d)",
			    track->extent(firstDim),
			    track->extent(secondDim)));
}

