//#  tWH_CN_Processing.cc: stand-alone test program for WH_CN_Processing
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

#include <Interface/CN_Configuration.h>
#include <Common/DataConvert.h>
#include <Common/Exception.h>
#include <Common/Timer.h>
#include <PPF.h>
#include <BeamFormer.h>
#include <Correlator.h>
#include <ArenaMapping.h>

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
using namespace LOFAR::RTCP;


template <typename T> void toComplex(double phi, T &z);

template <> inline void toComplex<i4complex>(double phi, i4complex &z)
{
    double s, c;

    sincos(phi, &s, &c);
    z = makei4complex(8 * c, 8 * s);
}

template <> inline void toComplex<i8complex>(double phi, i8complex &z)
{
    double s, c;

    sincos(phi, &s, &c);
    z = makei8complex((int) rint(127 * c), (int) rint(127 * s));
}

template <> inline void toComplex<i16complex>(double phi, i16complex &z)
{
    double s, c;

    sincos(phi, &s, &c);
    z = makei16complex((int) rint(32767 * c), (int) rint(32767 * s));
}


template <typename SAMPLE_TYPE> void setSubbandTestPattern(TransposedData<SAMPLE_TYPE> *transposedData, unsigned nrStations, double signalFrequency, double sampleRate)
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
    SAMPLE_TYPE sample;
    toComplex(phi, sample);

    for (unsigned stat = 0; stat < nrStations; stat ++) {
      transposedData->samples[stat][time][0] = sample;
      transposedData->samples[stat][time][1] = sample;
    }

    if (NR_POLARIZATIONS >= 2 && nrStations > 2) {
      toComplex(phi + phaseShift, transposedData->samples[1][time][1]);
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


void checkCorrelatorTestPattern(const CorrelatedData *correlatedData, unsigned nrStations, unsigned nrChannels)
{
  const boost::multi_array_ref<fcomplex, 4> &visibilities = correlatedData->visibilities;

  static const unsigned channels[] = { 1, 201, 255 };

  for (unsigned stat1 = 0; stat1 < std::min(nrStations, 8U); stat1 ++) {
    for (unsigned stat2 = stat1; stat2 < std::min(nrStations, 8U); stat2 ++) {
      int bl = Correlator::baseline(stat1, stat2);

      std::cout << "S(" << stat1 << ") * ~S(" << stat2 << ") :\n";

      for (unsigned pol1 = 0; pol1 < NR_POLARIZATIONS; pol1 ++) {
	for (unsigned pol2 = 0; pol2 < NR_POLARIZATIONS; pol2 ++) {
	  std::cout << " " << (char) ('x' + pol1) << (char) ('x' + pol2) << ':';

	  for (size_t chidx = 0; chidx < sizeof(channels) / sizeof(int); chidx ++) {
	    unsigned ch = channels[chidx];

	    if (ch < nrChannels) {
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

  for (unsigned ch = 1; ch < nrChannels; ch ++)
    if (abs(visibilities[0][ch][1][1]) > max)
      max = abs(visibilities[0][ch][1][1]);

//  std::clog << "max = " << max << std::endl;

  for (unsigned ch = 1; ch < nrChannels; ch ++)
    std::cout << ch << ' ' << (10 * std::log10(abs(visibilities[0][ch][1][1]) / max)) << '\n';
}


template <typename SAMPLE_TYPE> void doWork()
{
#if defined HAVE_BGL
  // only test on the one or two cores of the first compute node

  struct CNPersonality personality;

  if (rts_get_personality(&personality, sizeof personality) != 0) {
    std::cerr << "Could not get personality" << std::endl;
    exit(1);
  }

  if (personality.getXcoord() == 0 && personality.getYcoord() == 0 && personality.getZcoord() == 0)
#endif
  {
    unsigned   nrStations	= 64;
    unsigned   nrChannels	= 256;
    unsigned   nrSamplesPerIntegration = 768;
    double     sampleRate	= 195312.5;
    double     centerFrequency	= 384 * sampleRate;
    double     baseFrequency	= centerFrequency - .5 * sampleRate;
    unsigned   testSignalChannel = 5;
    double     signalFrequency	= baseFrequency + testSignalChannel * sampleRate / nrChannels;
    unsigned   nrSamplesToCNProc = nrChannels * (nrSamplesPerIntegration + NR_TAPS - 1) + 32 / sizeof(SAMPLE_TYPE[NR_POLARIZATIONS]);

    std::vector<unsigned> station2SuperStation;
# if 0
    station2SuperStation.resize(nrStations);
    for(unsigned i=0; i<nrStations; i++) {
      station2SuperStation[i] = (i / 7);
//      cerr << station2SuperStation[i] << endl;
    }
#endif

#if 0
    // just to get the factors!
    LOFAR::RTCP::BandPass bandpass(true, nrChannels);
    const float *f = bandpass.correctionFactors();
    
    std::clog << "bandpass correction:" << std::endl;
    for (unsigned i = 0; i < nrChannels; i ++)
      std::clog << i << ' ' << f[i] << std::endl;
#endif

    if(testSignalChannel >= nrChannels) {
      std::cerr << " signal lies outside the range." << std::endl;
      exit(1);
    }

    BeamFormer     beamFormer(nrStations, nrSamplesPerIntegration, station2SuperStation, nrChannels);

    const char *env;
    unsigned nrBeamFormedStations = beamFormer.getNrBeamFormedStations();
    unsigned nrBaselines = nrBeamFormedStations * (nrBeamFormedStations + 1) / 2;


    if ((env = getenv("SIGNAL_FREQUENCY")) != 0) {
      signalFrequency = atof(env);
    }

    std::clog << "base   frequency = " << baseFrequency   << std::endl;
    std::clog << "center frequency = " << centerFrequency << std::endl;
    std::clog << "signal frequency = " << signalFrequency << std::endl;

    ArenaMapping mapping;

    TransposedData<SAMPLE_TYPE> transposedData(nrStations, nrSamplesToCNProc);
    FilteredData   filteredData(nrStations, nrChannels, nrSamplesPerIntegration);
    CorrelatedData correlatedData(nrBaselines, nrChannels);

    mapping.addDataset( &transposedData, 1 );
    mapping.addDataset( &filteredData, 0 );
    mapping.addDataset( &correlatedData, 1 );
    mapping.allocate();

    std::clog << transposedData.requiredSize() << " " << filteredData.requiredSize() << " " << correlatedData.requiredSize() << std::endl;

    PPF<SAMPLE_TYPE> ppf(nrStations, nrChannels, nrSamplesPerIntegration, sampleRate / nrChannels, true, true);
    Correlator     correlator(beamFormer.getNrBeamFormedStations(), 
			      beamFormer.getStationMapping(), nrChannels, nrSamplesPerIntegration, true /* use bandpass correction */);

    setSubbandTestPattern(&transposedData, nrStations, signalFrequency, sampleRate);

    for (unsigned stat = 0; stat < nrStations; stat ++) {
      ppf.computeFlags(stat, &transposedData, &filteredData);
      ppf.filter(stat, centerFrequency, &transposedData, &filteredData);
    }

    beamFormer.formBeams(&filteredData);

    correlator.computeFlagsAndCentroids(&filteredData, &correlatedData);
    correlator.correlate(&filteredData, &correlatedData);

    checkCorrelatorTestPattern(&correlatedData, beamFormer.getNrBeamFormedStations(), nrChannels);
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

#if defined HAVE_BGP || defined HAVE_BGL
  INIT_BGP_LOGGER(argv[0]);
#endif

#if defined HAVE_MPI
  MPI_Init(&argc, &argv);
#else
  argc = argc; argv = argv;    // Keep compiler happy ;-)
#endif

  try {
    doWork<i16complex>();
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
