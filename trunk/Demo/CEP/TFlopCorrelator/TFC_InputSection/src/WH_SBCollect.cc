//#  WH_SBCollect.cc: Joins all data (stations, pols) for a subband
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
#include <TFC_InputSection/WH_SBCollect.h>
#include <TFC_Interface/DH_RSP.h>
#include <TFC_Interface/DH_FIR.h>

#include <Common/hexdump.h>

using namespace LOFAR;

WH_SBCollect::WH_SBCollect(const string& name, int sbID, 
			   const ACC::APS::ParameterSet pset) 
  : WorkHolder   (pset.getInt32("NRSP"), 
		  1,
		  name,
		  "WH_SBCollect"),
    itsPS        (pset),
    itsSubBandID (sbID)
{
  char str[32];
  for (int i=0; i<itsNinputs; i++)
  {
    sprintf(str, "DH_in_%d", i);
    getDataManager().addInDataHolder(i, new DH_RSP(str, itsPS));
  }

  getDataManager().addOutDataHolder(0, new DH_FIR(str, itsSubBandID, itsPS));

}

WH_SBCollect::~WH_SBCollect() {
}

WorkHolder* WH_SBCollect::construct(const string& name, int sbID, 
				    const ACC::APS::ParameterSet pset) 
{
  return new WH_SBCollect(name, sbID, pset);
}

WH_SBCollect* WH_SBCollect::make(const string& name)
{
  return new WH_SBCollect(name, itsSubBandID, itsPS);
}

void WH_SBCollect::process() 
{ 
  // pack all inputs into one output
  DH_RSP* inpDH;
  DH_FIR* outpDH = (DH_FIR*)getDataManager().getOutHolder(0);
  RectMatrix<DH_FIR::BufferType>& output = outpDH->getDataMatrix();
  dimType stationDim = output.getDim("Station");
  RectMatrix<DH_FIR::BufferType>::cursorType outCursor;

  DH_RSP::BufferType* inpPtr;
  uint inpSize;
  // Loop over all inputs (stations)
  for (int nr=0; nr<itsNinputs; nr++)
  {
    inpDH = (DH_RSP*)getDataManager().getInHolder(nr);
    inpPtr = inpDH->getBuffer();
    inpSize = inpDH->getBufferSize();

    outCursor = output.getCursor( nr*stationDim );
    // copy all freq, time and pol from an input to output
    output.cpyFromBlock(inpPtr, inpSize, outCursor, stationDim, 1);
  }

  for (int nr=0; nr<itsNinputs; nr++)
  {
    cout << "WH_SBCollect input " << nr << " : " << endl;
    DH_RSP* inp = (DH_RSP*)getDataManager().getInHolder(nr);
    hexdump(inp->getBuffer(), inp->getBufferSize() * sizeof(DH_RSP::BufferType));
  }

  cout << "WH_SBCollect output : " << endl;
  hexdump(outpDH->getBuffer(), outpDH->getBufferSize() * sizeof(DH_FIR::BufferType));
   
}
