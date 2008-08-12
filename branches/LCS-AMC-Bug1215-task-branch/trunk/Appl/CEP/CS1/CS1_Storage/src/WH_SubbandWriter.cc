

//#  WH_SubbandWriter.cc: Writes visibilities in an AIPS++ measurement set
//#
//#  Copyright (C) 2002-2005
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

// General includes
#include <Common/LofarLogger.h>
#include <APS/ParameterSet.h>
#include <Common/lofar_iomanip.h>

// Application specific includes
#include <CS1_Storage/WH_SubbandWriter.h>
#include <CS1_Interface/DH_Visibilities.h>
#include <CS1_Storage/MSWriter.h>
#include <tinyCEP/Sel_RoundRobin.h>
#include <Transport/TH_MPI.h>

#ifdef USE_MAC_PI
#include <GCF/GCF_PVDouble.h>
#include <GCF/GCF_PVString.h>
#endif

#include <time.h>

namespace LOFAR
{
  namespace CS1
  {

    WH_SubbandWriter::WH_SubbandWriter(const string& name, 
                                       const vector<uint>& subbandID,
				       CS1_Parset *pset) 
      : WorkHolder(pset->nrPsetsPerStorage(), 0, name, "WH_SubbandWriter"),
        itsCS1PS(pset),
        itsSubbandIDs(subbandID),
        itsTimeCounter(0),
	itsTimesToIntegrate(pset->storageIntegrationSteps()),
        itsFlagsBuffers(0),
        itsWeightsBuffers(0),
	itsVisibilities(0),
        itsWriteTimer ("writing-MS")
#ifdef USE_MAC_PI
      ,itsPropertySet(0)
#endif
    {
#ifdef USE_MAC_PI
      itsWriteToMAC = itsPS.getBool("Storage.WriteToMAC");
#endif
      itsNStations = itsCS1PS->nrStations();
      itsNBaselines = itsNStations * (itsNStations +1)/2;
      itsNChannels = itsCS1PS->nrChannelsPerSubband();
      itsNBeams = itsCS1PS->nrBeams();
      uint pols = itsCS1PS->getUint32("Observation.nrPolarisations");
      itsNPolSquared = pols*pols;

      // itsWeightFactor = the inverse of maximum number of valid samples
      itsWeightFactor = 1.0 / (pset->BGLintegrationSteps() * pset->IONintegrationSteps() * pset->storageIntegrationSteps());
      
      itsNVisibilities = itsNBaselines * itsNChannels * itsNPolSquared;

      char str[32];
      for (int i=0; i<itsNinputs; i++) {
        sprintf(str, "DH_in_%d", i);
        getDataManager().addInDataHolder(i, new DH_Visibilities(str, pset));
        getDataManager().setAutoTriggerIn(i, false);
      }
    }

    WH_SubbandWriter::~WH_SubbandWriter() 
    {
#if defined HAVE_AIPSPP
      for (unsigned i = 0; i < itsWriters.size(); i ++)
	delete itsWriters[i];

      itsWriters.clear();
#endif

#ifdef USE_MAC_PI
      delete itsPropertySet;

      GCF::Common::GCFPValueArray::iterator it;
      for (it = itsVArray.begin(); it != itsVArray.end(); it++){
        delete *it;
      }
      itsVArray.clear();
#endif
    }

    WorkHolder* WH_SubbandWriter::construct(const string& name,
                                            const vector<uint>& subbandIDs,
					          CS1_Parset *pset)
    {
      return new WH_SubbandWriter(name, subbandIDs, pset);
    }

    WH_SubbandWriter* WH_SubbandWriter::make(const string& name)
    {
      return new WH_SubbandWriter(name, itsSubbandIDs, itsCS1PS);
    }

