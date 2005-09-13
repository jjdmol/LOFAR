//#
//#  BeamServerTest.cc: class definition for the Beam Server task.
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

#include "Beam.h"
#include "BSTest.h"
#include "MEPHeader.h"
#include "Timestamp.h"
#include "SubArray.h"

#include <iostream>

#include <blitz/array.h>

#define N_BEAMS              (8)
#define N_SUBBANDS_PER_BEAM  (MEPHeader::N_BEAMLETS/N_BEAMS)

using namespace blitz;
using namespace std;
using namespace LOFAR;
using namespace BS;
using namespace EPA_Protocol;
using namespace BS_Protocol;
using namespace RTC;
using namespace CAL;

#define UPDATE_INTERVAL  1
#define COMPUTE_INTERVAL 10
#define N_ELEMENTS       100
#define N_POLARIZATIONS  2

namespace LOFAR {

  class BeamServerTest : public Test {

  private:
    Beams m_beams;
    Beam* m_beamptr[MEPHeader::N_BEAMLETS];

  public:

    BeamServerTest() :
      Test("BeamServerTest"),
      m_beams(MEPHeader::N_BEAMLETS,
	      MEPHeader::N_SUBBANDS,
	      AMC::EarthCoord(1.0,1.0,0.0))
    {
    }

    void run()
    {
#if 0
      allocate();
      deallocate();
#endif
      subbandSelection();
      oneTooManyBeam();
      oneTooManyBeamlet();
      emptyBeam();
      pointing();
      convert_pointings();
    }

    void allocate()
    {
      Beamlet2SubbandMap allocation;

      for (int bi = 0; bi < N_BEAMS; bi++) {
	allocation().clear();
	for (int si = 0; si < N_SUBBANDS_PER_BEAM; si++) {
	  allocation()[si + (N_SUBBANDS_PER_BEAM * bi)] = si;
	}
	
	char name[32];
	snprintf(name, 32, "beam%d", bi);
    
	TESTC(0 != (m_beamptr[bi] = m_beams.get(name, allocation)));
      }
    }

    void deallocate()
    {
      for (int i = 0; i < N_BEAMS; i++)
	{
	  TESTC(true == m_beams.destroy(m_beamptr[i]));
	}
    }

    void subbandSelection()
    {
      START_TEST("subbandsSelection", "test subband selection");

      allocate();

      Beamlet2SubbandMap selection;
      selection = m_beams.getSubbandSelection();

      TESTC(selection().size() == N_BEAMS*N_SUBBANDS_PER_BEAM);

      deallocate();

      STOP_TEST();
    }

    void oneTooManyBeam()
    {
      START_TEST("oneTooManyBeam", "test if allocation of one more beam than possible does indeed fail");

      allocate();

      // and allocate one more
      Beam* beam = 0;

      Beamlet2SubbandMap allocation;

      for (int i = 0; i < N_SUBBANDS_PER_BEAM; i++) {
	allocation()[i] = i;
      }

      TESTC(0 == (beam = m_beams.get("oneTooManyBeam", allocation)));

      deallocate();

      STOP_TEST();
    }

    void oneTooManyBeamlet()
    {
      START_TEST("oneTooManyBeamlet", "test if allocation of one more subband fails");
 
      Beamlet2SubbandMap allocation;

      for (int i = 0; i < MEPHeader::N_BEAMLETS + 1; i++) {
	allocation()[i] = i;
      }

      TESTC(0 == (m_beamptr[0] = m_beams.get("oneTooManyBeamlet", allocation)));

      STOP_TEST();
    }

    void emptyBeam()
    {
      START_TEST("emptyBeam", "check that empty beam allocation fails");

      Beamlet2SubbandMap allocation;

      TESTC(0 == (m_beamptr[0] = m_beams.get("empty", allocation)));
      TESTC(false == m_beams.destroy(m_beamptr[0]));

      STOP_TEST();
    }

