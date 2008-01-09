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

#if defined HAVE_MPI
#include <Transport/TH_MPI.h>
#endif

#if defined HAVE_BGL
#include <bglpersonality.h>
#include <rts.h>
#endif

#include <CS1_Interface/BGL_Configuration.h>
#include <Common/DataConvert.h>
#include <Common/Exception.h>
#include <Common/Timer.h>
#include <Transport/TH_Null.h>
#include <BGL_Processing.h>
#include <cmath>
#include <cstring>
#include <exception>

using namespace LOFAR;
using namespace LOFAR::CS1;


inline TransposedData::SampleType toComplex(double phi)
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


void setSubbandTestPattern(TransposedData *transposedData, unsigned nrStations, double signalFrequency, double sampleRate)
{
  // Simulate a monochrome complex signal into the PPF, with station 1 at a
  // distance of .25 labda to introduce a delay.  Also, a few samples can be
  // flagged.

  std::clog << "setSubbandTestPattern() ... ";

  static NSTimer timer("setTestPattern", true);
  timer.start();

  const double distance   = .25; // labda
  const double phaseShift = 2 * M_PI * distance;

  for (unsigned stat = 0; stat < nrStations; stat ++) {
    transposedData->delays[stat].delayAtBegin	= 0;
    transposedData->delays[stat].delayAfterEnd	= 0;
    transposedData->alignmentShifts[stat]	= 0;
  }

  for (unsigned time = 0; time < transposedData->samples3D[0].size(); time ++) {
    double phi = 2 * M_PI * signalFrequency * time / sampleRate;
    TransposedData::SampleType sample = toComplex(phi);

    for (unsigned stat = 0; stat < nrStations; stat ++) {
      transposedData->samples3D[stat][time][0] = sample;
      transposedData->samples3D[stat][time][1] = sample;
    }

    if (NR_POLARIZATIONS >= 2 && nrStations > 2) {
      transposedData->samples3D[1][time][1]     = toComplex(phi + phaseShift);
      transposedData->delays[1].delayAtBegin  = distance / signalFrequency;
      transposedData->delays[1].delayAfterEnd = distance / signalFrequency;
    }
  }
  
  for (unsigned stat = 0; stat < nrStations; stat ++) {
    transposedData->flags[stat].reset();
  }

#if 1
  if (transposedData->samples3D[0].size() > 17000 && nrStations >= 6) {
    transposedData->flags[4].include(14000);
    transposedData->flags[5].include(17000);
  }
#endif

  std::clog << "done." << std::endl;;

#if 0 && defined WORDS_BIGENDIAN
  std::clog << "swap bytes" << std::endl;
  dataConvert(LittleEndian, transposedData->samples3D.data(), transposedData->samples3D.num_elements());
#endif

  timer.stop();
}


#if 0
void WH_BGL_ProcessingTest::setRFItestPattern(unsigned nrStations)
{
  DH_RFI_Mitigation::ChannelFlagsType *flags = get_DH_RFI_Mitigation()->getChannelFlags();

  memset(flags, 0, sizeof(DH_RFI_Mitigation::ChannelFlagsType));

#if 0 && NR_SUBBAND_CHANNELS >= 256
  if (nrStations >= 3)
    (*flags)[2][255] = true;
#endif
}
#endif


void checkCorrelatorTestPattern(const CorrelatedData *correlatedData, unsigned nrStations)
{
  const boost::multi_array_ref<fcomplex, 4> &visibilities = correlatedData->visibilities;

  static const int channels[] = { 1, 201, 255 };

  for (unsigned stat1 = 0; stat1 < std::min(nrStations, 8U); stat1 ++) {
    for (unsigned stat2 = stat1; stat2 < std::min(nrStations, 8U); stat2 ++) {
      int bl = Correlator::baseline(stat1, stat2);

      std::cout << "S(" << stat1 << ") * ~S(" << stat2 << ") :\n";

      for (int pol1 = 0; pol1 < NR_POLARIZATIONS; pol1 ++) {
	for (int pol2 = 0; pol2 < NR_POLARIZATIONS; pol2 ++) {
	  std::cout << " " << (char) ('x' + pol1) << (char) ('x' + pol2) << ':';

	  for (size_t chidx = 0; chidx < sizeof(channels) / sizeof(int); chidx ++) {
	    int ch = channels[chidx];

	    if (ch < NR_SUBBAND_CHANNELS) {
	      std::cout << ' ' << visibilities[bl][ch][pol1][pol2] << '/' << correlatedData->nrValidSamples[bl][ch];
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

#if 0
  ACC::APS::ParameterSet parameterSet("../../test/test.parset");
  CS1_Parset ps(&parameterSet);
  double     signalFrequency = ps.refFreqs()[0] + 73 * ps.chanWidth(); // channel 73
  const char *env;

  if ((env = getenv("SIGNAL_FREQUENCY")) != 0) {
    signalFrequency = atof(env);
  }

  std::clog << "base frequency = " << ps.refFreqs()[0] << std::endl;
  std::clog << "channel bandwidth = " << ps.chanWidth() << std::endl;
  std::clog << "signal frequency = " << signalFrequency << std::endl;

  BGL_Processing proc;
  proc.preprocess(&ps);
  setSubbandTestPattern(proc.itsTransposedData, ps.nrStations(), signalFrequency, ps.sampleRate());
  proc.process();

  checkCorrelatorTestPattern(proc.itsCorrelatedData, ps.nrStations());
  proc.postprocess();
#else
  BGL_Configuration configuration;

  configuration.nrStations()		  = 6;
  configuration.nrSamplesPerIntegration() = 608;
  configuration.nrSamplesToBGLProc()	  = NR_SUBBAND_CHANNELS * (configuration.nrSamplesPerIntegration() + NR_TAPS - 1) + 32 / sizeof(TransposedData::SampleType[NR_POLARIZATIONS]);
  configuration.nrUsedCoresPerPset()	  = 1;
  configuration.nrSubbandsPerPset()	  = 1;
  configuration.delayCompensation()	  = true;
  configuration.sampleRate()		  = 156250.0;
  configuration.inputPsets()		  = std::vector<unsigned>();
  configuration.outputPsets()		  = std::vector<unsigned>(1, 0);
  configuration.refFreqs()		  = std::vector<double>(1, 384 * configuration.sampleRate());

  double     signalFrequency = configuration.refFreqs()[0] + 73 * configuration.sampleRate() / NR_SUBBAND_CHANNELS; // channel 73
  const char *env;

  if ((env = getenv("SIGNAL_FREQUENCY")) != 0) {
    signalFrequency = atof(env);
  }

  std::clog << "base frequency = " << configuration.refFreqs()[0] << std::endl;
  std::clog << "signal frequency = " << signalFrequency << std::endl;

  TH_Null	 th;
  BGL_Processing proc(&th);
  proc.preprocess(configuration);
  setSubbandTestPattern(proc.itsTransposedData, configuration.nrStations(), signalFrequency, configuration.sampleRate());
  proc.process();

  checkCorrelatorTestPattern(proc.itsCorrelatedData, configuration.nrStations());
  proc.postprocess();
#endif
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
