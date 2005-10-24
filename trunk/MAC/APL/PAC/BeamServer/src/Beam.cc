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

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <AMCBase/Converter.h>
#include <AMCBase/SkyCoord.h>
#include <AMCBase/EarthCoord.h>
#include <AMCBase/TimeCoord.h>

#include <APL/RTCCommon/PSAccess.h>
#include "Beam.h"

#include <math.h>
#include <iostream>
#include <sys/time.h>
#include <queue>

#include <blitz/array.h>

#include <fcntl.h>

using namespace blitz;
using namespace LOFAR;
using namespace AMC;
using namespace BS;
using namespace BS_Protocol;
using namespace std;
using namespace RTC;

Beam::Beam(string name, string subarrayname, int nsubbands, EarthCoord pos) :
  m_name(name), m_subarrayname(subarrayname), m_nsubbands(nsubbands), m_pos(pos)
{}

Beam::~Beam()
{
  deallocate();

  // TODO: free beamlets
}

bool Beam::allocate(Beamlet2SubbandMap allocation, Beamlets& beamlets, int nsubbands)
{
  if (   0 == allocation().size()
      || allocation().size() > (unsigned)nsubbands) return false;

  // set allocation
  m_allocation = allocation;

  // clear the beamlet set just to be sure
  m_beamlets.clear();

  for (map<uint16,uint16>::iterator it = m_allocation().begin();
       it != m_allocation().end(); ++it) {

    Beamlet* beamlet = beamlets.get((int)it->first);

    if (!beamlet) goto failure;

    m_beamlets.insert(beamlet);
    if (beamlet->allocate(*this, it->second, nsubbands) < 0) goto failure;
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

  // clear allocation
  m_allocation().clear();
}

void Beam::addPointing(const Pointing& pointing)
{
  m_pointing_queue.push(pointing);
}

void Beam::setSubarray(const CAL::SubArray& array)
{
  m_array = array;
}

void Beam::setCalibration(const CAL::AntennaGains& gains)
{
  m_gains = gains;
}

const CAL::AntennaGains& Beam::getCalibration() const
{
  return m_gains;
}

static int setNonblocking(int fd)
{
  int flags;

  /* If they have O_NONBLOCK, use the Posix way to do it */
#if defined(O_NONBLOCK)
  /* Fixme: O_NONBLOCK is defined but broken on SunOS 4.1.x and AIX 3.2.5. */
  if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
    flags = 0;
  return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
  /* Otherwise, use the old way of doing it */
  flags = 1;
  return ioctl(fd, FIOBIO, &flags);
#endif
}     

void Beam::logPointing(Pointing pointing)
{
  if (GET_CONFIG("BeamServer.DISABLE_INDI", i)) return;

  static FILE* pipe = 0;
  double hh, mm, ss, deg, degmm, degss;
  static string pipename = GET_CONFIG_PATH() + "/indi_pipe";
  int retry = 0;

  /**
   * Write RaDec coordinates to the named pipe.
   * If there is no-one listening on the named pipe it
   * will eventually block which we prevent by making the filedescriptor
   * non-blocking.
   * When a write (fprintf) to the named-pipe fails, assume that the pipe
   * has been closed by the other end and re-open it and write the value again.
   */

  do {
    /* open pipe if not already open */
    if (!pipe) {
      pipe = fopen(pipename.c_str(), "w+");
      if (!pipe) {
	LOG_WARN_STR("Could not open '" << GET_CONFIG_PATH() + "/indi_pipe");
	return;
      }
      setNonblocking(fileno(pipe));
    }

    hh = (pointing.angle0() * 180.0 / M_PI) / 15.0;
    mm = (hh - floor(hh)) * 60;
    ss = (mm - floor(mm)) * 60;
    hh = floor(hh);
    mm = floor(mm);
    
    deg = pointing.angle1() * 180.0 / M_PI;
    degmm = (deg - floor(deg)) * 60;
    degss = (degmm - floor(degmm)) * 60;
    deg = floor(deg);
    degmm = floor(degmm);
    if (deg > 90.0) deg = 360.0 - deg;

    /*
     * If failed to write, closed the pipe and retry by
     * re-opening the pipe (at the beginning of the while loop.
     */
    if (fprintf(pipe, "%s %d %lf %lf\n",
		getName().c_str(), 0, pointing.angle0(), pointing.angle1()) < 0) {
      fclose(pipe);
      pipe = 0;
    }
  }
  while (retry++ < 5);

  LOG_DEBUG_STR("RA=" << hh << "h " << mm << "m " << ss << "s");
  LOG_DEBUG_STR("DEC=" << deg << "deg " << degmm << "' " << degss << "''");
      
  if (pipe) fflush(pipe);
}

int Beam::convertPointings(RTC::Timestamp begintime, int compute_interval, AMC::Converter* conv)
{
  // remember last pointing from previous call
  Pointing current_pointing = m_pointing;

  Pointing track[compute_interval];
  bool     pset[compute_interval];
  for (int i = 0; i < compute_interval; i++) pset[i] = false;

  // reset lmns array
  m_lmns.resize(compute_interval, 3);
  m_lmns = 0.0;

  LOG_DEBUG_STR("Begintime=" << begintime);

  while (!m_pointing_queue.empty()) {

    Pointing pointing = m_pointing_queue.top();

    LOG_DEBUG_STR("Process pointing @ time " << pointing.time());

    if (pointing.time() < begintime) {

      //
      // Deadline missed, print warning but execute the command anyway
      // as soon as possible.
      //
      LOG_WARN_STR("Deadline missed by " << (begintime.sec() - pointing.time().sec())
		   << " seconds (" << pointing.angle0() << "," << pointing.angle1() << ") "
		   << "@ " << pointing.time());

      pointing.setTime(begintime);
    }

    if (pointing.time() < begintime + (long)compute_interval) {

      m_pointing_queue.pop(); // remove from queue

      //
      // insert converted LM coordinate at correct 
      // position in the m_lmns array
      //
      register int tsec = pointing.time().sec() - begintime.sec();

      if ( (tsec < 0) || (tsec >= compute_interval) ) {

	LOG_ERROR_STR("\ninvalid pointing time" << pointing.time());
	continue;
      }

      track[tsec] = pointing;
      pset[tsec] = true;

      m_pointing = pointing; // remember current pointing

    } else {
      // done with this period
      break;
    }
  }

  //
  // track array will have unset values because
  // there are not necessarily pointings at each second
  // these holes are fixed up in this loop
  // and each pointing is converted to LMN coordinates
  //
  for (int t=0; t < compute_interval; ++t) {

    if (!pset[t]) {
      track[t] = current_pointing;
    } else {
      current_pointing = track[t];
    }

    /* set time and convert to LMN */
    track[t].setTime(begintime + (long)t);
    Pointing lmn = track[t].convertToLMN(conv, &m_pos);

    /* store in m_lmns and calculate normalized n-coordinate */
    m_lmns(t,0) = lmn.angle0();
    m_lmns(t,1) = lmn.angle1();
    m_lmns(t,2) = ::sqrt(1.0 - ((m_lmns(t,0) * m_lmns(t,0))
				+ (m_lmns(t,1) * m_lmns(t,1))));

    
    LOG_TRACE_VAR(formatString("direction@%d=(%f,%f,%f)",
			       begintime.sec() + t,
			       m_lmns(t,0), m_lmns(t,1), m_lmns(t,2)));

    if ((fabs(m_lmns(t,0)) > 1.0) || (fabs(m_lmns(t,1)) > 1.0)) {

      LOG_ERROR("\nl or m coordinate out of range -1.0 < l < 1.0, setting to (l,m) to (0.0, 0.0)\n");
      m_lmns(t,0) = 0.0;
      m_lmns(t,1) = 0.0;
      m_lmns(t,2) = 1.0;
    }
  }

  LOG_DEBUG_STR("Current pointing (" << current_pointing.angle0() << ", " << current_pointing.angle1() <<
		") @ time " << current_pointing.time());
  logPointing(current_pointing);

  return 0;
}

const Array<double,2>& Beam::getLMNCoordinates() const
{
  return m_lmns;
}

const CAL::SpectralWindow& Beam::getSPW() const
{
  return m_array.getSPW();
}

Beams::Beams(int nbeamlets, int nsubbands, EarthCoord pos) : m_beamlets(nbeamlets), m_nsubbands(nsubbands), m_pos(pos)
{
}

Beam* Beams::get(string nodeid, string subarrayname, Beamlet2SubbandMap allocation)
{
  Beam* beam = new Beam(nodeid, subarrayname, m_nsubbands, m_pos);

  if (beam) {

    if (!beam->allocate(allocation, m_beamlets, m_nsubbands)) {
      delete beam;
      beam = 0;
    } else {
      // add to list of active beams with 0 calibration handle
      m_beams[beam] = 0;
    }
  }
  return beam;
}

void Beams::setCalibrationHandle(Beam* beam, uint32 handle)
{
  m_handle2beam[handle] = beam;
  m_beams[beam]         = handle;
}

uint32 Beams::findCalibrationHandle(Beam* beam) const
{
  map<Beam*,uint32>::const_iterator it = m_beams.find(beam);

  if (it != m_beams.end()) {
    return it->second;
  }

  return 0;
}

bool Beams::updateCalibration(uint32 handle, CAL::AntennaGains& gains)
{
  map<uint32,Beam*>::iterator it = m_handle2beam.find(handle);

  if ( (it == m_handle2beam.end()) || (!it->second) ) return false;

  it->second->setCalibration(gains);

  return true;
}

bool Beams::exists(Beam *beam)
{
  // if beam not found, return 0
  map<Beam*,uint32>::iterator it = m_beams.find(beam);

  if (it == m_beams.end()) return false;
  
  return true;
}

bool Beams::destroy(Beam* beam)
{
  // remove from handle2beam map
  for (map<uint32,Beam*>::iterator it = m_handle2beam.begin();
       it != m_handle2beam.end(); ++it) {
    if (beam == it->second) m_handle2beam.erase(it);
  }
  
  // if beam not found, return false
  map<Beam*,uint32>::iterator it = m_beams.find(beam);

  if (it != m_beams.end()) {
    delete(it->first);
    m_beams.erase(it);
    return true;
  }

  return false;
}

void Beams::calculate_weights(Timestamp timestamp,
			      int compute_interval,
			      blitz::Array<std::complex<double>, 3>& weights,
			      AMC::Converter* conv)
{
  // iterate over all beams
  for (map<Beam*,uint32>::iterator bi = m_beams.begin(); bi != m_beams.end(); ++bi)
  {
    bi->first->convertPointings(timestamp, compute_interval, conv);
    LOG_DEBUG(formatString("current_pointing=(%f,%f)",
			   bi->first->getPointing().angle0(),
			   bi->first->getPointing().angle1()));
  }

  m_beamlets.calculate_weights(weights);
}

Beamlet2SubbandMap Beams::getSubbandSelection()
{
  Beamlet2SubbandMap selection;
  for (map<Beam*,uint32>::iterator bi = m_beams.begin(); bi != m_beams.end(); ++bi)
  {
    Beamlet2SubbandMap beammap = bi->first->getAllocation();
    selection().insert(beammap().begin(), beammap().end());
  }

  return selection;
}
