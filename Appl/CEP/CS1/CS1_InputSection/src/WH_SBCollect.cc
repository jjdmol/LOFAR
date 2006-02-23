//#  WH_SBCollect.cc: Joins all data (stations, pols) for a subband
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

//# Includes
#include <Common/LofarLogger.h>
#include <CS1_InputSection/WH_SBCollect.h>
#include <CS1_Interface/DH_RSP.h>
#include <CS1_Interface/DH_Subband.h>
#include <Common/hexdump.h>
#include <tinyCEP/Sel_RoundRobin.h>

namespace LOFAR {
  namespace CS1_InputSection {

    WH_SBCollect::WH_SBCollect(const string& name, int sbID, 
			       const ACC::APS::ParameterSet pset,
			       const int noutputs) 
      : WorkHolder   (pset.getInt32("Data.NStations"), 
		      noutputs,
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
      for(int i=0;i<itsNoutputs; i++)
	{
	  sprintf(str, "DH_out_%d", i);
	  getDataManager().addOutDataHolder(i, new DH_Subband(str, itsPS));
	}
      // Set a round robin output selector
      getDataManager().setOutputSelector(new Sel_RoundRobin(itsNoutputs));
    }

    WH_SBCollect::~WH_SBCollect() {
    }

    WorkHolder* WH_SBCollect::construct(const string& name, int sbID, 
					const ACC::APS::ParameterSet pset,
					const int noutputs) 
    {
      return new WH_SBCollect(name, sbID, pset, noutputs);
    }

    WH_SBCollect* WH_SBCollect::make(const string& name)
    {
      return new WH_SBCollect(name, itsSubBandID, itsPS, itsNoutputs);
    }

    void WH_SBCollect::process() 
    { 
      RectMatrix<DH_RSP::BufferType>* inMatrix = &((DH_RSP*)getDataManager().getInHolder(0))->getDataMatrix();
      RectMatrix<DH_Subband::SampleType>& outMatrix = ((DH_Subband*)getDataManager().selectOutHolder())->getDataMatrix();
      dimType outStationDim = outMatrix.getDim("Station");
      dimType inStationDim = inMatrix->getDim("Stations");

      RectMatrix<DH_Subband::SampleType>::cursorType outCursor;
      RectMatrix<DH_RSP::BufferType>::cursorType inCursor;

      outCursor = outMatrix.getCursor( 0 * outStationDim);

      // Loop over all inputs (stations)
      for (int nr=0; nr<itsNinputs; nr++)
	{
	  inMatrix = &((DH_RSP*)getDataManager().getInHolder(nr))->getDataMatrix();
	  inCursor = inMatrix->getCursor(0*inStationDim);
	  // copy all freq, time and pol from an input to output
	  inMatrix->cpy2Matrix(inCursor, inStationDim, outMatrix, outCursor, outStationDim, 1);
	  outMatrix.moveCursor(&outCursor, outStationDim);
	}

#if 0
      // dump the contents of outDH to stdout
      cout << "WH_SBCollect output : " << endl;
      dimType outTimeDim = outMatrix.getDim("Time");
      dimType outPolDim = outMatrix.getDim("Polarisation");
      int matrixSize = itsNinputs * 
	outMatrix.getNElemInDim(outTimeDim) * 
	outMatrix.getNElemInDim(outPolDim);    

      hexdump(outMatrix.getBlock(outMatrix.getCursor(0), 
				 outStationDim, 
				 itsNinputs,
				 matrixSize),
	      matrixSize * sizeof(DH_Subband::SampleType));
      cout << "WH_SBCollect output done " << endl;
#endif
    }
  } // namespace CS1_InputSection
} // namespace LOFAR
