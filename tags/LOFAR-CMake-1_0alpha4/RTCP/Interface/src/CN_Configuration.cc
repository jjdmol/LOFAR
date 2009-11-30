//#  CN_Configuration.cc:
//#
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

#include <Interface/CN_Configuration.h>

#include <cassert>
#include <cstring>
#include <algorithm>

namespace LOFAR {
namespace RTCP {


CN_Configuration::CN_Configuration()
{
#if defined HAVE_VALGRIND
  memset(&itsMarshalledData, 0, sizeof itsMarshalledData);
#endif
}


#if ! defined HAVE_BGP_CN

CN_Configuration::CN_Configuration(const Parset &parset)
{
#if defined HAVE_VALGRIND
  memset(&itsMarshalledData, 0, sizeof itsMarshalledData);
#endif

  nrStations()              = parset.nrStations();
  nrBitsPerSample()	    = parset.nrBitsPerSample();
  nrSubbands()              = parset.nrSubbands();
  nrChannelsPerSubband()    = parset.nrChannelsPerSubband();
  nrSamplesPerIntegration() = parset.CNintegrationSteps();
  nrSamplesPerStokesIntegration() = parset.stokesIntegrationSteps();
  nrSamplesToCNProc()       = parset.nrSamplesToCNProc();
  nrSubbandsPerPset()       = parset.nrSubbandsPerPset();
  delayCompensation()       = parset.delayCompensation();
  correctBandPass()  	    = parset.correctBandPass();
  sampleRate()              = parset.sampleRate();
  inputPsets()              = parset.inputPsets();
  outputPsets()             = parset.outputPsets();
  tabList()                 = parset.tabList();
  usedCoresInPset()	    = parset.usedCoresInPset();
  refFreqs()                = parset.subbandToFrequencyMapping();
  nrPencilBeams()           = parset.nrPencilBeams();
  refPhaseCentre()          = parset.getRefPhaseCentres();
  outputFilteredData()      = parset.outputFilteredData();
  outputCorrelatedData()    = parset.outputCorrelatedData();
  outputBeamFormedData()    = parset.outputBeamFormedData();
  outputCoherentStokes()    = parset.outputCoherentStokes();
  outputIncoherentStokes()  = parset.outputIncoherentStokes();
  nrStokes()                = parset.nrStokes();
  flysEye()                 = parset.flysEye();

  stokesIntegrateChannels() = parset.stokesIntegrateChannels();

  // Get the phase centres of all station, not just the one we receive input from. The compute nodes
  // need the phase centres for beam forming, which is after the transpose so all stations are present.

  // The order of the stations is the order in which they are defined in inputPsets and parset.getStationNamesAndRSPboardNumbers.
  // The CNProc/src/AsyncTranspose module should honor the same order.
  itsPhaseCentres.resize(parset.nrStations(), 3);
  std::vector<double> positions = parset.positions();

  for (unsigned stat = 0; stat < parset.nrStations(); stat ++)
    for (unsigned dim = 0; dim < 3; dim ++)
      itsPhaseCentres[stat][dim] = positions[stat*3+dim];
}

#endif
 

void CN_Configuration::read(Stream *str)
{
  str->read(&itsMarshalledData, sizeof itsMarshalledData);

  itsInputPsets.resize(itsMarshalledData.itsInputPsetsSize);
  memcpy(&itsInputPsets[0], itsMarshalledData.itsInputPsets, itsMarshalledData.itsInputPsetsSize * sizeof(unsigned));

  itsOutputPsets.resize(itsMarshalledData.itsOutputPsetsSize);
  memcpy(&itsOutputPsets[0], itsMarshalledData.itsOutputPsets, itsMarshalledData.itsOutputPsetsSize * sizeof(unsigned));

  itsTabList.resize(itsMarshalledData.itsTabListSize);
  memcpy(&itsTabList[0], itsMarshalledData.itsTabList, itsMarshalledData.itsTabListSize * sizeof(unsigned));

  itsUsedCoresInPset.resize(itsMarshalledData.itsNrUsedCoresPerPset);
  memcpy(&itsUsedCoresInPset[0], itsMarshalledData.itsUsedCoresInPset, itsMarshalledData.itsNrUsedCoresPerPset * sizeof(unsigned));

  itsRefFreqs.resize(itsMarshalledData.itsRefFreqsSize);
  memcpy(&itsRefFreqs[0], itsMarshalledData.itsRefFreqs, itsMarshalledData.itsRefFreqsSize * sizeof(double));

  itsRefPhaseCentre.resize(3);
  memcpy(&itsRefPhaseCentre[0], itsMarshalledData.itsRefPhaseCentre, 3 * sizeof(double));

  itsPhaseCentres.resize(nrStations(),3);
  for( unsigned stat = 0; stat < nrStations(); stat++ ) {
    memcpy(&itsPhaseCentres[stat][0], &itsMarshalledData.itsPhaseCentres[stat*3], 3 * sizeof(double));
  }
}


void CN_Configuration::write(Stream *str)
{
  itsMarshalledData.itsInputPsetsSize = itsInputPsets.size();
  assert(itsMarshalledData.itsInputPsetsSize <= MAX_PSETS);
  memcpy(itsMarshalledData.itsInputPsets, &itsInputPsets[0], itsMarshalledData.itsInputPsetsSize * sizeof(unsigned));

  itsMarshalledData.itsOutputPsetsSize = itsOutputPsets.size();
  assert(itsMarshalledData.itsOutputPsetsSize <= MAX_PSETS);
  memcpy(itsMarshalledData.itsOutputPsets, &itsOutputPsets[0], itsMarshalledData.itsOutputPsetsSize * sizeof(unsigned));

  itsMarshalledData.itsTabListSize = itsTabList.size();
  assert(itsMarshalledData.itsTabListSize <= MAX_PSETS);
  memcpy(itsMarshalledData.itsTabList, &itsTabList[0], itsMarshalledData.itsTabListSize * sizeof(unsigned));

  itsMarshalledData.itsNrUsedCoresPerPset = itsUsedCoresInPset.size();
  assert(itsMarshalledData.itsNrUsedCoresPerPset <= MAX_CORES_PER_PSET);
  memcpy(itsMarshalledData.itsUsedCoresInPset, &itsUsedCoresInPset[0], itsMarshalledData.itsNrUsedCoresPerPset * sizeof(unsigned));

  itsMarshalledData.itsRefFreqsSize = itsRefFreqs.size();
  assert(itsMarshalledData.itsRefFreqsSize <= MAX_SUBBANDS);
  memcpy(itsMarshalledData.itsRefFreqs, &itsRefFreqs[0], itsMarshalledData.itsRefFreqsSize * sizeof(double));

  memcpy(itsMarshalledData.itsRefPhaseCentre, &itsRefPhaseCentre[0], 3 * sizeof(double));
  for( unsigned stat = 0; stat < nrStations(); stat++ ) {
    memcpy(&itsMarshalledData.itsPhaseCentres[stat*3], &itsPhaseCentres[stat][0], 3 * sizeof(double));
  }

  str->write(&itsMarshalledData, sizeof itsMarshalledData);
}

unsigned CN_Configuration::nrMergedStations()
{
  if( tabList().empty() ) {
    return nrStations();
  }

  return *std::max_element( tabList().begin(), tabList().end() ) + 1;
}

} // namespace RTCP
} // namespace LOFAR
