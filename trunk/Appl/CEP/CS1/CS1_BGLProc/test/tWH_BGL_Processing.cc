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
#include <CS1_BGLProc/WH_BGL_Processing.h>
#include <cmath>
#include <cstring>
#include <exception>


namespace LOFAR
{
#if defined HAVE_BGL
static BGL_Barrier *barrier;
#endif

inline i16complex toComplex(double phi)
{
    double s, c;

    sincos(phi, &s, &c);
    return makei16complex((int) (32767 * c), (int) (32767 * s));
}


void setSubbandTestPattern(WH_BGL_Processing &wh, double signalFrequency)
{
  // Simulate a monochrome complex signal into the PPF, with station 1 at a
  // distance of .25 labda to introduce a delay.  Also, a few samples can be
  // flagged.

  (std::cerr << "setSubbandTestPattern::setTestPattern() ... ").flush();

  static NSTimer timer("setTestPattern", true);
  timer.start();

  DH_Subband		     *dh	= wh.get_DH_Subband();
  DH_Subband::AllSamplesType *samples	= dh->getSamples();
  DH_Subband::AllFlagsType   *flags	= dh->getFlags();
  DH_Subband::AllDelaysType  *delays	= dh->getDelays();

  const double		     distance	= .25; // labda
  const double		     phaseShift = 2 * M_PI * distance;

  for (int time = 0; time < NR_INPUT_SAMPLES; time ++) {
    double     phi    = 2 * M_PI * signalFrequency * time / SAMPLE_RATE;
    i16complex sample = toComplex(phi);

    for (int stat = 0; stat < NR_STATIONS; stat ++) {
      (*samples)[stat][time][0] = time;
      (*samples)[stat][time][1] = sample;
    }

#if NR_STATIONS >= 2 && NR_POLARIZATIONS == 2
    (*samples)[1][time][1]    = toComplex(phi + phaseShift);
    (*delays)[1].delayAtBegin = (*delays)[1].delayAfterEnd = -distance / signalFrequency;
#endif
  }
  
  memset(flags, 0, sizeof(DH_Subband::AllFlagsType));

#if 0 && NR_INPUT_SAMPLES >= 17000
  (*flags)[4][14000] = true;
  (*flags)[5][17000] = true;
#endif

  (std::cerr << "done.\n").flush();

#if defined WORDS_BIGENDIAN
  (std::cerr << "swapBytes()\n").flush();
  dh->swapBytes();
#endif

  timer.stop();
}


void setRFItestPattern(WH_BGL_Processing &wh)
{
  DH_RFI_Mitigation::ChannelFlagsType *flags = wh.get_DH_RFI_Mitigation()->getChannelFlags();

  memset(flags, 0, sizeof(DH_RFI_Mitigation::ChannelFlagsType));

#if 0 && NR_STATIONS >= 3 && NR_SUBBAND_CHANNELS >= 256
  (*flags)[2][255] = true;
#endif
}


void checkCorrelatorTestPattern(WH_BGL_Processing &wh)
{
  DH_Visibilities		      *dh	    = wh.get_DH_Visibilities();
  DH_Visibilities::VisibilitiesType   *visibilities = dh->getVisibilities();
  DH_Visibilities::NrValidSamplesType *validSamples = dh->getNrValidSamplesCounted();

  static const int		      channels[]    = { 0, 73, 255 };

  for (int stat1 = 0; stat1 < std::min(NR_STATIONS, 8); stat1 ++) {
    for (int stat2 = stat1; stat2 < std::min(NR_STATIONS, 8); stat2 ++) {
      int bl = DH_Visibilities::baseline(stat1, stat2);

      std::cout << "S(" << stat1 << ") * ~S(" << stat2 << ") :\n";

      for (int pol1 = 0; pol1 < NR_POLARIZATIONS; pol1 ++) {
	for (int pol2 = 0; pol2 < NR_POLARIZATIONS; pol2 ++) {
	  std::cout << " " << (char) ('x' + pol1) << (char) ('x' + pol2) << ':';

	  for (int chidx = 0; chidx < sizeof(channels) / sizeof(int); chidx ++) {
	    int ch = channels[chidx];

	    if (ch < NR_SUBBAND_CHANNELS) {
	      std::cout << ' ' << (*visibilities)[bl][ch][pol1][pol2] << '/' << (*validSamples)[bl][ch];
	    }
	  }

	  std::cout << '\n';
	}
      }
    }
  }

  std::cout << "newgraph newcurve linetype solid marktype none pts\n";
  float max = 0.0;

  for (int ch = 0; ch < NR_SUBBAND_CHANNELS; ch ++) {
    if (cabs((*visibilities)[0][ch][1][1]) > max) {
      max = cabs((*visibilities)[0][ch][1][1]);
    }
  }

  for (int ch = 0; ch < NR_SUBBAND_CHANNELS; ch ++) {
    std::cout << ch << ' ' << (10 * std::log10(cabs((*visibilities)[0][ch][1][1]) / max)) << '\n';
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

  barrier = rts_allocate_barrier();
#endif

  double     baseFrequency   = 49938432.0; // 254th Nyquist zone
  double     signalFrequency = 49994496.0; // channel 73
  int	     nRuns	     = 1;
  const char *env;

  if ((env = getenv("BASE_FREQUENCY")) != 0) {
    signalFrequency = atof(env);
    std::cerr << "setting signal frequency to " << env << '\n';
  }

  if ((env = getenv("SIGNAL_FREQUENCY")) != 0) {
    baseFrequency = atof(env);
    std::cerr << "setting base frequency to " << env << '\n';
  }

  if ((env = getenv("NRUNS")) != 0) {
    nRuns = atoi(env);
    std::cerr << "setting nRuns to " << env << '\n';
  }

  ACC::APS::ParameterSet pset("CS1.cfg");
  WH_BGL_Processing wh("WH_BGL_Processing", baseFrequency, pset);

#if defined HAVE_MPI
  wh.runOnNode(TH_MPI::getCurrentRank());
#endif

  wh.basePreprocess();
  setSubbandTestPattern(wh, signalFrequency);
  setRFItestPattern(wh);

#if defined HAVE_BGL
  if (TH_MPI::getNumberOfNodes() > 1) {
    BGL_Barrier_Pass(barrier);
  }
#endif

  for (int i = 0; i < nRuns; i ++) {
    wh.baseProcess();
  }

  checkCorrelatorTestPattern(wh);
  wh.basePostprocess();
}

} // namespace LOFAR


using namespace LOFAR;

int main (int argc, const char **argv)
{
  int retval = 0;

#if defined HAVE_MPI
  TH_MPI::initMPI(argc, argv);
#endif

  try {
    doWork();
  } catch (LOFAR::Exception e) {
    cerr << "Caught exception: " << e.what() << endl;
    retval = 1;
  } catch (std::exception e) {
    cerr << "Caught exception: " << e.what() << endl;
    retval = 1;
  } catch (...) {
    cerr << "Caught exception " << endl;
    retval = 1;
  }

#if defined HAVE_MPI
  TH_MPI::finalize();
#endif

  return retval;
}