    void WH_SubbandWriter::preprocess() 
    {
#if defined HAVE_AIPSPP
      LOG_TRACE_FLOW("WH_SubbandWriter enabling PropertySet");
#ifdef USE_MAC_PI
      if (itsWriteToMAC) {
        itsPropertySet = new GCF::CEPPMLlight::CEPPropertySet("CEP_TFCD", "TTeraFlopCorrelator", GCF::Common::PS_CAT_PERMANENT);
        itsPropertySet->enable();
        LOG_TRACE_FLOW("WH_SubbandWriter PropertySet enabled");
      } else {
        LOG_TRACE_FLOW("WH_SubbandWriter PropertySet not enabled");
      };
#endif

      double startTime = itsCS1PS->startTime();
      LOG_TRACE_VAR_STR("startTime = " << startTime);
      
      vector<double> antPos = itsCS1PS->positions();
      ASSERTSTR(antPos.size() == 3 * itsNStations,
                antPos.size() << " == " << 3 * itsNStations);
      itsNrSubbandsPerPset	= itsCS1PS->nrSubbandsPerPset();
      itsNrSubbandsPerStorage	= itsNrSubbandsPerPset * itsCS1PS->nrPsetsPerStorage();
      LOG_TRACE_VAR_STR("SubbandsPerStorage = " << itsNrSubbandsPerStorage);
      vector<string> storageStationNames = itsCS1PS->getStringVector("OLAP.storageStationNames");

      unsigned mssesPerStorage = itsCS1PS->getUint32("OLAP.subbandsPerPset") * itsCS1PS->getUint32("OLAP.psetsPerStorage");
      itsWriters.resize(mssesPerStorage);
      
#if 0
      vector<int32>  bl2beams = itsCS1PS->beamlet2beams();
      vector<uint32> sb2Index = itsCS1PS->subband2Index();
#else
      vector<unsigned> subbandToBeamMapping = itsCS1PS->subbandToBeamMapping();
#endif
      
      for (unsigned i = 0; i < mssesPerStorage; i ++) {
#if defined HAVE_MPI
	unsigned currentSubband = (TH_MPI::getCurrentRank() * mssesPerStorage + i);
#else
	unsigned currentSubband = i;
#endif
	itsWriters[i] = new MSWriter(
	  itsCS1PS->getMSname(currentSubband).c_str(),
	  startTime, itsCS1PS->storageIntegrationTime(), itsNChannels,
	  itsNPolSquared, itsNStations, antPos,
	  storageStationNames, itsTimesToIntegrate);

#if 0
        vector<double> beamDir = itsCS1PS->getBeamDirection(bl2beams[sb2Index[currentSubband]]);
	itsWriters[i]->addField(beamDir[0], beamDir[1], bl2beams[sb2Index[currentSubband]]);
#else
	unsigned       beam    = subbandToBeamMapping[currentSubband];
        vector<double> beamDir = itsCS1PS->getBeamDirection(beam);
	itsWriters[i]->addField(beamDir[0], beamDir[1], beam); // FIXME add 1???
#endif
      }

      vector<double> refFreqs = itsCS1PS->subbandToFrequencyMapping();

      // Now we must add \a itsNrSubbandsPerStorage to the measurement set. The
      // correct indices for the reference frequencies are in the vector of
      // subbandIDs.      
      itsBandIDs.resize(itsNrSubbandsPerStorage);
      double chanWidth = itsCS1PS->channelWidth();
      LOG_TRACE_VAR_STR("chanWidth = " << chanWidth);

      for (uint sb = 0; sb < itsNrSubbandsPerStorage; ++sb) {
	// compensate for the half-channel shift introduced by the PPF
	double refFreq = refFreqs[itsSubbandIDs[sb]] - chanWidth / 2;
	itsBandIDs[sb] = itsWriters[sb]->addBand(itsNPolSquared, itsNChannels, refFreq, chanWidth);
      }
      
      // Allocate buffers
      if (itsTimesToIntegrate > 1) {
	itsFlagsBuffers   = new bool[itsNrSubbandsPerStorage * itsNVisibilities];
	itsWeightsBuffers = new float[itsNrSubbandsPerStorage * itsNBaselines * itsNChannels];
	itsVisibilities   = new DH_Visibilities::VisibilityType[itsNrSubbandsPerStorage * itsNVisibilities];
	clearAllSums();
      } else {
	itsFlagsBuffers   = new bool[itsNVisibilities];
	itsWeightsBuffers = new float[itsNBaselines * itsNChannels];
      }
#endif // defined HAVE_AIPSPP
    }

    void WH_SubbandWriter::clearAllSums()
    {
      assert(itsTimesToIntegrate > 1);
      memset(itsWeightsBuffers, 0, itsNrSubbandsPerStorage * itsNBaselines * itsNChannels * sizeof(float));
      memset(itsVisibilities, 0, itsNrSubbandsPerStorage * itsNVisibilities * sizeof(DH_Visibilities::VisibilityType));
      for (uint i = 0; i < itsNrSubbandsPerStorage * itsNVisibilities; i++) {
	itsFlagsBuffers[i] = true;
      }
    }

