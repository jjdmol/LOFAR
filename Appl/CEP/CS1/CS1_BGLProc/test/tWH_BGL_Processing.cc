//#  tWH_BGL_Processing.cc: stand-alone test program for WH_BGL_Processing
//#
//#  Copyright (C) 2006
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <APS/ParameterSet.h>

#if defined HAVE_MPI
#include <Transport/TH_MPI.h>
#endif

#if defined HAVE_BGL
#include <bglpersonality.h>
#include <rts.h>
#endif

#include <Common/Timer.h>
#include <WH_BGL_Processing.h>
#include <cmath>
#include <cstring>
#include <exception>

using namespace LOFAR;
using namespace LOFAR::CS1;


inline DH_Subband::SampleType toComplex(double phi)
{
    double s, c;

    sincos(phi, &s, &c);
#if INPUT_TYPE == I4COMPLEX_TYPE
    return makei4complex((int) rint(7 * c), (int) rint(7 * s));
#elif INPUT_TYPE == I16COMPLEX_TYPE
    return makei16complex((int) rint(32767 * c), (int) rint(32767 * s));
#else
#error Unknown INPUT_TYPE
#endif
}


void setSubbandTestPattern(WH_BGL_Processing &wh, unsigned nrStations, double signalFrequency, double sampleRate)
{
  // Simulate a monochrome complex signal into the PPF, with station 1 at a
  // distance of .25 labda to introduce a delay.  Also, a few samples can be
  // flagged.

  std::clog << "setSubbandTestPattern::setTestPattern() ... ";

  static NSTimer timer("setTestPattern", true);
  timer.start();

  DH_Subband		    *dh        = wh.get_DH_Subband();
  DH_Subband::Samples3Dtype samples    = dh->getSamples3D();
  DH_Subband::DelaysType    delays     = dh->getDelays();
  DH_Subband::FlagsType     flags      = dh->getFlags();

  const double		    distance   = .25; // labda
  const double		    phaseShift = 2 * M_PI * distance;

  for (unsigned stat = 0; stat < nrStations; stat ++) {
    delays[0].delayAtBegin = delays[0].delayAfterEnd = 0;
  }

  for (unsigned time = 0; time < samples[0].size(); time ++) {
    double phi = 2 * M_PI * signalFrequency * time / sampleRate;
    DH_Subband::SampleType sample = toComplex(phi);

    for (unsigned stat = 0; stat < nrStations; stat ++) {
      samples[stat][time][0] = samples[stat][time][1] = sample;
    }

    if (NR_POLARIZATIONS >= 2 && nrStations > 2) {
      samples[1][time][1]    = toComplex(phi + phaseShift);
      delays[1].delayAtBegin = delays[1].delayAfterEnd = distance / signalFrequency;
    }
  }
  
  for (unsigned stat = 0; stat < nrStations; stat ++) {
    flags[stat].reset();
  }

#if 1
  if (dh->nrInputSamples() > 17000 && nrStations >= 6) {
    flags[4].include(14000);
    flags[5].include(17000);
  }
#endif

  dh->fillExtraData();
  std::clog << "done." << std::endl;;

#if defined WORDS_BIGENDIAN
  std::clog << "swapBytes()" << std::endl;
  dh->swapBytes();
#endif

  timer.stop();
}


#if 0
void setRFItestPattern(WH_BGL_Processing &wh, unsigned nrStations)
{
  DH_RFI_Mitigation::ChannelFlagsType *flags = wh.get_DH_RFI_Mitigation()->getChannelFlags();

  memset(flags, 0, sizeof(DH_RFI_Mitigation::ChannelFlagsType));

#if 0 && NR_SUBBAND_CHANNELS >= 256
  if (nrStations >= 3)
    (*flags)[2][255] = true;
#endif
}
#endif


