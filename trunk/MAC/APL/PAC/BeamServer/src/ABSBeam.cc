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

#include <math.h>
#include <iostream>
#include <sys/time.h>
#include <queue>

#include <blitz/array.h>
using namespace blitz;

#include <boost/date_time/posix_time/posix_time.hpp>
using namespace boost::gregorian;
using namespace boost::posix_time;

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>
using namespace LOFAR;

using namespace ABS;
using namespace std;

int            Beam::m_ninstances       = 0;
Beam*          Beam::m_beams            = 0;
unsigned short Beam::m_update_interval  = 0;
unsigned short Beam::m_compute_interval = 0;

Beam::Beam() :
  m_allocated(false), m_pointing(), m_index(-1) 
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

int Beam::init(int            ninstances,
	       unsigned short update_interval,
	       unsigned short compute_interval)
{
  // if already initialised just return
  // only one initialisation is allowed
  if (m_beams) return -1;

  m_beams = new Beam[ninstances];
  if (!m_beams) return -1;
  m_ninstances = ninstances;

  for (int i = 0; i < m_ninstances; i++)
  {
      // inplace constructor
      m_beams[i].m_index = i; // assign index
      m_beams[i].m_azels.resize(compute_interval,2);
      m_beams[i].m_lmns.resize(compute_interval,3);
  }

  m_update_interval = update_interval;
  m_compute_interval = compute_interval;

  return 0;
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

int Beam::convertPointings(time_period period)
{
  // remember last pointing from previous call
  Pointing current_pointing = m_pointing;

  bool pset[m_compute_interval];
  for (int i = 0; i < m_compute_interval; i++) pset[i] = false;

  // only works on allocated beams
  if (!allocated()) return -1;

  if (period.length().seconds() != m_compute_interval)
  {
    LOG_ERROR("\ninvalid time period\n");
    return -1;
  }

  // reset azel, lm arrays
  m_azels = 0.0;
  m_lmns = 0.0;

  LOG_DEBUG_STR("Period=" << to_simple_string(period));

  while (!m_pointing_queue.empty())
  {
      Pointing pointing = m_pointing_queue.top();

      LOG_DEBUG_STR("Pointing time=" << to_simple_string(pointing.time()));

      if (pointing.direction().type() != Direction::LOFAR_LMN)
      {
	m_pointing_queue.pop(); // discard pointing
	LOG_ERROR("\nDirection type not supported yet, pointing discarded.\n");
	continue;
      }

      if (pointing.time() < period.begin())
      {
	LOG_WARN(formatString("Deadline missed by %d seconds(%f,%f) @ %s",
			      (period.begin() - pointing.time()).seconds(),
			      pointing.direction().angle1(),
			      pointing.direction().angle2(),
			      to_simple_string(pointing.time()).c_str()));

	m_pointing_queue.pop(); // discard pointing
      }
      else if (pointing.time() < period.end())
      {
	  m_pointing_queue.pop(); // remove from queue

	  //
	  // convert direction
	  // INSERT COORDINATE CONVERSION CALL
	  // converts from Direction -> AZEL
	  //

	  //
	  // insert converted LM coordinate at correct 
	  // position in the m_lmns array
	  //
	  //time_duration t = pointing.time() - period.begin();
	  register int tsec = (pointing.time()-period.begin()).seconds();

	  if ( (tsec < 0) || (tsec >= m_compute_interval) )
	  {
	    LOG_ERROR_STR("\ninvalid pointing time\n" << to_simple_string(pointing.time()));
	    continue;
	  }

	  
	  m_lmns(tsec,0) = pointing.direction().angle1();
	  m_lmns(tsec,1) = pointing.direction().angle2();
	  m_lmns(tsec,2) = sqrt(1.0 - ((m_lmns(tsec,0)*m_lmns(tsec,0))
				       + (m_lmns(tsec,1)*m_lmns(tsec,1))));
	  pset[tsec] = true;

	  m_pointing = pointing; // remember current pointing
      }
      else
      {
	// done with this period
	break;
      }
  }
  
  //
  // m_lmns array will have unset values because
  // there are not necessarily pointings at each second
  // these wholes are fixed up in this loop
  //
  time_iterator thetime(period.begin(), seconds(m_update_interval));
  for (int t=0; t < m_compute_interval; ++t, ++thetime)
  {
    if (!pset[t])
    {
      m_lmns(t,0) = current_pointing.direction().angle1();
      m_lmns(t,1) = current_pointing.direction().angle2();
      m_lmns(t,2) = sqrt(1.0 - ((m_lmns(t,0) * m_lmns(t,0))
			      + (m_lmns(t,1) * m_lmns(t,1))));
    }
    else
    {
      double a1 = m_lmns(t,0);
      double a2 = m_lmns(t,1);
      current_pointing.setDirection(Direction(a1,a2));
    }

    if ((fabs(m_lmns(t,0)) > 1.0)
	|| (fabs(m_lmns(t,1)) > 1.0))
    {
	LOG_ERROR("\nl or m coordinate out of range -1.0 < l < 1.0, setting to (l,m) to (0.0, 0.0)\n");
	m_lmns(t,0) = 0.0;
	m_lmns(t,1) = 0.0;
	m_lmns(t,2) = 0.0;
    }

    LOG_TRACE_VAR(formatString("direction@%s=(%f,%f,%f)",
			       to_simple_string(*thetime).c_str(),
			       m_lmns(t,0), m_lmns(t,1), m_lmns(t,2)));
  }

  return 0;
}

const Array<W_TYPE,2>& Beam::getLMNCoordinates() const
{
  return m_lmns;
}

void Beam::getSubbandSelection(map<int,int>& selection) const
{
  for (set<Beamlet*>::iterator bl = m_beamlets.begin();
       bl != m_beamlets.end(); ++bl)
  {
      selection[(*bl)->index()] = (*bl)->subband();
  }
}