    void WH_SubbandWriter::process() 
    {
      static int counter = 0;
      time_t now = time(0);
      char buf[26];
      ctime_r(&now, buf);
      buf[24] = '\0';

      cout << "time = " << buf <<
#if defined HAVE_MPI
	      ", rank = " << TH_MPI::getCurrentRank() <<
#endif
	      ", count = " << counter ++ << endl;

#if defined HAVE_AIPSPP
      if (itsTimesToIntegrate > 1 && itsTimeCounter % itsTimesToIntegrate == 0) {
	clearAllSums();
      }
#endif

      // Write the visibilities for all subbands per pset.
      for (uint sb = 0; sb < itsNrSubbandsPerStorage; ++ sb) {
        // find out from which input channel we should read
	unsigned pset = sb / itsNrSubbandsPerPset;

	DH_Visibilities			    *inputDH	= static_cast<DH_Visibilities *>(getDataManager().getInHolder(pset));
        DH_Visibilities::NrValidSamplesType *valSamples = &inputDH->getNrValidSamples(0, 0);
  	DH_Visibilities::VisibilityType     *newVis	= &inputDH->getVisibility(0, 0, 0, 0);
       
        // Write 1 DH_Visibilities of size
        // fcomplex[itsNBaselines][itsNChannels][npol][npol]

#if defined HAVE_AIPSPP
	if (itsTimesToIntegrate > 1) {
	  for (uint i = 0; i < itsNBaselines * itsNChannels; i ++) {
	    itsWeightsBuffers[sb * itsNBaselines * itsNChannels + i] += itsWeightFactor * valSamples[i];
	    bool flagged = valSamples[i] == 0;
	    itsFlagsBuffers[sb * itsNVisibilities + 4 * i    ] &= flagged;
	    itsFlagsBuffers[sb * itsNVisibilities + 4 * i + 1] &= flagged;
	    itsFlagsBuffers[sb * itsNVisibilities + 4 * i + 2] &= flagged;
	    itsFlagsBuffers[sb * itsNVisibilities + 4 * i + 3] &= flagged;
	    // Currently we just add the samples, this way the time centroid stays in place
	    // We could also divide by the weight and multiple the sum by the total weight.
	    itsVisibilities[sb * itsNVisibilities + 4 * i    ] += newVis[4 * i    ];
	    itsVisibilities[sb * itsNVisibilities + 4 * i + 1] += newVis[4 * i + 1];
	    itsVisibilities[sb * itsNVisibilities + 4 * i + 2] += newVis[4 * i + 2];
	    itsVisibilities[sb * itsNVisibilities + 4 * i + 3] += newVis[4 * i + 3];
	  }

	  if ((itsTimeCounter + 1) % itsTimesToIntegrate == 0) {
	    itsWriteTimer.start();
	    itsWriters[sb]->write(itsBandIDs[sb],
	      0, itsNChannels, itsTimeCounter, itsNVisibilities,
	      &itsVisibilities[sb * itsNVisibilities],
	      &itsFlagsBuffers[sb * itsNVisibilities], 
	      &itsWeightsBuffers[sb * itsNBaselines * itsNChannels]);
	    itsWriteTimer.stop();
	  }
	} else {
	  for (uint i = 0; i < itsNBaselines * itsNChannels; i ++) {
	    itsWeightsBuffers[i] = itsWeightFactor * valSamples[i];
	    bool flagged = valSamples[i] == 0;
	    itsFlagsBuffers[4 * i    ] = flagged;
	    itsFlagsBuffers[4 * i + 1] = flagged;
	    itsFlagsBuffers[4 * i + 2] = flagged;
	    itsFlagsBuffers[4 * i + 3] = flagged;
	  }

	  itsWriteTimer.start();
	  itsWriters[sb]->write(itsBandIDs[sb],
	    0, itsNChannels, itsTimeCounter, itsNVisibilities,
	    newVis, itsFlagsBuffers, itsWeightsBuffers);
	  itsWriteTimer.stop();
	}
#endif

	getDataManager().readyWithInHolder(pset);
      }

      // Update the time counter.
      itsTimeCounter++;
    }


    void WH_SubbandWriter::postprocess() 
    {
      delete [] itsFlagsBuffers;	itsFlagsBuffers   = 0;
      delete [] itsWeightsBuffers;	itsWeightsBuffers = 0;

#if defined HAVE_AIPSPP
      for (unsigned i = 0; i < itsWriters.size(); i ++)
	delete itsWriters[i];

      itsWriters.clear();
#endif
      cout<<itsWriteTimer<<endl;
    }
    
  } // namespace CS1

} // namespace LOFAR
