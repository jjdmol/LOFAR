//#  WH_Storage.cc: 
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
#include <TFC_Interface/DH_Vis.h>
#include <TFC_Storage/MSWriter.h>
#include <GCF/GCF_PVDouble.h>
#include <GCF/GCF_PVString.h>

using namespace LOFAR;

WH_Storage::WH_Storage(const string& name, 
		       const ACC::APS::ParameterSet& pset) 
  : WorkHolder (pset.getInt32("Input.NSubbands"),        // for now number of correlator outputs
		0,                                  // is equal to number of subbands
		name,
		"WH_Storage"),
    itsPS      (pset),
    itsWriter  (0),
    itsCounter (0),
    itsPropertySet(0)
{
  itsWriteToMAC = itsPS.getBool("Storage.WriteToMAC");
  itsNstations = itsPS.getInt32("Input.NRSP");
  int pols = itsPS.getInt32("Input.NPolarisations");
  itsNpolSquared = pols*pols;

  vector<double> refFreqs= itsPS.getDoubleVector("Storage.refFreqs");
  ASSERTSTR(refFreqs.size() == itsPS.getInt32("Input.NSubbands"), 
	    "Wrong number of refFreqs specified!");
  char str[32];
  for (int i=0; i<itsNinputs; i++) {
    sprintf(str, "DH_in_%d", i);
    getDataManager().addInDataHolder(i, new DH_Vis(str, refFreqs[i], pset));
  }
 }

WH_Storage::~WH_Storage() 
{
  delete itsWriter;
  delete itsPropertySet;

  GCF::Common::GCFPValueArray::iterator it;
  for (it = itsVArray.begin(); it != itsVArray.end(); it++){
    delete *it;
  }
  itsVArray.clear();
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
  if (itsWriteToMAC) {
    itsPropertySet = new GCF::CEPPMLlight::CEPPropertySet("CEP_TFCD", "TTeraFlopCorrelator", GCF::Common::PS_CAT_PERMANENT);
    itsPropertySet->enable();
    LOG_TRACE_FLOW("WH_Storage PropertySet enabled");
  } else {
    LOG_TRACE_FLOW("WH_Storage PropertySet not enabled");
  };

  // create MSWriter object
  string msName = itsPS.getString("Storage.MSName");
  double startTime = itsPS.getDouble("Storage.startTime");
  double timeStep = itsPS.getDouble("Storage.timeStep");
  uint nAntennas = itsPS.getUint32("Storage.nStations");
  vector<double> antPos = itsPS.getDoubleVector("Storage.stationPositions");
  itsWriter = new MSWriter(msName.c_str(), startTime, timeStep, nAntennas, antPos);

  int nPolarisations = itsPS.getInt32("Input.NPolarisations");
  int nChannels = itsPS.getInt32("Storage.nChannels");
  double chanWidth = itsPS.getDouble("Storage.chanWidth");
  vector<double> refFreqs= itsPS.getDoubleVector("Storage.refFreqs");
  vector<double>::iterator iter;
  // Add the subbands
  for (iter=refFreqs.begin(); iter!=refFreqs.end(); iter++)
  {
    int bandId = itsWriter->addBand (nPolarisations*nPolarisations,  nChannels,
				     *iter, chanWidth);
    itsBandIds.push_back(bandId);
  }
  double azimuth = itsPS.getDouble("Storage.beamAzimuth");
  double elevation = itsPS.getDouble("Storage.beamElevation");
  double pi = itsPS.getDouble("Storage.pi");
  // For nr of beams
  itsFieldId = itsWriter->addField (azimuth*pi/180., elevation*pi/180.);

  // fill the strings that identify the subbands
  vector<string> refFreqStr = itsPS.getStringVector("Storage.refFreqs");
  vector<string>::iterator sb;
  for (sb=refFreqStr.begin(); it!=refFreqStr.end(); it++)
  {
    itsSubbandStrings.push_back(GCF::Common::GCFPVString(*sb));
  }
}

void WH_Storage::process() 
{
  // loop over all inputs
  DH_Vis* inputDH = 0;
  for (int i=0; i<itsNinputs; i++)
  {
    inputDH = (DH_Vis*)getDataManager().getInHolder(i);
    int rownr = -1;
    // Write 1 frequency
    itsWriter->write (rownr, itsBandIds[i], itsFieldId, 0, 
		      itsCounter, inputDH->getBufSize(),
		      inputDH->getBuffer());   // To do: add flags
  }
  itsCounter++;

  if (itsWriteToMAC) {
    DBGASSERTSTR(itsPropertySet != 0, "no propertySet constructed yet");
    LOG_TRACE_FLOW("WH_Storage setting properties");
    GCF::Common::GCFPValueArray::iterator it;
    for (it = itsVArray.begin(); it != itsVArray.end(); it++){
      delete *it;
    }
    itsVArray.clear();
    
    // loop over values
    for (int i=0; i<itsNinputs; i++) {
      inputDH = (DH_Vis*)getDataManager().getInHolder(i);
      int rownr = -1;
      // Write 1 frequency
      for (int s1 = 0; s1 < itsNstations; s1++) {
	for (int s2 = 0; s2 <= s1; s2++) {
	  for (int p = 0; p < itsNpolSquared; p++) {
	    itsVArray.push_back(new GCF::Common::GCFPVDouble((double)*inputDH->getBufferElement(s1, s2, p)));
	  }
	}
      }
    }

    (*itsPropertySet)["data"].setValue(GCF::Common::GCFPVDynArr(GCF::Common::LPT_DOUBLE, itsVArray));
    (*itsPropertySet)["subband"].setValue(GCF::Common::GCFPVString("1"));
    LOG_TRACE_FLOW("WH_Storage properties set");
  };
}
