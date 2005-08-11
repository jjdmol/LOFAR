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

#include "Beam.h"

#include <math.h>
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

using namespace BS;
using namespace BS_Protocol;
using namespace std;
using namespace RTC;

Beam::Beam(double sampling_frequency, int nyquist_zone, int nsubbands) :
  m_spw("BeamSPW", sampling_frequency, nyquist_zone, nsubbands)
{}

Beam::~Beam()
{
}

bool Beam::allocate(Beamlet2SubbandMap allocation, Beamlets& beamlets)
{
  map<uint16,uint16>::iterator it;

  // clear the beamlet set just to be sure
  m_beamlets.clear();

  for (it = allocation().begin(); it != allocation().end(); ++it) {
    Beamlet* beamlet = beamlets.get((int)it->first);

    if (!beamlet) goto failure;

    m_beamlets.insert(beamlet);
    if (beamlet->allocate(*this, m_spw, it->second) < 0) goto failure;
  }

  return true;

 failure:
  //
  // cleanup on failure
  //
  deallocate();

  return false;
}

bool Beam::modify(BS_Protocol::Beamlet2SubbandMap allocation)
{
  // check for equal size
  if (allocation().size() != m_allocation().size()) return false;
  
  // check that keys match
  map<uint16,uint16>::iterator it1;
  map<uint16,uint16>::iterator it2;
  for (it1 = allocation().begin(), it2 = m_allocation().begin();
       it1 != allocation().end(); ++it1, ++it2) {
    if ((*it1).first != (*it2).first) return false;
  }

  // all ok, replace m_allocation by allocation
  m_allocation = allocation;

  return true;
}

Beamlet2SubbandMap Beam::getAllocation() const
{
  return m_allocation;
}

void Beam::deallocate()
{
  // release beamlets
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
}

void Beam::addPointing(const Pointing& pointing)
{
  m_pointing_queue.push(pointing);
}

