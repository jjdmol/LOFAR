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

#include "test.h"
#include "suite.h"

#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>
using namespace boost::posix_time;

#include <blitz/array.h>
using namespace blitz;

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>
using namespace LOFAR;

#define N_BEAMS              (8)
#define N_BEAMLETS           (2)
#define N_SUBBANDS_PER_BEAM  (N_BEAMLETS/N_BEAMS)

using namespace ABS;
using namespace std;

#define UPDATE_INTERVAL  1
#define COMPUTE_INTERVAL 7
#define N_SIGNALS 4

class BeamServerTest : public Test
{
private:
    Beam*          m_beam[N_BEAMS];
    SpectralWindow m_spw;

public:

    /**
     * create a spectral window from 10MHz to 90Mhz
     * steps of 256kHz
     * SpectralWindow spw(10e6, 256*1e3, 80*(1000/250));
     */
    BeamServerTest() :
	Test("BeamServerTest"),
	m_spw(10e6, 256*1e3, 80*(1000/256))
	{
	  //cerr << "c";
	  Beam::init(N_BEAMS, UPDATE_INTERVAL, COMPUTE_INTERVAL);
	  Beamlet::init(N_BEAMLETS);
	}

    void setUp()
	{
#if 0
	  //cerr << "s";
	  for (int i = 0; i < N_BEAMS; i++)
	  {
	      m_beam[i] = Beam::getInstance();
	      _test(m_beam[i] != 0);
	  }
#endif
	}

    void tearDown()
	{
	  //cerr << "t";
	  // nothing needed
	}

