//#
//#  CheckWeightsTest.cc: class definition for the Beam Server task.
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

#include "SpectralWindow.h"
#include "Beam.h"
#include <APL/BS_Protocol/Pointing.h>

#include <TestSuite/suite.h>

#include <iostream>
#include <blitz/array.h>

#include <stdio.h>

using namespace blitz;
using namespace LOFAR;
using namespace BS_Protocol;
using namespace BS;
using namespace std;

#define N_BEAMS          1 // must always be greater than 0!
#define N_BEAMLETS       2
#define UPDATE_INTERVAL  1
#define COMPUTE_INTERVAL 14
#define N_ELEMENTS       7
#define N_POLARIZATIONS  2
#define N_SUBBANDS       N_BEAMLETS

class CheckWeightsTest : public Test
{
private:
  Beams m_beams;
  Beam* m_beam[N_BEAMS];

public:

    /**
     * create a spectral window from 10MHz to 90Mhz
     * steps of 256kHz
     * SpectralWindow spw(10e6, 256*1e3, 80*(1000/250));
     */
    CheckWeightsTest() :
      Test("CheckWeightsTest"), m_beams(N_BEAMLETS)
	{
#if 0
	  SpectralWindow* window = new SpectralWindow(10e6, 256*1e3, 80*(1000/256), 0/*don't care*/);
	  SpectralWindowConfig::getInstance().setSingle(window);
#endif
	}

    void run()
	{
	  check_weights();
	}

    void check_weights()
	{
	  set<int> subbands;
	  Range all = Range::all();

	  //
	  // Compare result of this test with Octave/Matlab generated reference
	  //
	  BS_Protocol::Beamlet2SubbandMap allocation;
	  for (int i = 0; i < N_SUBBANDS; i++) allocation()[i] = i;
	  TESTC(0 != (m_beam[0] = m_beams.get("CheckWeightsTest", allocation, N_SUBBANDS)));

	  RTC::Timestamp now;
	  now.setNow();
	  
	  // add a few pointings
	  m_beam[0]->addPointing(Pointing(0.0, 0.0, now,
					  Pointing::LOFAR_LMN));
	  m_beam[0]->addPointing(Pointing(0.0, 1.0, now + (long)1,
					  Pointing::LOFAR_LMN));
	  m_beam[0]->addPointing(Pointing(1.0, 0.0, now + (long)2,
					  Pointing::LOFAR_LMN));
	  m_beam[0]->addPointing(Pointing(::sin(M_PI/4.0), ::sin(M_PI/4.0), now + (long)3,
					  Pointing::LOFAR_LMN));
	  m_beam[0]->addPointing(Pointing(::sin(M_PI/4.0), 0.0, now + (long)4,
					  Pointing::LOFAR_LMN));
	  m_beam[0]->addPointing(Pointing(0.0, ::sin(M_PI/4.0), now + (long)5,
					  Pointing::LOFAR_LMN));
	  m_beam[0]->addPointing(Pointing(::sqrt(1.0/3.0), ::sqrt(1.0/3.0), now + (long)6,
					  Pointing::LOFAR_LMN));

	  struct timeval start, delay;
	  gettimeofday(&start, 0);
	  // iterate over all beams

	  TESTC(0 == m_beam[0]->convertPointings(now, COMPUTE_INTERVAL, 0));

	  Array<double, 3>          pos(N_ELEMENTS, N_POLARIZATIONS, 3);
	  Array<bool, 2>            select(N_ELEMENTS, N_POLARIZATIONS);
	  Array<complex<double>, 3> weights(COMPUTE_INTERVAL, N_ELEMENTS * N_POLARIZATIONS, N_SUBBANDS);

	  select = true;

	  pos = 0.0;
	  pos(0,all,all) = 0.0, 0.0, 0.0;
	  pos(1,all,all) = 0.0, 0.0, 1.0;
	  pos(2,all,all) = 0.0, 1.0, 0.0;
	  pos(3,all,all) = 1.0, 0.0, 0.0;
	  pos(4,all,all) = 1.0, 1.0, 0.0;
	  pos(5,all,all) = 0.0, 1.0, 1.0;
	  pos(6,all,all) = 1.0, 0.0, 1.0;
	  
	  blitz::Array<double,1> geoloc(3);
	  geoloc=0.0;
	  CAL::SubArray subarray("subarray", geoloc, pos, select, 2.0 * N_SUBBANDS * 156.25e3, 1, 2);
	  m_beam[0]->setSubarray(subarray);

	  cout << "pos = " << pos << endl;

	  cout << "lmn = " << m_beam[0]->getLMNCoordinates() << endl;

	  m_beams.calculate_weights(now, COMPUTE_INTERVAL, pos, weights, 0);
	  gettimeofday(&delay, 0);

	  cout << "weights(subband=0) = " << weights(all,all,0) << endl;
	  cout << "weights(subband=1) = " << weights(all,all,1) << endl;

	  Array<complex<double>, 3> weights_ref(COMPUTE_INTERVAL, N_ELEMENTS * N_POLARIZATIONS, N_SUBBANDS);
	  Array<complex<double>, 3> error(COMPUTE_INTERVAL, N_ELEMENTS);

	  //
	  // read in reference data
	  //
	  FILE* wfile = fopen("../../../test/weights.dat", "r");
	  TESTC(wfile);
	  if (!wfile) return;

	  for (int bl = 0; bl < N_BEAMLETS; bl++)
	  {
	    TESTC((size_t)weights_ref.size()
		  == fread(weights_ref.data(), sizeof(complex<double>), weights_ref.size(), wfile));

	    error = weights_ref - weights; //(all, Range(0,weights.extent(secondDim)-1,2), bl);
	    cout << "error = abs(sum(error)) = " << abs(sum(error)) << endl;
	    TESTC(abs(sum(error)) < 1e-8);
#if 0
	    error = weights_ref - weights(all, Range(1,weights.extent(secondDim),2), bl);
	    cout << "error = abs(sum(error)) = " << abs(sum(error)) << endl;
	    TESTC(abs(sum(error)) < 1e-8);
#endif
	  }
	  fclose(wfile);

	  delay.tv_sec -= start.tv_sec;
	  delay.tv_usec -= start.tv_usec;
	  if (delay.tv_usec < 0)
	  {
	      delay.tv_sec -= 1;
	      delay.tv_usec = 1000000 + delay.tv_usec;
	  }
	  LOG_INFO(formatString("calctime = %d sec %d msec", delay.tv_sec, delay.tv_usec/1000));

	  TESTC(m_beams.destroy(m_beam[0]) == true);
	}
};

int main(int /*argc*/, char** /*argv*/)
{
  Pointing p;
  p = p;

  Suite s("Check Weights Test Suite", &cout);

  s.addTest(new CheckWeightsTest);
  
  try {
    s.run();
  } catch (string e) {
    cerr << "Uncaught exception: " << e << endl;
  }
  long nFail = s.report();
  s.free();
  return nFail;
}
