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

using namespace LOFAR;

WH_Storage::WH_Storage(const string& name, 
		       const ACC::APS::ParameterSet& pset) 
  : WorkHolder (pset.getInt32("Input.NSubbands"),        // for now number of correlator outputs
		0,                                  // is equal to number of subbands
		name,
		"WH_Storage"),
    itsPS      (pset),
    itsWriter  (0),
    itsCounter (0)
{
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

void WH_Storage::preprocess()
{
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

  // >>> Temporary!
  for (int i=0; i<itsNinputs; i++) {
    ((DH_Vis*)getDataManager().getInHolder(i))->setStorageTestPattern();
  }

}

void WH_Storage::process() 
{
  // Write data in MS 
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
}