int Beam::convertPointings(RTC::Timestamp begintime, int compute_interval)
{
  // remember last pointing from previous call
  Pointing current_pointing = m_pointing;

  bool pset[compute_interval];
  for (int i = 0; i < compute_interval; i++) pset[i] = false;

  // reset azels array
  m_azels.resize(compute_interval, 2);
  m_azels = 0.0;

  // reset lmns array
  m_lmns.resize(compute_interval, 3);
  m_lmns = 0.0;

  LOG_DEBUG_STR("Begintime=" << begintime);

  while (!m_pointing_queue.empty())
  {
      Pointing pointing = m_pointing_queue.top();

      LOG_DEBUG_STR("Pointing time=" << pointing.time());

      if (pointing.getType() != Pointing::LOFAR_LMN)
      {
	m_pointing_queue.pop(); // discard pointing
	LOG_ERROR("\nDirection type not supported yet, pointing discarded.\n");
	continue;
      }

      if (pointing.time() < begintime)
      {
	//
	// Deadline missed, print warning but execute the command anyway
	// as soon as possible.
	//
	LOG_WARN_STR("Deadline missed by " << (begintime.sec() - pointing.time().sec())
		     << " seconds (" << pointing.angle1() << "," << pointing.angle2() << ") "
		     << "@ " << pointing.time());

	pointing.setTime(begintime);
      }

#if 0
	m_pointing_queue.pop(); // discard pointing
      }
      else
#endif
      
      if (pointing.time() < begintime + compute_interval)
      {
	  m_pointing_queue.pop(); // remove from queue

	  //
	  // convert direction
	  // ***** INSERT COORDINATE CONVERSION CALL *****
	  // converts from Direction -> AZEL
	  //

	  // convert from AZEL -> LOFAR_LMN

	  //
	  // insert converted LM coordinate at correct 
	  // position in the m_lmns array
	  //
	  register int tsec = pointing.time().sec() - begintime.sec();

	  if ( (tsec < 0) || (tsec >= compute_interval) )
	  {
	    LOG_ERROR_STR("\ninvalid pointing time" << pointing.time());
	    continue;
	  }
	  
	  m_lmns(tsec,0) = pointing.angle1();
	  m_lmns(tsec,1) = pointing.angle2();
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
  // these holes are fixed up in this loop
  //
  for (int t=0; t < compute_interval; ++t)
  {
    if (!pset[t])
    {
      m_lmns(t,0) = current_pointing.angle1();
      m_lmns(t,1) = current_pointing.angle2();
      m_lmns(t,2) = sqrt(1.0 - ((m_lmns(t,0) * m_lmns(t,0))
			      + (m_lmns(t,1) * m_lmns(t,1))));
    }
    else
    {
      double a1 = m_lmns(t,0);
      double a2 = m_lmns(t,1);
      current_pointing.setDirection(a1, a2);
    }

    if ((fabs(m_lmns(t,0)) > 1.0)
	|| (fabs(m_lmns(t,1)) > 1.0))
    {
	LOG_ERROR("\nl or m coordinate out of range -1.0 < l < 1.0, setting to (l,m) to (0.0, 0.0)\n");
	m_lmns(t,0) = 0.0;
	m_lmns(t,1) = 0.0;
	m_lmns(t,2) = 0.0;
    }

    LOG_TRACE_VAR(formatString("direction@%d=(%f,%f,%f)",
			       begintime.sec() + t,
			       m_lmns(t,0), m_lmns(t,1), m_lmns(t,2)));
  }

  return 0;
}

const Array<W_TYPE,2>& Beam::getLMNCoordinates() const
{
  return m_lmns;
}

Beams::Beams(int nbeamlets) : m_beamlets(nbeamlets)
{
}

Beam* Beams::get(Beamlet2SubbandMap allocation,
		 double sampling_frequency, int nyquist_zone)
{
  Beam* beam = new Beam(sampling_frequency, nyquist_zone, allocation().size());

  if (beam) {

    if (!beam->allocate(allocation, m_beamlets)) {
      delete beam;
      beam = 0;
    } else {
      // add to list of beams
      m_beams.push_back(beam);
    }
  }
  return beam;
}

Beam* Beams::get(uint32 handle)
{
  Beam* beam = (Beam*)handle;

  // if beam not found, return 0
  list<Beam*>::const_iterator it = find(m_beams.begin(),
					m_beams.end(),
					beam);
  if (it == m_beams.end()) {
    beam = 0;
  }
  
  return beam;
}

bool Beams::destroy(uint32 handle)
{
  Beam* beam = (Beam*)handle;

  // if beam not found, return 0
  list<Beam*>::iterator it = find(m_beams.begin(),
				  m_beams.end(),
				  beam);
  if (it != m_beams.end()) {
    delete beam;
    m_beams.erase(it);
    return true;
  }

  return false;
}

void Beams::calculate_weights(Timestamp timestamp,
			      int compute_interval,
			      const blitz::Array<W_TYPE, 3>&         pos,
			      blitz::Array<std::complex<W_TYPE>, 3>& weights)
{
  // iterate over all beams
  for (list<Beam*>::iterator bi = m_beams.begin(); bi != m_beams.end(); ++bi)
  {
    (*bi)->convertPointings(timestamp, compute_interval);
    LOG_INFO(formatString("current_pointing=(%f,%f)",
			  (*bi)->getPointing().angle1(),
			  (*bi)->getPointing().angle2()));
  }

  m_beamlets.calculate_weights(pos, weights);
}

Beamlet2SubbandMap Beams::getSubbandSelection()
{
  Beamlet2SubbandMap selection;
  for (list<Beam*>::iterator bi = m_beams.begin(); bi != m_beams.end(); ++bi)
  {
    Beamlet2SubbandMap beammap = (*bi)->getAllocation();
    selection().insert(beammap().begin(), beammap().end());
  }

  return selection;
}
