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

#include "ABSSpectralWindow.h"
#include "ABSBeam.h"

#include "ABSTest.h"

#include <iostream>

#include <blitz/array.h>
using namespace blitz;

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>
using namespace LOFAR;

#define N_BEAMS              (8)
#define N_BEAMLETS           (206)
#define N_SUBBANDS_PER_BEAM  (N_BEAMLETS/N_BEAMS)

using namespace ABS;
using namespace std;

#define UPDATE_INTERVAL  1
#define COMPUTE_INTERVAL 10
#define N_ELEMENTS       100
#define N_POLARIZATIONS  2

class BeamServerTest : public Test
{
private:
    Beam*          m_beam[N_BEAMS];

public:

    /**
     * create a spectral window from 10MHz to 90Mhz
     * steps of 256kHz
     * SpectralWindow spw(10e6, 256*1e3, 80*(1000/250));
     */
    BeamServerTest() :
	Test("BeamServerTest")
	{
	  //cerr << "c";
	  Beam::init(N_BEAMS, UPDATE_INTERVAL, COMPUTE_INTERVAL);
	  Beamlet::init(N_BEAMLETS);

	  SpectralWindow* window = new SpectralWindow(10e6, 256*1e3, 80*(1000/256), 0/*don't care*/);
	  SpectralWindowConfig::getInstance().setSingle(window);
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
	  set<int> subbands;

	  subbands.clear();
	  for (int i = 0; i < N_SUBBANDS_PER_BEAM; i++)
	  {
	      subbands.insert(i);
	  }
	  for (int i = 0; i < N_BEAMS; i++)
	  {
	      TESTC(0 != (m_beam[i] = Beam::allocate(0, subbands)));
	  }
	}

    void deallocate()
	{
	  for (int i = 0; i < N_BEAMS; i++)
	  {
	      TESTC(m_beam[i]->deallocate() == 0);
	  }
	}

    void subbandSelection()
	{
	  START_TEST("subbandsSelection", "test subband selection");

	  allocate();

	  map<int,int>   selection;
	  selection.clear();
	  for (int i = 0; i < N_BEAMS; i++)
	  {
	      m_beam[i]->getSubbandSelection(selection);
	  }

	  TESTC(selection.size() == N_BEAMS*N_SUBBANDS_PER_BEAM);

	  deallocate();

	  STOP_TEST();
	}

    void oneTooManyBeam()
	{
	  START_TEST("oneTooManyBeam", "test if allocation of one more beam than possible does indeed fail");

	  allocate();

	  // and allocate one more
	  Beam* beam = 0;
	  set<int> subbands;

	  subbands.clear();
	  for (int i = 0; i < N_SUBBANDS_PER_BEAM; i++) subbands.insert(i);
	  TESTC(0 == (beam = Beam::allocate(0, subbands)));

	  deallocate();

	  STOP_TEST();
	}

    void oneTooManyBeamlet()
	{
	  START_TEST("oneTooManyBeamlet", "test if allocation of one more subband fails");
 
	  // insert one too many subbands
	  set<int> subbands;
	  subbands.clear();
	  for (int i = 0; i < N_BEAMLETS + 1; i++)
	  {
	      subbands.insert(i);
	  }
	  TESTC(0 == (m_beam[0] = Beam::allocate(0, subbands)));

	  STOP_TEST();
	}

    void emptyBeam()
	{
	  START_TEST("emptyBeam", "check that empty beam allocation fails");

	  set<int> subbands;
	  subbands.clear();
	  TESTC(0 != (m_beam[0] = Beam::allocate(0, subbands)));
	  TESTC(m_beam[0]->deallocate() == 0);

	  STOP_TEST();
	}

    void pointing()
	{
	  START_TEST("pointing", "check addPointing on allocated and deallocated beam");

	  set<int> subbands;
	  subbands.clear();

	  time_t thetime = time(0) + 20;

	  // allocate beam, addPointing, should succeed
	  TESTC(0 != (m_beam[0] = Beam::allocate(0, subbands)));
	  TESTC(m_beam[0]->addPointing(Pointing(Direction(
			    0.0, 0.0, Direction::J2000), thetime)) == 0);

	  // deallocate beam, addPointing, should fail
	  TESTC(m_beam[0]->deallocate() == 0);
	  TESTC(m_beam[0]->addPointing(Pointing(Direction(
			    0.0, 0.0, Direction::J2000), thetime + 5)) < 0);

	  STOP_TEST();
	}

    void convert_pointings()
	{
	  START_TEST("convert_pointings", "convert pointings and calculate weights");

	  Range all = Range::all();

	  allocate();

	  time_t now = time(0);

	  // add a few pointings
	  TESTC(m_beam[0]->addPointing(Pointing(Direction(
			    0.0, 1.0, Direction::LOFAR_LMN), now + 1)) == 0);
	  TESTC(m_beam[0]->addPointing(Pointing(Direction(
			    0.0, 0.2, Direction::LOFAR_LMN), now + 3)) == 0);
	  TESTC(m_beam[0]->addPointing(Pointing(Direction(
			    0.3, 0.0, Direction::LOFAR_LMN), now + 5)) == 0);
	  TESTC(m_beam[0]->addPointing(Pointing(Direction(
			    0.4, 0.0, Direction::LOFAR_LMN), now + 8)) == 0);
	  TESTC(m_beam[0]->addPointing(Pointing(Direction(
			    0.5, 0.0, Direction::LOFAR_LMN), now + COMPUTE_INTERVAL)) == 0);

	  struct timeval start, delay;
	  gettimeofday(&start, 0);
	  // iterate over all beams
	  time_t begintime = now;

	  TESTC(0 == m_beam[0]->convertPointings(begintime));
	  TESTC(0 == m_beam[0]->convertPointings(begintime + COMPUTE_INTERVAL));

	  Array<W_TYPE, 3>          pos(N_ELEMENTS, N_POLARIZATIONS, 3);
	  Array<complex<W_TYPE>, 4> weights(COMPUTE_INTERVAL, N_ELEMENTS, N_BEAMLETS, N_POLARIZATIONS);

	  pos = 1.0; // x,y coordiante = 1
	  pos(all, all, 2) = 0.0; // z-coordinate = 0

	  Beamlet::calculate_weights(pos, weights);
	  gettimeofday(&delay, 0);

	  //cout << "weights(0,0,:,:) = " << weights(0,0,Range::all(),Range::all()) << endl;
	  //cout << "weights(0,0,:,:) = " << weights(0,1,Range::all(),Range::all()) << endl;

	  delay.tv_sec -= start.tv_sec;
	  delay.tv_usec -= start.tv_usec;
	  if (delay.tv_usec < 0)
	  {
	      delay.tv_sec -= 1;
	      delay.tv_usec = 1000000 + delay.tv_usec;
	  }
	  LOG_INFO(formatString("calctime = %d sec %d msec", delay.tv_sec, delay.tv_usec/1000));

	  deallocate();

	  STOP_TEST();
	}

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
