//#  ABSBeam.h: implementation of the Beam class
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

#include <ABSBeam.h>

#include <iostream>

using namespace ABS;
using namespace std;

int   Beam::m_ninstances = 0;
Beam* Beam::m_beams = 0;

Beam::Beam() : m_allocated(false)
{}

Beam::~Beam()
{
  if (m_beams) delete [] m_beams;
  m_ninstances = 0;
}

Beam* Beam::getInstance(int beam)
{
  // if not yet initialised, just return 0
  if (!m_beams) return 0;

  // only return beam if beam is a valid index
  if (beam >= 0 && beam < m_ninstances)
  {
      return &m_beams[beam];
  }

  // otherwise return 0
  return 0;
}

int Beam::setNInstances(int ninstances)
{
  // if already initialised just return
  // only one initialisation is allowed
  if (m_beams) return -1;

  m_ninstances = ninstances;
  m_beams = new Beam[m_ninstances];

  return (m_beams ? 0 : -1);
}

int Beam::allocate(SpectralWindow const& spw, set<int> subbands)
{
  // cannot allcate an already allocated beam
  if (m_allocated) return -1;
  
  // clear the beamlet set just to be sure
  m_beamlets.clear();

  // obtain enough beamlet instances
  for (set<int>::iterator sb = subbands.begin(); sb != subbands.end(); ++sb)
  {
      Beamlet* beamlet = Beamlet::getInstance();

      if (!beamlet) goto failure;

      if (beamlet->allocate(spw, *sb) < 0) goto failure;

      m_beamlets.insert(beamlet);
  }

  m_allocated = true;

  return 0;

 failure:
  // cleanup on failure
  (void)deallocate();

  return -1;
}

int Beam::deallocate()
{
  if (!m_allocated) return -1;

  for (set<Beamlet*>::iterator bl = m_beamlets.begin();
       bl != m_beamlets.end(); ++bl)
  {
      (*bl)->deallocate();
  }
  m_beamlets.clear();

  m_allocated = false;

  return 0;
}

int Beam::addPointing()
{ return 0; }

int Beam::convertPointings()
{ return 0; }

int Beam::getPointings()
{ return 0; }

void Beam::getSubbandSelection(map<int,int>& selection) const
{
  for (set<Beamlet*>::iterator bl = m_beamlets.begin();
       bl != m_beamlets.end(); ++bl)
  {
      selection[(*bl)->index()] = (*bl)->subband();
  }
}
