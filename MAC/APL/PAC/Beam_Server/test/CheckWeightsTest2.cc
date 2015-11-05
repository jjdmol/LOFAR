//#
//#  CheckWeightsTest2.cc: class definition for the Beam Server task.
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
#include "Beamlet.h"

#include <TestSuite/suite.h>

#include <iostream>
#include <blitz/array.h>

#include <stdio.h>
#include <netinet/in.h>

using namespace blitz;
using namespace LOFAR;
using namespace ABS;
using namespace std;

#define N_BEAMS          1 // must always be greater than 0!
#define N_BEAMLETS       1
#define UPDATE_INTERVAL  1
#define COMPUTE_INTERVAL 3
#define N_ELEMENTS       1
#define N_POLARIZATIONS  2
#define N_SUBBANDS       N_BEAMLETS

#define SCALE (1<<(16-2))

inline complex<int16_t> convert2complex_int16_t(complex<W_TYPE> cd)
{
#ifdef W_TYPE_DOUBLE
  return complex<int16_t>(htons((int16_t)(round(cd.real()*SCALE))),
			  htons((int16_t)(round(cd.imag()*SCALE))));
#else
  return complex<int16_t>(htons((int16_t)(roundf(cd.real()*SCALE))),
			  htons((int16_t)(roundf(cd.imag()*SCALE))));
#endif
}

class CheckWeightsTest : public Test
{
private:
  Beam*          m_beam[N_BEAMS];

public:

    /**
     * create a spectral window from 10MHz to 90Mhz
     * steps of 256kHz
     * SpectralWindow spw(1.5e6, 256*1e3, 80*(1000/250));
     */
    CheckWeightsTest() :
	Test("CheckWeightsTest")
	{
	  //cerr << "c";
	  Beam::init(N_BEAMS, UPDATE_INTERVAL, COMPUTE_INTERVAL);
	  Beamlet::init(N_BEAMLETS);

	  SpectralWindow* window = new SpectralWindow(1.5e6, 256*1e3, 80*(1000/256), 0 /*don't care*/);
	  SpectralWindowConfig::getInstance().setSingle(window);
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

	  subbands.clear();
	  for (int i = 0; i < N_SUBBANDS; i++) subbands.insert(i);
	  TESTC(0 != (m_beam[0] = Beam::allocate(0, subbands)));

	  time_t now = time(0);
	  
	  // add a few pointings
	  TESTC(m_beam[0]->addPointing(Pointing(Direction(
			    0.0, 0.0,
			    Direction::LOFAR_LMN), now + 0)) == 0);
	  TESTC(m_beam[0]->addPointing(Pointing(Direction(
			    0.0, 1.0,
			    Direction::LOFAR_LMN), now + 1)) == 0);
	  TESTC(m_beam[0]->addPointing(Pointing(Direction(
			    0.0, -1.0,
			    Direction::LOFAR_LMN), now + 2)) == 0);

	  struct timeval start, delay;
	  gettimeofday(&start, 0);
	  // iterate over all beams
	  time_t begintime = now;

	  TESTC(0 == m_beam[0]->convertPointings(begintime));

	  Array<W_TYPE, 3>           pos(N_ELEMENTS, N_POLARIZATIONS, 3);
	  Array<complex<W_TYPE>, 3>  weights(COMPUTE_INTERVAL, N_ELEMENTS * N_POLARIZATIONS, N_SUBBANDS);
	  Array<complex<int16_t>, 3> weights16(COMPUTE_INTERVAL, N_ELEMENTS * N_POLARIZATIONS, N_SUBBANDS);

	  weights   = complex<W_TYPE>(0,0);
	  weights16 = complex<int16_t>(0,0);

	  pos(0, 0, all) = 0.0, 0.0, 0.0;
	  pos(0, 1, all) = 100.0, 0.0, 0.0;

	  cout << "pos = " << pos << endl;

	  cout << "lmn = " << m_beam[0]->getLMNCoordinates() << endl;

	  Beamlet::calculate_weights(pos, weights);
	  gettimeofday(&delay, 0);

	  cout << "weights = " << weights << endl;

	  //weights16 = convert2complex_int16_t(conj(weights));

	  for (int i = 0; i < COMPUTE_INTERVAL; i++)
	    for (int j = 0; j < N_ELEMENTS * N_POLARIZATIONS; j++)
	      for (int k = 0; k < N_SUBBANDS; k++)
		  {
		    //weights16(i,j,k,l) = conj(weights16(i,j,k,l));
		    weights16(i,j,k) = complex<int16_t>((int16_t)round(weights(i,j,k).real()*SCALE),
							-1*(int16_t)round(weights(i,j,k).imag()*SCALE));
		  }	  
	  cout << "weights16 = " << weights16 << endl;
	  
	  imag(weights16(all, all, all)) *= -1;

	  cout << "weights16_imag_neg = " << weights16 << endl;

	  weights16 *= complex<int16_t>(0,1);

	  cout << "weights16_times_i = " << weights16 << endl;

#if 0
	  Array<complex<W_TYPE>, 2> weights_ref(COMPUTE_INTERVAL, N_ELEMENTS);
	  Array<complex<W_TYPE>, 2> error(COMPUTE_INTERVAL, N_ELEMENTS);

	  //
	  // read in reference data
	  //
	  FILE* wfile = fopen("../../../test/weights2.dat", "r");
	  TESTC(wfile);
	  if (!wfile) return;

	  for (int bl = 0; bl < N_BEAMLETS; bl++)
	  {
	    TESTC((size_t)weights_ref.size()
		  == fread(weights_ref.data(), sizeof(complex<W_TYPE>), weights_ref.size(), wfile));

	    error = weights_ref - weights(all, all, bl, 0);
	    cout << "error = abs(sum(error)) = " << abs(sum(error)) << endl;
	    TESTC(abs(sum(error)) < 1e-8);

	    error = weights_ref - weights(all, all, bl, 1);
	    cout << "error = abs(sum(error)) = " << abs(sum(error)) << endl;
	    TESTC(abs(sum(error)) < 1e-8);
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
#endif
	  TESTC(m_beam[0]->deallocate() == 0);
	}
};

int main(int /*argc*/, char** /*argv*/)
{
  Pointing p;
  p = p;

#if 0
  char prop_path[PATH_MAX];
  const char* mac_config = getenv("MAC_CONFIG");

  snprintf(prop_path, PATH_MAX-1,
	   "%s/%s", (mac_config?mac_config:"."),
	   "log4cplus.properties");
  INIT_LOGGER(prop_path);
#endif

  Suite s("Check Weights Test Suite", &cout);

  s.addTest(new CheckWeightsTest);
  s.run();
  long nFail = s.report();
  s.free();
  return nFail;
}
