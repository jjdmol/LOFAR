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

#include "ABSBeam.h"

#include <iostream>
#include <sys/time.h>
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

int   Beam::m_ninstances = 0;
Beam* Beam::m_beams = 0;
//static const int timestep = Beam::TIMESTEP;
//static const int _n_timesteps_var = Beam::N_TIMESTEPS;

Beam::Beam() : m_allocated(false), m_pointing(), m_track(2, N_TIMESTEPS)
{}

Beam::~Beam()
{
  if (m_beams) delete [] m_beams;
  m_ninstances = 0;
}

Beam* Beam::getInstance()
{
  // if not yet initialised, just return 0
  if (!m_beams) return 0;
  
  for (int i = 0; i < m_ninstances; i++)
  {
      if (!m_beams[i].allocated())
	  return &m_beams[i];
  }

  // otherwise return 0
  return 0;
}

Beam* Beam::getFromHandle(int handle)
{
  // if invalid handle return 0
  if (!m_beams
      || handle < 0
      || handle > m_ninstances
      || !m_beams[handle].allocated()) return 0;

  return &m_beams[handle];
}

int Beam::setNInstances(int ninstances)
{
  // if already initialised just return
  // only one initialisation is allowed
  if (m_beams) return -1;

  m_beams = new Beam[ninstances];
  if (!m_beams) return -1;
  m_ninstances = ninstances;

  for (int i = 0; i < m_ninstances; i++)
  {
      m_beams[i].m_index = i; // assign index
  }

  return (m_beams ? 0 : -1);
}

Beam* Beam::allocate(SpectralWindow const& spw, set<int> subbands)
{
  // get a free beam instance
  Beam* beam = Beam::getInstance();

  if (!beam) return 0;
  
  // clear the beamlet set just to be sure
  beam->m_beamlets.clear();

  // obtain enough beamlet instances
  for (set<int>::iterator sb = subbands.begin();
       sb != subbands.end(); ++sb)
  {
      Beamlet* beamlet = Beamlet::getInstance();

      if (!beamlet) goto failure;
      if (beamlet->allocate(*beam, spw, *sb) < 0) goto failure;

      beam->m_beamlets.insert(beamlet);
  }
  
  beam->m_allocated = true;

  return beam;

 failure:
  //
  // cleanup on failure
  // need to set allocated to true
  // to prevent failure of deallocate
  // will be reset to false in deallocate
  //
  beam->m_allocated = true;
  (void)beam->deallocate();

  return 0;
}

int Beam::deallocate()
{
  if (!m_allocated) return -1;

  // deallocate all beamlets
  for (set<Beamlet*>::iterator bl = m_beamlets.begin();
       bl != m_beamlets.end(); ++bl)
  {
      (*bl)->deallocate();
  }
  m_beamlets.clear();

  // reset pointing to zenith
  m_pointing = Pointing();

  // clear the pointing queue
  while (!m_pointing_queue.empty()) m_pointing_queue.pop();

  m_allocated = false;

  return 0;
}

int Beam::addPointing(const Pointing& pointing)
{
  if (!m_allocated) return -1;

  m_pointing_queue.push(pointing);
  return 0;
}

int Beam::convertPointings(struct timeval fromtime,
			   unsigned long duration)
//			   int timestep, // in seconds
//			   int ntimesteps)
{
  // only works on allocated beams
  if (!allocated()) return -1;

  Pointing from(Direction(0.0,0.0,Direction::LOFAR_LMN), fromtime);
  struct timeval totime = fromtime;
  totime.tv_sec += duration;
  Pointing deadline(Direction(0.0,0.0,Direction::LOFAR_LMN), totime);

  // clear previous coordinate track, assumed to be empty
  if (!m_coord_track.empty())
  {
      LOG_WARN(formatString("Coordinate track not empty; %d items left!",
			    m_coord_track.size()));
      while (!m_coord_track.empty()) m_coord_track.pop();
  }

  if (m_pointing_queue.empty())
  {
      // make sure there is always at least one pointing
      // on the m_coord_track queue to drive the weights calculation.

      //
      // convert direction
      // INSERT COORDINATE CONVERSION CALL
      //
      m_coord_track.push(m_pointing);
  }
  else
  {
      do
      {
	  Pointing pointing = m_pointing_queue.top();
	  if (pointing < from)
	  {
	      // discard
	      m_pointing_queue.pop();

	      LOG_WARN(formatString("Deadline missed for pointing (%f,%f)",
				    pointing.direction().angle1(),
				    pointing.direction().angle2()));
	  }
	  else if (pointing < deadline)
	  {
	      if (pointing.direction().type() != Direction::LOFAR_LMN)
	      {
		  LOG_ERROR("Direction type not supported yet, pointing discarded.");
	      }
	      else
	      {
		  //
		  // convert direction
		  // INSERT COORDINATE CONVERSION CALL
		  //
		  Pointing p = pointing;
		  for (unsigned long sec = 0; sec < duration; sec++)
		  {
		      p.time().tv_sec += sec;
		      m_coord_track.push(p);
		  }
	      }

	      m_pointing_queue.pop();
	  }
	  else
	  {
	      // the current pointing should be updated
	      m_pointing = pointing;

	      break; // done
	  }
      } while (!m_pointing_queue.empty());
  }
  
  return 0;
}

const Array<double,2>& Beam::getCoordinates() const
{
  return m_track;
}

void Beam::getSubbandSelection(map<int,int>& selection) const
{
  for (set<Beamlet*>::iterator bl = m_beamlets.begin();
       bl != m_beamlets.end(); ++bl)
  {
      selection[(*bl)->index()] = (*bl)->subband();
  }
}
