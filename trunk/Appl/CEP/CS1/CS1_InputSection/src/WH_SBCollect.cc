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

namespace LOFAR
{
  namespace CS1
  {

    WH_SBCollect::WH_SBCollect(const string& name, int sbID, 
			       const ACC::APS::ParameterSet pset,
			       const int noutputs) 
      : WorkHolder   (pset.getInt32("Observation.NStations"), 
		      noutputs,
		      name,
		      "WH_SBCollect"),
	itsPS        (pset),
	itsSubBandID (sbID),
	itsCore(0)
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
	  //getDataManager().setAutoTriggerOut(i, false);
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
#if 0
      // now we send every station seperately using the same connection.
      // this means we send nstations times per process

      RectMatrix<DH_RSP::BufferType>* inMatrix = &((DH_RSP*)getDataManager().getInHolder(0))->getDataMatrix();
      dimType inStationDim = inMatrix->getDim("Stations");

      RectMatrix<DH_Subband::SampleType>& outMatrix = ((DH_Subband*)getDataManager().getOutHolder(itsCore))->getDataMatrix();
      dimType outStationDim = outMatrix.getDim("Station");

      RectMatrix<DH_Subband::SampleType>::cursorType outCursor;
      RectMatrix<DH_RSP::BufferType>::cursorType inCursor;

      // Loop over all inputs (stations)
      for (int nr=0; nr<itsNinputs; nr++) {
	outMatrix = &((DH_Subband*)getDataManager().getOutHolder(core))->getDataMatrix();
	outCursor = outMatrix.getCursor( 0 * outStationDim);

	inMatrix = &((DH_RSP*)getDataManager().getInHolder(nr))->getDataMatrix();
	inCursor = inMatrix->getCursor(0*inStationDim);
	// copy all freq, time and pol from an input to output
	inMatrix->cpy2Matrix(inCursor, inStationDim, outMatrix, outCursor, outStationDim, 1);
	getDataManager().readyWithOutHolder(core);
      }
#else
      RectMatrix<DH_RSP::BufferType>* inMatrix = &((DH_RSP*)getDataManager().getInHolder(0))->getDataMatrix();
      RectMatrix<DH_Subband::SampleType>* outMatrix = &((DH_Subband*)getDataManager().getOutHolder(itsCore))->getDataMatrix();
      dimType outStationDim = outMatrix->getDim("Station");
      dimType inStationDim = inMatrix->getDim("Stations");

      RectMatrix<DH_Subband::SampleType>::cursorType outCursor;
      RectMatrix<DH_RSP::BufferType>::cursorType inCursor;

      outCursor = outMatrix->getCursor( 0 * outStationDim);

      // Loop over all inputs (stations)
      for (int nr=0; nr<itsNinputs; nr++)
	{
	  inMatrix = &((DH_RSP*)getDataManager().getInHolder(nr))->getDataMatrix();
	  inCursor = inMatrix->getCursor(0*inStationDim);
	  // copy all freq, time and pol from an input to output
	  inMatrix->cpy2Matrix(inCursor, inStationDim, *outMatrix, outCursor, outStationDim, 1);
	  outMatrix->moveCursor(&outCursor, outStationDim);

	  DH_Subband::AllSamplesType* ptr = ((DH_Subband*)getDataManager().getOutHolder(itsCore))->getSamples();
	  (*ptr)[0][0][0] = makei16complex(1, 2);
	}

      getDataManager().readyWithOutHolder(itsCore);
#endif
      itsCore ++ ;
      if (itsCore >= itsNoutputs) itsCore = 0;

#if 0
      // dump the contents of outDH to stdout
      cout << "WH_SBCollect output : " << endl;
      dimType outTimeDim = outMatrix->getDim("Time");
      dimType outPolDim = outMatrix->getDim("Polarisation");
      int matrixSize = itsNinputs * 
	outMatrix->getNElemInDim(outTimeDim) * 
	outMatrix->getNElemInDim(outPolDim);    

      hexdump(outMatrix->getBlock(outMatrix->getCursor(0), 
				 outStationDim, 
				 itsNinputs,
				 matrixSize),
	      matrixSize * sizeof(DH_Subband::SampleType));
      cout << "WH_SBCollect output done " << endl;
#endif
    }

    void WH_SBCollect::postprocess() 
    {
      sleep(10);
    }  

  } // namespace CS1

} // namespace LOFAR
