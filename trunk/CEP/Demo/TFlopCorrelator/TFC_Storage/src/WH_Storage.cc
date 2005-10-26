//#  WH_Storage.cc: Writes visibilities in an AIPS++ measurement set
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
#include <TFC_Storage/WH_Storage.h>
#include <TFC_Interface/DH_VisArray.h>
#include <TFC_Storage/MSWriter.h>
#ifdef USE_MAC_PI
#include <GCF/GCF_PVDouble.h>
#include <GCF/GCF_PVString.h>
#endif

using namespace LOFAR;

WH_Storage::WH_Storage(const string& name, 
		       const ACC::APS::ParameterSet& pset) 
  : WorkHolder (pset.getInt32("BGLProc.NrStoredSubbands"),   // number of correlator outputs
		0,
		name,
		"WH_Storage"),
    itsPS      (pset),
    itsWriter  (0),
    itsCounter (0)
#ifdef USE_MAC_PI
    ,itsPropertySet(0)
#endif
{
#ifdef USE_MAC_PI
  itsWriteToMAC = itsPS.getBool("Storage.WriteToMAC");
#endif
  itsNstations = itsPS.getInt32("PPF.NrStations");
  itsNChannels = itsPS.getInt32("PPF.NrSubChannels");
  itsNCorrPerFilt = itsPS.getInt32("PPF.NrCorrelatorsPerFilter");
  itsNChanPerVis = itsNChannels/itsNCorrPerFilt;
  int pols = itsPS.getInt32("Input.NPolarisations");
  itsNpolSquared = pols*pols;

  vector<double> refFreqs= itsPS.getDoubleVector("Storage.refFreqs");
  ASSERTSTR(refFreqs.size() == itsPS.getInt32("BGLProc.NrStoredSubbands"), 
	    "Wrong number of refFreqs specified!");
  char str[32];
  for (int i=0; i<itsNinputs; i++) {
    sprintf(str, "DH_in_%d", i);
    getDataManager().addInDataHolder(i, new DH_VisArray(str, pset));
  }
 }

WH_Storage::~WH_Storage() 
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

WorkHolder* WH_Storage::construct(const string& name,
				  const ACC::APS::ParameterSet& pset)
{
  return new WH_Storage(name, pset);
}

WH_Storage* WH_Storage::make(const string& name)
{
  return new WH_Storage(name, itsPS);
}

void WH_Storage::preprocess() {
  LOG_TRACE_FLOW("WH_Storage enabling PropertySet");
#ifdef USE_MAC_PI
  if (itsWriteToMAC) {
    itsPropertySet = new GCF::CEPPMLlight::CEPPropertySet("CEP_TFCD", "TTeraFlopCorrelator", GCF::Common::PS_CAT_PERMANENT);
    itsPropertySet->enable();
    LOG_TRACE_FLOW("WH_Storage PropertySet enabled");
  } else {
    LOG_TRACE_FLOW("WH_Storage PropertySet not enabled");
  };
#endif

  // create MSWriter object
  string msName = itsPS.getString("Storage.MSName");
  double startTime = itsPS.getDouble("Storage.startTime");
  double timeStep = itsPS.getDouble("Storage.timeStep");
  int nPolarisations = itsPS.getInt32("Input.NPolarisations");
  uint nAntennas = itsNstations;
  vector<double> antPos = itsPS.getDoubleVector("Storage.stationPositions");
  itsWriter = new MSWriter(msName.c_str(), startTime, timeStep, itsNChannels, 
			   nPolarisations*nPolarisations, nAntennas, antPos);

  double chanWidth = itsPS.getDouble("Storage.chanWidth");
  vector<double> refFreqs= itsPS.getDoubleVector("Storage.refFreqs");
  vector<double>::iterator iter;
  // Add the subbands
  for (iter=refFreqs.begin(); iter!=refFreqs.end(); iter++)
  {
    int bandId = itsWriter->addBand (nPolarisations*nPolarisations, itsNChannels,
				     *iter, chanWidth);
    itsBandIds.push_back(bandId);
  }
  double azimuth = itsPS.getDouble("Storage.beamAzimuth");
  double elevation = itsPS.getDouble("Storage.beamElevation");
  double pi = itsPS.getDouble("Storage.pi");
  // For nr of beams
  itsFieldId = itsWriter->addField (azimuth*pi/180., elevation*pi/180.);
}

void WH_Storage::process() 
{
  // loop over all inputs
  // It is assumed each input DH_VisArray contains all frequency channels for 1 subband
  // and the data is ordered in ascending frequency.
  DH_VisArray* inputDH = 0;
  for (int i=0; i<itsNinputs; i++)   // Loop over subbands/inputs
  {
    inputDH = (DH_VisArray*)getDataManager().getInHolder(i);

    int rownr = -1;           // Set rownr -1 when new rows need to be added
                              // when writing a new subband
    int dataSize = (inputDH->getBufSize())/(inputDH->getNumVis());
    
    for (uint v=0; v < inputDH->getNumVis(); v++)  // Loop over "DH_Vis"s
    {
	// Check if channel frequency is ascending. 
	if (v > 0) {	
	  //	  DBGASSERT(inputDH->getCenterFreq(v) > inputDH->getCenterFreq(v-1)); 
	}
	// Write 1 DH_Vis size fcomplex[nbaselines][nChannelsPerVis][npol][npol]
	itsWriter->write (rownr, itsBandIds[i], itsFieldId, v*itsNChanPerVis, itsNChanPerVis,
			  itsCounter, dataSize,
			  inputDH->getBufferElement(v));   // To do: add flags
    }
  }

  itsCounter++;

#ifdef USE_MAC_PI
//   if (itsWriteToMAC) {
//     DBGASSERTSTR(itsPropertySet != 0, "no propertySet constructed yet");
//     LOG_TRACE_FLOW("WH_Storage setting properties");
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
//     LOG_TRACE_FLOW("WH_Storage properties set");
//   };
#endif
}