    void pointing()
    {
      START_TEST("pointing", "check addPointing on allocated and deallocated beam");

      Beamlet2SubbandMap allocation;
      for (int i = 0; i < MEPHeader::N_BEAMLETS; i++) {
	allocation()[i] = i;
      }

      // allocate beam, addPointing, should succeed
      TESTC(0 != (m_beamptr[0] = m_beams.get("pointing", allocation)));
      if (m_beamptr[0]) {
	m_beamptr[0]->addPointing(Pointing(0.0, 0.0, Timestamp::now(20), Pointing::J2000));
      }

      TESTC(true == m_beams.destroy(m_beamptr[0]));

      STOP_TEST();
    }

    void convert_pointings()
    {
      START_TEST("convert_pointings", "convert pointings and calculate weights");

      Range all = Range::all();

      //allocate();

      Beamlet2SubbandMap allocation;
      for (int i = 0; i < MEPHeader::N_BEAMLETS; i++) {
	allocation()[i] = i;
      }
      TESTC(0 != (m_beamptr[0] = m_beams.get("beam", allocation)));

      Timestamp now = Timestamp::now();

      // add a few pointings
      m_beamptr[0]->addPointing(Pointing(0.0, 1.0, now + (long)1, Pointing::LOFAR_LMN));
      m_beamptr[0]->addPointing(Pointing(0.0, 0.2, now + (long)3, Pointing::LOFAR_LMN));
      m_beamptr[0]->addPointing(Pointing(0.3, 0.0, now + (long)5, Pointing::LOFAR_LMN));
      m_beamptr[0]->addPointing(Pointing(0.4, 0.0, now + (long)8, Pointing::LOFAR_LMN));
      m_beamptr[0]->addPointing(Pointing(0.5, 0.0, now + (long)COMPUTE_INTERVAL, Pointing::LOFAR_LMN));

      // start timer
      struct timeval start, delay;
      gettimeofday(&start, 0);

      Array<double, 3>          pos(N_ELEMENTS, N_POLARIZATIONS, 3);
      Array<bool, 2>            select(N_ELEMENTS, N_POLARIZATIONS);
      Array<complex<double>, 3> weights(COMPUTE_INTERVAL, N_ELEMENTS * N_POLARIZATIONS, MEPHeader::N_BEAMLETS);
      Array<double, 1> loc(3);

      loc = 0.0, 0.0, 0.0;
      select = true;
      SubArray subarray("subarray", loc, pos, select, 160000000.0, 1, MEPHeader::N_SUBBANDS);

      pos = 1.0; // x,y coordiante = 1
      pos(all, all, 2) = 0.0; // z-coordinate = 0

      m_beamptr[0]->setSubarray(subarray);
      m_beams.calculate_weights(now, COMPUTE_INTERVAL,
				pos, weights, 0);

      // stop timer
      gettimeofday(&delay, 0);
      delay.tv_sec -= start.tv_sec;
      delay.tv_usec -= start.tv_usec;
      if (delay.tv_usec < 0)
	{
	  delay.tv_sec -= 1;
	  delay.tv_usec = 1000000 + delay.tv_usec;
	}
      LOG_INFO(formatString("calctime = %d sec %d msec", delay.tv_sec, delay.tv_usec/1000));

      TESTC(true == m_beams.destroy(m_beamptr[0]));

      STOP_TEST();
    }

  };
};

int main(int /*argc*/, char** /*argv*/)
{
  Pointing p;
  p = p;

  char prop_path[PATH_MAX];
  const char* mac_config = getenv("MAC_CONFIG");

  snprintf(prop_path, PATH_MAX-1,
	   "%s/%s", (mac_config?mac_config:"."),
	   "log4cplus.properties");
  INIT_LOGGER(prop_path);

  Suite s("Beam Server Test Suite", &cout);

  s.addTest(new BeamServerTest);
  s.run();
  long nFail = s.report();
  s.free();
  return nFail;
}
