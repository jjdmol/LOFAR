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

#ifdef USE_MAC_PI
#include <GCF/GCF_PVDouble.h>
#include <GCF/GCF_PVString.h>
#endif

namespace LOFAR
{
  namespace CS1
  {

    WH_SubbandWriter::WH_SubbandWriter(const string& name, 
                                       const vector<uint>& subbandID,
                                       const ACC::APS::ParameterSet& pset) 
      : WorkHolder    (pset.getUint32("BGLProc.NodesPerCell"), 
                       0,
                       name,
                       "WH_SubbandWriter"),
        itsSubbandIDs  (subbandID),
        itsPS         (pset),
        itsWriter     (0),
        itsTimeCounter(0),
        itsFlagsBuffer(0),
        itsWeightsBuffer(0),
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
      itsNInputsPerSubband = itsNinputs;
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
      }

      // Set a round robin input selector
      getDataManager().setInputSelector(new Sel_RoundRobin(itsNinputs));
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
      string msName = itsPS.getString("Storage.MSName");

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
      
      LOG_TRACE_VAR_STR("Creating MS-file \"" << msName << "\"");
      
      double startTime = itsPS.getDouble("Observation.StartTime");
      LOG_TRACE_VAR_STR("startTime = " << startTime);
      
      double timeStep = itsPS.getDouble("Observation.NSubbandSamples") / 
                        itsPS.getDouble("Observation.SampleRate");
      LOG_TRACE_VAR_STR("timeStep = " << timeStep);
      
      vector<double> antPos = itsPS.getDoubleVector("Observation.StationPositions");
      ASSERTSTR(antPos.size() == 3 * itsNStations,
                antPos.size() << " == " << 3 * itsNStations);
      
      itsWriter = new MSWriter(msName.c_str(), startTime, timeStep, 
                               itsNChannels, itsNPolSquared, itsNStations, 
                               antPos);

      double chanWidth = itsPS.getDouble("Observation.SampleRate") /
			 itsPS.getDouble("Observation.NChannels");
      LOG_TRACE_VAR_STR("chanWidth = " << chanWidth);
      
      vector<double> refFreqs= itsPS.getDoubleVector("Observation.RefFreqs");

      // Here we should (somehow) derive which subband we're going to write.
      // At least we know how many subbands we can expect, because that's in
      // the parameter set file.
      itsNrSubbandsPerCell = itsPS.getUint32("General.SubbandsPerCell");
      LOG_TRACE_VAR_STR("General.SubbandsPerCell = " << itsNrSubbandsPerCell);

      // Now we must add \a itsNrSubbandsPerCell to the measurement set. The
      // correct indices for the reference frequencies are in the vector of
      // subbandIDs.
      itsBandIDs.resize(itsNrSubbandsPerCell);
      for (uint sb = 0; sb < itsNrSubbandsPerCell; ++sb) {
        itsBandIDs[sb] = itsWriter->addBand (itsNPolSquared, itsNChannels,
                                             refFreqs[itsSubbandIDs[sb]], 
                                             chanWidth);
      }
      
      //## TODO: add support for more than 1 beam ##//
      vector<double> beamDirs = itsPS.getDoubleVector("Observation.BeamDirections");
      ASSERT(beamDirs.size() == 2 * itsPS.getUint32("Observation.NBeams"));
      double RA = beamDirs[0];
      double DEC = beamDirs[1];
      // For nr of beams
      itsFieldID = itsWriter->addField (RA, DEC);

      // Allocate buffers
      itsFlagsBuffer   = new bool[itsNVisibilities];
      itsWeightsBuffer = new float[itsNBaselines * itsNChannels];

#if 0
      memset(itsFlagsBuffer, 0, itsNVisibilities * sizeof(bool));
      for (uint j=0; j < itsNBaselines*itsNChannels; j++)
      {
        itsWeightsBuffer[j] = 1;
      }
#endif
    }

    void WH_SubbandWriter::process() 
    {
      // Write the visibilities for all subbands per cell.
      for (uint sb = 0; sb < itsNrSubbandsPerCell; ++sb) {
        
        // Select the next input
        DH_Visibilities* inputDH = 
          (DH_Visibilities*)getDataManager().selectInHolder();

        // Write 1 DH_Visibilities of size
        // fcomplex[nbaselines][nsubbandchannesl][npol][npol]
        DH_Visibilities::NrValidSamplesType *valSamples = 
          &inputDH->getNrValidSamples(0, 0);

        for (uint i = 0; i < itsNBaselines * itsNChannels; i ++) {
          itsWeightsBuffer[i] = itsWeightFactor * valSamples[i];
          bool flagged = valSamples[i] == 0;
          itsFlagsBuffer[4 * i    ] = flagged;
          itsFlagsBuffer[4 * i + 1] = flagged;
          itsFlagsBuffer[4 * i + 2] = flagged;
          itsFlagsBuffer[4 * i + 3] = flagged;
        }
  
        itsWriteTimer.start();
        itsWriter->write (itsBandIDs[sb], itsFieldID, 0, itsNChannels,
                          itsTimeCounter, itsNVisibilities,
                          &inputDH->getVisibility(0, 0, 0, 0),
                          itsFlagsBuffer, itsWeightsBuffer);
        itsWriteTimer.stop();
      }

      // Update the time counter.
      itsTimeCounter++;
    }


    void WH_SubbandWriter::postprocess() 
    {
      delete [] itsFlagsBuffer;
      delete [] itsWeightsBuffer;
      delete itsWriter;
      itsFlagsBuffer = 0;
      itsWeightsBuffer = 0;
      cout<<itsWriteTimer<<endl;
    }
    
  } // namespace CS1

} // namespace LOFAR
