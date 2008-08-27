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

#include <CS1_Interface/BGL_Configuration.h>
#include <Common/DataConvert.h>
#include <Common/Exception.h>
#include <Common/Timer.h>
#include <PPF.h>
#include <Correlator.h>

#if defined HAVE_MPI
#define MPICH_IGNORE_CXX_SEEK
#include <mpi.h>
#endif

#if defined HAVE_BGL
#include <bglpersonality.h>
#include <rts.h>
#endif

#include <cmath>
#include <cstring>
#include <exception>


using namespace LOFAR;
using namespace LOFAR::CS1;


inline TransposedData::SampleType toComplex(double phi)
{
    double s, c;

    sincos(phi, &s, &c);
#if NR_BITS_PER_SAMPLE == 4
    return makei4complex(8 * c, 8 * s);
#elif NR_BITS_PER_SAMPLE == 8
    return makei8complex((int) rint(127 * c), (int) rint(127 * s));
#elif NR_BITS_PER_SAMPLE == 16
    return makei16complex((int) rint(32767 * c), (int) rint(32767 * s));
#else
#error Unknown NR_BITS_PER_SAMPLE
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
    transposedData->metaData[stat].delayAtBegin   = 0;
    transposedData->metaData[stat].delayAfterEnd  = 0;
    transposedData->metaData[stat].alignmentShift = 0;
    transposedData->metaData[stat].setFlags(SparseSet<unsigned>());
  }

  for (unsigned time = 0; time < transposedData->samples[0].size(); time ++) {
    double phi = 2 * M_PI * signalFrequency * time / sampleRate;
    TransposedData::SampleType sample = toComplex(phi);

    for (unsigned stat = 0; stat < nrStations; stat ++) {
      transposedData->samples[stat][time][0] = sample;
      transposedData->samples[stat][time][1] = sample;
    }

    if (NR_POLARIZATIONS >= 2 && nrStations > 2) {
      transposedData->samples[1][time][1]     = toComplex(phi + phaseShift);
      transposedData->metaData[1].delayAtBegin  = distance / signalFrequency;
      transposedData->metaData[1].delayAfterEnd = distance / signalFrequency;
    }
  }
  
#if 1
  if (transposedData->samples[0].size() > 17000 && nrStations >= 6) {
    transposedData->metaData[4].setFlags(SparseSet<unsigned>().include(14000));
    transposedData->metaData[5].setFlags(SparseSet<unsigned>().include(17000));
  }
#endif

  std::clog << "done." << std::endl;;

#if 1 && defined WORDS_BIGENDIAN
  std::clog << "swap bytes" << std::endl;
  dataConvert(LittleEndian, transposedData->samples.data(), transposedData->samples.num_elements());
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

  for (int ch = 1; ch < NR_SUBBAND_CHANNELS; ch ++)
    if (abs(visibilities[0][ch][1][1]) > max)
      max = abs(visibilities[0][ch][1][1]);

  std::clog << "max = " << max << std::endl;

  for (int ch = 1; ch < NR_SUBBAND_CHANNELS; ch ++)
    std::cout << ch << ' ' << (10 * std::log10(abs(visibilities[0][ch][1][1]) / max)) << '\n';
}


void doWork()
{
#if defined HAVE_BGL
  // only test on the one or two cores of the first compute node

  struct BGLPersonality personality;

  if (rts_get_personality(&personality, sizeof personality) != 0) {
    std::cerr << "Could not get personality" << std::endl;
    exit(1);
  }

  if (personality.getXcoord() == 0 && personality.getYcoord() == 0 && personality.getZcoord() == 0)
#endif
  {
    unsigned   nrStations	= 77;
    unsigned   nrSamplesPerIntegration = 768;
    double     sampleRate	= 195312.5;
    double     refFreq		= 384 * sampleRate;
    double     signalFrequency	= refFreq + 73 * sampleRate / NR_SUBBAND_CHANNELS; // channel 73
    unsigned   nrSamplesToBGLProc = NR_SUBBAND_CHANNELS * (nrSamplesPerIntegration + NR_TAPS - 1) + 32 / sizeof(TransposedData::SampleType[NR_POLARIZATIONS]);
    unsigned   nrBaselines	= nrStations * (nrStations + 1) / 2;

    const char *env;

    if ((env = getenv("SIGNAL_FREQUENCY")) != 0) {
      signalFrequency = atof(env);
    }

    std::clog << "base frequency = " << refFreq << std::endl;
    std::clog << "signal frequency = " << signalFrequency << std::endl;

    size_t transposedDataSize = TransposedData::requiredSize(nrStations, nrSamplesToBGLProc);
    size_t filteredDataSize   = FilteredData::requiredSize(nrStations, nrSamplesPerIntegration);
    size_t correlatedDataSize = CorrelatedData::requiredSize(nrBaselines);

    std::clog << transposedDataSize << " " << filteredDataSize << " " << correlatedDataSize << std::endl;
    MallocedArena arena0(filteredDataSize, 32);
    MallocedArena arena1(std::max(transposedDataSize, correlatedDataSize), 32);

    TransposedData transposedData(arena1, nrStations, nrSamplesToBGLProc);
    FilteredData   filteredData(arena0, nrStations, nrSamplesPerIntegration);
    CorrelatedData correlatedData(arena1, nrBaselines);

    PPF		   ppf(nrStations, nrSamplesPerIntegration, sampleRate / NR_SUBBAND_CHANNELS, true);
    Correlator     correlator(nrStations, nrSamplesPerIntegration);

    setSubbandTestPattern(&transposedData, nrStations, signalFrequency, sampleRate);
    ppf.computeFlags(&transposedData, &filteredData);
    ppf.filter(refFreq, &transposedData, &filteredData);

    correlator.computeFlagsAndCentroids(&filteredData, &correlatedData);
    correlator.correlate(&filteredData, &correlatedData);

    checkCorrelatorTestPattern(&correlatedData, nrStations);
  }
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
  MPI_Init(&argc, &argv);
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
  MPI_Finalize();
#endif

  return retval;
}