    void run()
	{
	  setUp();
	  
	  allocate();
	  deallocate();
	  subbandSelection();
	  oneTooManyBeam();
	  oneTooManyBeamlet();
	  emptyBeam();
	  pointing();
	  convert_pointings();
	  check_weights();

	  tearDown();
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
	      _test(0 != (m_beam[i] = Beam::allocate(m_spw, subbands)));
	  }
	}

    void deallocate()
	{
	  for (int i = 0; i < N_BEAMS; i++)
	  {
	      _test(m_beam[i]->deallocate() == 0);
	  }
	}

    void subbandSelection()
	{
	  allocate();

	  map<int,int>   selection;
	  selection.clear();
	  for (int i = 0; i < N_BEAMS; i++)
	  {
	      m_beam[i]->getSubbandSelection(selection);
	  }

	  _test(selection.size() == N_BEAMS*N_SUBBANDS_PER_BEAM);

	  deallocate();
	}

    void oneTooManyBeam()
	{
	  allocate();

	  // and allocate one more
	  Beam* beam = 0;
	  set<int> subbands;

	  subbands.clear();
	  for (int i = 0; i < N_SUBBANDS_PER_BEAM; i++) subbands.insert(i);
	  _test(0 == (beam = Beam::allocate(m_spw, subbands)));

	  deallocate();
	}

    void oneTooManyBeamlet()
	{
	  // insert one too many subbands
	  set<int> subbands;
	  subbands.clear();
	  for (int i = 0; i < N_BEAMLETS + 1; i++)
	  {
	      subbands.insert(i);
	  }
	  _test(0 == (m_beam[0] = Beam::allocate(m_spw, subbands)));
	}

    void emptyBeam()
	{
	  set<int> subbands;
	  subbands.clear();
	  _test(0 != (m_beam[0] = Beam::allocate(m_spw, subbands)));
	  _test(m_beam[0]->deallocate() == 0);
	}

    void pointing()
	{
	  set<int> subbands;
	  subbands.clear();

	  ptime thetime = from_time_t(time(0)) + seconds(20);

	  // allocate beam, addPointing, should succeed
	  _test(0 != (m_beam[0] = Beam::allocate(m_spw, subbands)));
	  _test(m_beam[0]->addPointing(Pointing(Direction(
			    0.0, 0.0, Direction::J2000), thetime)) == 0);

	  // deallocate beam, addPointing, should fail
	  _test(m_beam[0]->deallocate() == 0);
	  _test(m_beam[0]->addPointing(Pointing(Direction(
			    0.0, 0.0, Direction::J2000), thetime + seconds(5))) < 0);
	}

    void convert_pointings()
	{
	  allocate();

	  ptime now = from_time_t(time(0));

	  // add a few pointings
	  _test(m_beam[0]->addPointing(Pointing(Direction(
			    0.0, 1.0, Direction::LOFAR_LMN), now + seconds(1))) == 0);
	  _test(m_beam[0]->addPointing(Pointing(Direction(
			    0.0, 0.2, Direction::LOFAR_LMN), now + seconds(3))) == 0);
	  _test(m_beam[0]->addPointing(Pointing(Direction(
			    0.3, 0.0, Direction::LOFAR_LMN), now + seconds(5))) == 0);
	  _test(m_beam[0]->addPointing(Pointing(Direction(
			    0.4, 0.0, Direction::LOFAR_LMN), now + seconds(8))) == 0);
	  _test(m_beam[0]->addPointing(Pointing(Direction(
			    0.5, 0.0, Direction::LOFAR_LMN), now + seconds(COMPUTE_INTERVAL))) == 0);

	  struct timeval start, delay;
	  gettimeofday(&start, 0);
	  // iterate over all beams
	  time_period period(now, seconds(COMPUTE_INTERVAL));

	  _test(0 == m_beam[0]->convertPointings(period));
	  period = time_period(now + seconds(COMPUTE_INTERVAL), seconds(COMPUTE_INTERVAL));
	  _test(0 == m_beam[0]->convertPointings(period));

	  Array<W_TYPE, 2>          pos(N_SIGNALS, 3);
	  Array<complex<W_TYPE>, 3> weights(COMPUTE_INTERVAL, N_SIGNALS, N_BEAMLETS);

	  pos(0,Range::all()) = 1.0;
	  pos(1,Range::all()) = 0.5;

	  Beamlet::calculate_weights(pos, weights);
	  gettimeofday(&delay, 0);

	  //cout << "weights(0,0,:) = " << weights(0,0,Range::all()) << endl;
	  //cout << "weights(0,0,:) = " << weights(0,1,Range::all()) << endl;

	  delay.tv_sec -= start.tv_sec;
	  delay.tv_usec -= start.tv_usec;
	  if (delay.tv_usec < 0)
	  {
	      delay.tv_sec -= 1;
	      delay.tv_usec = 1000000 + delay.tv_usec;
	  }
	  LOG_INFO(formatString("calctime = %d sec %d msec", delay.tv_sec, delay.tv_usec/1000));

	  deallocate();
	}

    void check_weights()
	{
	  set<int> subbands;

	  subbands.clear();
	  subbands.insert(0);
	  subbands.insert(1);
	  _test(0 != (m_beam[0] = Beam::allocate(m_spw, subbands)));

	  ptime now = from_time_t(time(0));

	  // add a few pointings
	  _test(m_beam[0]->addPointing(Pointing(Direction(
			    0.0, 0.0,
			    Direction::LOFAR_LMN), now + seconds(0))) == 0);
	  _test(m_beam[0]->addPointing(Pointing(Direction(
			    0.0, 1.0,
			    Direction::LOFAR_LMN), now + seconds(1))) == 0);
	  _test(m_beam[0]->addPointing(Pointing(Direction(
			    1.0, 0.0,
			    Direction::LOFAR_LMN), now + seconds(2))) == 0);
	  _test(m_beam[0]->addPointing(Pointing(Direction(
			    sin(M_PI/4.0), sin(M_PI/4.0),
			    Direction::LOFAR_LMN), now + seconds(3))) == 0);
	  _test(m_beam[0]->addPointing(Pointing(Direction(
			    sin(M_PI/4.0), 0.0,
			    Direction::LOFAR_LMN), now + seconds(4))) == 0);
	  _test(m_beam[0]->addPointing(Pointing(Direction(
			    0.0, sin(M_PI/4.0),
			    Direction::LOFAR_LMN), now + seconds(5))) == 0);
	  _test(m_beam[0]->addPointing(Pointing(Direction(
			    sqrt(1.0/3.0), sqrt(1.0/3.0),
			    Direction::LOFAR_LMN), now + seconds(6))) == 0);

	  struct timeval start, delay;
	  gettimeofday(&start, 0);
	  // iterate over all beams
	  time_period period(now, seconds(COMPUTE_INTERVAL));

	  _test(0 == m_beam[0]->convertPointings(period));
//	  period = time_period(now + seconds(COMPUTE_INTERVAL), seconds(COMPUTE_INTERVAL));
//	  _test(0 == m_beam[0]->convertPointings(period));

	  Array<W_TYPE, 2>          pos(N_SIGNALS, 3);
	  Array<complex<W_TYPE>, 3> weights(COMPUTE_INTERVAL, N_SIGNALS, N_BEAMLETS);

	  pos(0,Range::all()) = 0.0, 0.0, 0.0;
	  pos(1,Range::all()) = 0.0, 0.0, 1.0;
	  pos(2,Range::all()) = 0.0, 1.0, 0.0;	  
	  pos(3,Range::all()) = 1.0, 0.0, 0.0;

	  cout << "pos = " << pos << endl;

	  cout << "lmn = " << m_beam[0]->getLMNCoordinates() << endl;

	  Beamlet::calculate_weights(pos, weights);
	  gettimeofday(&delay, 0);

	  cout << "weights(:,:,0) = " << weights(Range::all(),Range::all(),0) << endl;
	  cout << "weights(:,:,1) = " << weights(Range::all(),Range::all(),1) << endl;

	  delay.tv_sec -= start.tv_sec;
	  delay.tv_usec -= start.tv_usec;
	  if (delay.tv_usec < 0)
	  {
	      delay.tv_sec -= 1;
	      delay.tv_usec = 1000000 + delay.tv_usec;
	  }
	  LOG_INFO(formatString("calctime = %d sec %d msec", delay.tv_sec, delay.tv_usec/1000));

	  _test(m_beam[0]->deallocate() == 0);
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
