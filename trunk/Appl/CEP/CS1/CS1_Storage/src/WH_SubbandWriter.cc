
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
                                       const ACC::APS::ParameterSet& pset) 
      : WorkHolder    (pset.getUint32("BGLProc.NodesPerPset") * pset.getUint32("BGLProc.PsetsPerCell") * pset.getUint32("Storage.PsetsPerStorage"), 
                       0,
                       name,
                       "WH_SubbandWriter"),
        itsSubbandIDs  (subbandID),
        itsPS         (pset),
        itsWriter     (0),
        itsTimeCounter(0),
	itsTimesToIntegrate(1),
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
      itsNStations = itsPS.getUint32("Observation.NStations");
      itsNBaselines = itsNStations * (itsNStations +1)/2;
      itsNChannels = itsPS.getUint32("Observation.NChannels");
      itsNBeams = itsPS.getUint32("Observation.NBeams");
      uint pols = itsPS.getUint32("Observation.NPolarisations");
      itsNPolSquared = pols*pols;

      uint nrSamples = itsPS.getUint32("Observation.NSubbandSamples");
      itsWeightFactor = (float)itsNChannels/(float)nrSamples;  // The inverse of maximum number of valid samples
      
      vector<double> refFreqs= itsPS.getDoubleVector("Observation.RefFreqs");
      unsigned nrSubbands = itsPS.getUint32("Observation.NSubbands");
      ASSERTSTR(refFreqs.size() >= nrSubbands, "Wrong number of refFreqs specified!");

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
      delete itsWriter;
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
                                            const ACC::APS::ParameterSet& pset)
    {
      return new WH_SubbandWriter(name, subbandIDs, pset);
    }

    WH_SubbandWriter* WH_SubbandWriter::make(const string& name)
    {
      return new WH_SubbandWriter(name, itsSubbandIDs, itsPS);
    }

    void WH_SubbandWriter::preprocess() 
    {
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

      // create MSWriter object
      vector<string> msNames = itsPS.getStringVector("Storage.MSNames");

#if 0
      // split name in base part and extension (if any)
      string::size_type idx = msName.rfind('.');
      pair<string,string> ms(msName.substr(0,idx),
                             idx == string::npos ? "" : msName.substr(idx));
      // insert subband-id into the name
      ostringstream oss;
      oss << '.' << setfill('0') << setw(3) << itsSubbandID;
      // rebuild the MS name string
      msName = ms.first + oss.str() + ms.second;
#endif
      
#if defined HAVE_MPI
      LOG_TRACE_VAR_STR("Creating MS-file \"" << msNames[TH_MPI::getCurrentRank()] << "\"");
#else
      LOG_TRACE_VAR_STR("Creating MS-file \"" << msNames[0] << "\"");
#endif
      
      double startTime = itsPS.getDouble("Observation.StartTime");
      LOG_TRACE_VAR_STR("startTime = " << startTime);
      
      itsTimesToIntegrate = itsPS.getInt32("Storage.IntegrationTime");
      double timeStep = itsPS.getDouble("Observation.NSubbandSamples") / 
                        itsPS.getDouble("Observation.SampleRate");
      LOG_TRACE_VAR_STR("timeStep = " << timeStep);
      
      vector<double> antPos = itsPS.getDoubleVector("Observation.StationPositions");
      ASSERTSTR(antPos.size() == 3 * itsNStations,
                antPos.size() << " == " << 3 * itsNStations);
      
      itsNrSubbandsPerCell    = itsPS.getUint32("General.SubbandsPerPset") * itsPS.getUint32("BGLProc.PsetsPerCell");
      itsNrSubbandsPerStorage = itsNrSubbandsPerCell * itsPS.getUint32("Storage.PsetsPerStorage");
      itsNrNodesPerCell       = itsPS.getUint32("BGLProc.NodesPerPset") * itsPS.getUint32("BGLProc.PsetsPerCell");
      itsCurrentInputs.resize(itsNrSubbandsPerStorage / itsNrSubbandsPerCell, 0);

      LOG_TRACE_VAR_STR("SubbandsPerStorage = " << itsNrSubbandsPerStorage);

      vector<string> storageStationNames = itsPS.getStringVector("Storage.StorageStationNames");
#if defined HAVE_MPI
      itsWriter = new MSWriter(msNames[TH_MPI::getCurrentRank()].c_str(),
#else
      itsWriter = new MSWriter(msNames[0].c_str(),
#endif
			       startTime, timeStep * itsTimesToIntegrate, 
                               itsNChannels, itsNPolSquared, 
			       itsNBeams,
			       itsNStations, 
                               antPos, storageStationNames, itsTimesToIntegrate, 
			       itsPS.getUint32("General.SubbandsPerPset") * itsPS.getUint32("Storage.PsetsPerStorage"));
			       
      double chanWidth = itsPS.getDouble("Observation.SampleRate") /
			 itsPS.getDouble("Observation.NChannels");
      LOG_TRACE_VAR_STR("chanWidth = " << chanWidth);
      
      vector<double> refFreqs= itsPS.getDoubleVector("Observation.RefFreqs");

      // Now we must add \a itsNrSubbandsPerStorage to the measurement set. The
      // correct indices for the reference frequencies are in the vector of
      // subbandIDs.
      itsBandIDs.resize(itsNrSubbandsPerStorage);
      for (uint sb = 0; sb < itsNrSubbandsPerStorage; ++sb) {
	// compensate for the half-channel shift introduced by the PPF
	double refFreq = refFreqs[itsSubbandIDs[sb]] - chanWidth / 2;
        itsBandIDs[sb] = itsWriter->addBand (itsNPolSquared, itsNChannels,
                                             refFreq, chanWidth);
      }
      
      //## TODO: add support for more than 1 beam ##//
      vector<double> beamDirs = itsPS.getDoubleVector("Observation.BeamDirections");
      ASSERT(beamDirs.size() == 2 * itsNBeams);
      double RA = beamDirs[0];
      double DEC = beamDirs[1];
      // For nr of beams
      itsFieldID = itsWriter->addField (RA, DEC);

      // Allocate buffers
      itsFlagsBuffers   = new bool[itsNrSubbandsPerStorage * itsNVisibilities];
      itsWeightsBuffers = new float[itsNrSubbandsPerStorage * itsNBaselines * itsNChannels];
      itsVisibilities   = new DH_Visibilities::VisibilityType[itsNrSubbandsPerStorage * itsNVisibilities];

      clearAllSums();
    }

    void WH_SubbandWriter::clearAllSums(){
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

      if (itsTimeCounter % itsTimesToIntegrate == 0) {
	clearAllSums();
      }

      // Write the visibilities for all subbands per cell.
      for (uint sb = 0; sb < itsNrSubbandsPerStorage; ++sb) {
        // find out from which input channel we should read
	unsigned cell	      = sb / itsNrSubbandsPerCell;
	unsigned inputChannel = itsCurrentInputs[cell] + cell * itsNrNodesPerCell;

	DH_Visibilities			    *inputDH	= static_cast<DH_Visibilities *>(getDataManager().getInHolder(inputChannel));
        DH_Visibilities::NrValidSamplesType *valSamples = &inputDH->getNrValidSamples(0, 0);
	DH_Visibilities::VisibilityType     *newVis	= &inputDH->getVisibility(0, 0, 0, 0);

        // Write 1 DH_Visibilities of size
        // fcomplex[itsNBaselines][itsNChannels][npol][npol]

        for (uint i = 0; i < itsNBaselines * itsNChannels; i ++) {
          itsWeightsBuffers[sb * itsNBaselines * itsNChannels + i] += itsWeightFactor * valSamples[i] / itsTimesToIntegrate;
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
	  itsWriter->write (itsBandIDs[sb], itsFieldID, 0, itsNChannels,
			    itsTimeCounter, itsNVisibilities,
			    &(itsVisibilities[sb * itsNVisibilities]),
			    &(itsFlagsBuffers[sb * itsNVisibilities]), 
			    &(itsWeightsBuffers[sb * itsNBaselines * itsNChannels]));
	  itsWriteTimer.stop();
	}

	getDataManager().readyWithInHolder(inputChannel);

	// select next channel
	if (++ itsCurrentInputs[cell] == itsNrNodesPerCell)
	  itsCurrentInputs[cell] = 0;
      }

      // Update the time counter.
      itsTimeCounter++;
    }


    void WH_SubbandWriter::postprocess() 
    {
      delete [] itsFlagsBuffers;
      delete [] itsWeightsBuffers;
      delete itsWriter;
      itsFlagsBuffers = 0;
      itsWeightsBuffers = 0;
      cout<<itsWriteTimer<<endl;
    }
    
  } // namespace CS1

} // namespace LOFAR
