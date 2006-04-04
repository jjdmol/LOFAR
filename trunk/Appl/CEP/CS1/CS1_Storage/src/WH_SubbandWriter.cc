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

// Application specific includes
#include <CS1_Storage/WH_SubbandWriter.h>
#include <CS1_Interface/DH_Visibilities.h>
#include <CS1_Storage/MSWriter.h>
#include <tinyCEP/Sel_RoundRobin.h>

#ifdef USE_MAC_PI
#include <GCF/GCF_PVDouble.h>
#include <GCF/GCF_PVString.h>
#endif

using namespace LOFAR;

WH_SubbandWriter::WH_SubbandWriter(const string& name, int subbandID,
		 		   const ACC::APS::ParameterSet& pset) 
  : WorkHolder    (pset.getInt32("BGLProc.SlavesPerSubband"), 
		   0,
		   name,
		   "WH_SubbandWriter"),
    itsSubbandID  (subbandID),
    itsPS         (pset),
    itsWriter     (0),
    itsBandId     (-1),
    itsTimeCounter(0),
    itsWriteTimer ("writing-MS")
#ifdef USE_MAC_PI
    ,itsPropertySet(0)
#endif
{

#ifdef USE_MAC_PI
  itsWriteToMAC = itsPS.getBool("Storage.WriteToMAC");
#endif
  itsNStations = itsPS.getInt32("FakeData.NStations");
  itsNBaselines = itsNStations * (itsNStations +1)/2;
  itsNChannels = itsPS.getInt32("Observation.NChannels");
  itsNInputsPerSubband = itsNinputs;
  int pols = itsPS.getInt32("Observation.NPolarisations");
  itsNPolSquared = pols*pols;

  int nrSamples = itsPS.getInt32("Observation.NSubbandSamples");
  itsWeightFactor = (float)itsNChannels/(float)nrSamples;  // The inverse of maximum number of valid samples

  vector<double> refFreqs= itsPS.getDoubleVector("Observation.RefFreqs");
  ASSERTSTR(refFreqs.size() >= itsPS.getInt32("Observation.NSubbands"), 
	    "Wrong number of refFreqs specified!");
  for (int i = refFreqs.size(); i < itsPS.getInt32("FakeData.NSubbands"); i++) {
    refFreqs.push_back(1);
  }

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

WorkHolder* WH_SubbandWriter::construct(const string& name, int subbandID,
					const ACC::APS::ParameterSet& pset)
{
  return new WH_SubbandWriter(name, subbandID, pset);
}

WH_SubbandWriter* WH_SubbandWriter::make(const string& name)
{
  return new WH_SubbandWriter(name, itsSubbandID, itsPS);
}

void WH_SubbandWriter::preprocess() {
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
  double startTime = itsPS.getDouble("Observation.StartTime");
  double timeStep = itsPS.getDouble("Observation.SampleRate") * itsPS.getDouble("Observation.NSubbandSamples");
  vector<double> antPos = itsPS.getDoubleVector("Observation.StationPositions");

  for (int i = antPos.size(); i < 3 * itsNStations; i++) {
    antPos.push_back(1);
  }

  itsWriter = new MSWriter(msName.c_str(), startTime, timeStep, itsNChannels, 
			   itsNPolSquared, itsNStations, antPos);

  double chanWidth = itsPS.getDouble("Observation.ChanWidth");
  vector<double> refFreqs= itsPS.getDoubleVector("Observation.RefFreqs");
  // Add the subband
  itsBandId = itsWriter->addBand (itsNPolSquared, itsNChannels,
				  refFreqs[itsSubbandID], chanWidth);
  vector<double> beamDirections = itsPS.getDoubleVector("Observation.BeamDirections");
  double RA = beamDirections[0];
  double DEC = beamDirections[1];
  // For nr of beams
  itsFieldId = itsWriter->addField (RA, DEC);

  // Allocate buffers
  bool* flagPtr = (bool*)itsFlagsBuffer;
  for (int i=0; i < itsNBaselines*itsNChannels*itsNPolSquared; i++)
  {
      flagPtr[i] = false;
  }
  float* weightPtr = (float*)itsWeightsBuffer;
  for (int j=0; j < itsNBaselines*itsNChannels; j++)
  {
      weightPtr[j] = 1;
  }
}

void WH_SubbandWriter::process() 
{
  // Select the next input
  DH_Visibilities* inputDH = (DH_Visibilities*)getDataManager().selectInHolder();

  int dataSize = inputDH->getBufSize();
    
  // Write 1 DH_Visibilities of size fcomplex[nbaselines][nsubbandchannesl][npol][npol]
  itsWriteTimer.start();
    
  DH_Visibilities::NrValidSamplesType* valSamples = inputDH->getNrValidSamplesCounted();
  for (int bl=0; bl < itsNBaselines; bl++)
  {
    for (int ch=0; ch < itsNChannels; ch++)
    {
      itsWeightsBuffer[bl][ch] = itsWeightFactor * (float)((*valSamples)[bl][ch]);
      if ((*valSamples)[bl][ch] == 0)
      {
	itsFlagsBuffer[bl][ch][0][0] = true;
	itsFlagsBuffer[bl][ch][0][1] = true;
	itsFlagsBuffer[bl][ch][1][0] = true;
	itsFlagsBuffer[bl][ch][1][1] = true;
      }
      else
      {
	itsFlagsBuffer[bl][ch][0][0] = false;
	itsFlagsBuffer[bl][ch][0][1] = false;
	itsFlagsBuffer[bl][ch][1][0] = false;
	itsFlagsBuffer[bl][ch][1][1] = false;
      }
    }
  }
  
  itsWriter->write (itsBandId, itsFieldId, 0, itsNChannels,
		    itsTimeCounter, dataSize,
		    (fcomplex*)(inputDH->getVisibilities()), (bool*)itsFlagsBuffer,
		    (float*)itsWeightsBuffer);
  
  itsWriteTimer.stop();

  itsTimeCounter++;

#ifdef USE_MAC_PI
//   if (itsWriteToMAC) {
//     DBGASSERTSTR(itsPropertySet != 0, "no propertySet constructed yet");
//     LOG_TRACE_FLOW("WH_SubbandWriter setting properties");
//     GCF::Common::GCFPValueArray::iterator it;
//     for (it = itsVArray.begin(); it != itsVArray.end(); it++){
//       delete *it;
//     }
//     itsVArray.clear();
    
//     // loop over values
//     for (int i=0; i<itsNinputs; i++) {
//       inputDH = (DH_VisArray*)getDataManager().getInHolder(i);
//       // loop over channels
//       for (uint ch = 0; ch < inputDH->getNumVis(); ch++)
//       {
// 	// loop over baselines
// 	for (int s1 = 0; s1 < itsNstations; s1++) {
// 	  for (int s2 = 0; s2 <= s1; s2++) {
// 	    for (int p = 0; p < itsNpolSquared; p++) {
// 	      itsVArray.push_back(new GCF::Common::GCFPVDouble((double)*inputDH->getBufferElement(ch, s1, s2, p)));
// 	    }
// 	  }
// 	}
//       }
//     }

//     (*itsPropertySet)["data"].setValue(GCF::Common::GCFPVDynArr(GCF::Common::LPT_DOUBLE, itsVArray));
//     (*itsPropertySet)["subband"].setValue(GCF::Common::GCFPVString("1"));
//     LOG_TRACE_FLOW("WH_SubbandWriter properties set");
//   };
#endif

}

void WH_SubbandWriter::postprocess() {
  cout<<itsWriteTimer<<endl;
}
