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

#include "ABSSpectralWindow.h"
#include "ABSBeam.h"
#include "ABSBeamlet.h"

#include "test.h"
#include "suite.h"

#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <blitz/array.h>

#include <stdio.h>
#include <netinet/in.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace boost::posix_time;
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
  SpectralWindow m_spw;

public:

    /**
     * create a spectral window from 10MHz to 90Mhz
     * steps of 256kHz
     * SpectralWindow spw(1.5e6, 256*1e3, 80*(1000/250));
     */
    CheckWeightsTest() :
	Test("CheckWeightsTest"),
	m_spw(1.5e6, 256*1e3, 80*(1000/256))
	{
	  //cerr << "c";
	  Beam::init(N_BEAMS, UPDATE_INTERVAL, COMPUTE_INTERVAL);
	  Beamlet::init(N_BEAMLETS);
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
			    0.0, -1.0,
			    Direction::LOFAR_LMN), now + seconds(2))) == 0);

	  struct timeval start, delay;
	  gettimeofday(&start, 0);
	  // iterate over all beams
	  time_period period(now, seconds(COMPUTE_INTERVAL));

	  _test(0 == m_beam[0]->convertPointings(period));

	  Array<W_TYPE, 3>           pos(N_ELEMENTS, N_POLARIZATIONS, 3);
	  Array<complex<W_TYPE>, 4>  weights(COMPUTE_INTERVAL, N_ELEMENTS, N_SUBBANDS, N_POLARIZATIONS);
	  Array<complex<int16_t>, 4> weights16(COMPUTE_INTERVAL, N_ELEMENTS, N_SUBBANDS, N_POLARIZATIONS);

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

#if 0	  
	  firstIndex i;
	  secondIndex j;
	  thirdIndex k;
	  fourthIndex l;
#else
	  for (int i = 0; i < COMPUTE_INTERVAL; i++)
	    for (int j = 0; j < N_ELEMENTS; j++)
	      for (int k = 0; k < N_SUBBANDS; k++)
		for (int l = 0; l < N_POLARIZATIONS; l++)
#endif
		  {
		    weights16(i,j,k,l) = conj(weights16(i,j,k,l));
		    weights16(i,j,k,l) = complex<int16_t>((int16_t)round(weights(i,j,k,l).real()*SCALE),
							  (int16_t)round(weights(i,j,k,l).imag()*SCALE));
		  }	  
	  cout << "weights16 = " << weights16 << endl;
	  
	  imag(weights16(all, all, all, 1)) *= -1;

	  cout << "weights16_imag_neg = " << weights16 << endl;

#if 0
	  Array<complex<W_TYPE>, 2> weights_ref(COMPUTE_INTERVAL, N_ELEMENTS);
	  Array<complex<W_TYPE>, 2> error(COMPUTE_INTERVAL, N_ELEMENTS);

	  //
	  // read in reference data
	  //
	  FILE* wfile = fopen("../../../test/weights2.dat", "r");
	  _test(wfile);
	  if (!wfile) return;

	  for (int bl = 0; bl < N_BEAMLETS; bl++)
	  {
	    _test((size_t)weights_ref.size()
		  == fread(weights_ref.data(), sizeof(complex<W_TYPE>), weights_ref.size(), wfile));

	    error = weights_ref - weights(all, all, bl, 0);
	    cout << "error = abs(sum(error)) = " << abs(sum(error)) << endl;
	    _test(abs(sum(error)) < 1e-8);

	    error = weights_ref - weights(all, all, bl, 1);
	    cout << "error = abs(sum(error)) = " << abs(sum(error)) << endl;
	    _test(abs(sum(error)) < 1e-8);
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

  Suite s("Check Weights Test Suite", &cout);

  s.addTest(new CheckWeightsTest);
  s.run();
  long nFail = s.report();
  s.free();
  return nFail;
}