void checkCorrelatorTestPattern(WH_BGL_Processing &wh, unsigned nrStations)
{
  DH_Visibilities::VisibilitiesType	 visibilities	= wh.get_DH_Visibilities()->getVisibilities();
  DH_Visibilities::AllNrValidSamplesType nrValidSamples = wh.get_DH_Visibilities()->getNrValidSamples();

  static const int			 channels[]	= { 1, 73, 255 };

  for (unsigned stat1 = 0; stat1 < std::min(nrStations, 8U); stat1 ++) {
    for (unsigned stat2 = stat1; stat2 < std::min(nrStations, 8U); stat2 ++) {
      int bl = DH_Visibilities::baseline(stat1, stat2);

      std::cout << "S(" << stat1 << ") * ~S(" << stat2 << ") :\n";

      for (int pol1 = 0; pol1 < NR_POLARIZATIONS; pol1 ++) {
	for (int pol2 = 0; pol2 < NR_POLARIZATIONS; pol2 ++) {
	  std::cout << " " << (char) ('x' + pol1) << (char) ('x' + pol2) << ':';

	  for (size_t chidx = 0; chidx < sizeof(channels) / sizeof(int); chidx ++) {
	    int ch = channels[chidx];

	    if (ch < NR_SUBBAND_CHANNELS) {
	      std::cout << ' ' << visibilities[bl][ch][pol1][pol2] << '/' << nrValidSamples[bl][ch];
	    }
	  }

	  std::cout << '\n';
	}
      }
    }
  }

  std::cout << "newgraph newcurve linetype solid marktype none pts\n";
  float max = 0.0;

  for (int ch = 1; ch < NR_SUBBAND_CHANNELS; ch ++) {
    if (abs(visibilities[0][ch][1][1]) > max) {
      max = abs(visibilities[0][ch][1][1]);
    }
  }

  for (int ch = 1; ch < NR_SUBBAND_CHANNELS; ch ++) {
    std::cout << ch << ' ' << (10 * std::log10(abs(visibilities[0][ch][1][1]) / max)) << '\n';
  }
}


void doWork()
{
#if defined HAVE_BGL
  // only test on the one or two cores of the first compute node

  struct BGLPersonality personality;
  int retval = rts_get_personality(&personality, sizeof personality);
  ASSERTSTR(retval == 0, "Could not get personality");

  if (personality.getXcoord() != 0 || personality.getYcoord() != 0 || personality.getZcoord() != 0) {
    return;
  }
#endif

  ACC::APS::ParameterSet parameterSet("CS1.parset");
  CS1_Parset pset(&parameterSet);
  double     signalFrequency = pset.refFreqs()[0] + 73 * pset.chanWidth(); // channel 73
  int	     nRuns	     = 1;
  const char *env;

  if ((env = getenv("NRUNS")) != 0) {
    nRuns = atoi(env);
    std::clog << "setting nRuns to " << env << std::endl;
  }

  if ((env = getenv("SIGNAL_FREQUENCY")) != 0) {
    signalFrequency = atof(env);
  }

  std::clog << "base frequency = " << pset.refFreqs()[0] << std::endl;
  std::clog << "channel bandwidth = " << pset.chanWidth() << std::endl;
  std::clog << "signal frequency = " << signalFrequency << std::endl;
  WH_BGL_Processing wh("WH_BGL_Processing", 0, &pset);

#if defined HAVE_MPI
  wh.runOnNode(TH_MPI::getCurrentRank());
#endif

  wh.basePreprocess();
  setSubbandTestPattern(wh, pset.nrStations(), signalFrequency, pset.sampleRate());
//setRFItestPattern(wh);

  for (int i = 0; i < nRuns; i ++) {
    wh.baseProcess();
  }

  checkCorrelatorTestPattern(wh, pset.nrStations());
  wh.basePostprocess();
}


int main (int argc, char **argv)
{
  int retval = 0;

#if defined HAVE_BGL
  // make std::clog line buffered
  static char buffer[4096];
  setvbuf(stderr, buffer, _IOLBF, sizeof buffer);
#endif

#if defined HAVE_MPI
  TH_MPI::initMPI(argc, argv);
#else
  argc = argc; argv = argv;    // Keep compiler happy ;-)
#endif

  try {
    doWork();
  } catch (Exception& e) {
    std::cerr << "Caught exception: " << e.what() << std::endl;
    retval = 1;
  } catch (std::exception& e) {
    std::cerr << "Caught exception: " << e.what() << std::endl;
    retval = 1;
  } catch (...) {
    std::cerr << "Caught exception " << std::endl;
    retval = 1;
  }

#if defined HAVE_MPI
  TH_MPI::finalize();
#endif

  return retval;
}
