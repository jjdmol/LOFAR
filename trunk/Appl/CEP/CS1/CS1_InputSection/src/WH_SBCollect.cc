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

    WH_SBCollect::WH_SBCollect(const string& name,
			       const ACC::APS::ParameterSet pset,
			       const int noutputs) 
      : WorkHolder          (pset.getInt32("Observation.NStations"), 
			     noutputs,
			     name,
			     "WH_SBCollect"),
	itsPS               (pset),
	itsNStations        (pset.getInt32("Observation.NStations")),
	itsNSubbandsPerCell (pset.getInt32("General.SubbandsPerCell"))
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

    WorkHolder* WH_SBCollect::construct(const string& name,
					const ACC::APS::ParameterSet pset,
					const int noutputs) 
    {
      return new WH_SBCollect(name, pset, noutputs);
    }

    WH_SBCollect* WH_SBCollect::make(const string& name)
    {
      return new WH_SBCollect(name, itsPS, itsNoutputs);
    }

    void WH_SBCollect::process() 
    { 
      vector<DH_RSP*> inHolders;
      vector<RectMatrix<DH_RSP::BufferType> *> inMatrices;
      vector<RectMatrix<DH_RSP::BufferType>::cursorType> inCursors;
      RectMatrix<DH_Subband::SampleType>::cursorType outCursor;

      // create cursors to incoming data
      for (unsigned station = 0; station < itsNStations; station++) {
	inHolders.push_back(dynamic_cast<DH_RSP *>(getDataManager().getInHolder(station)));
	dimType inSubbandDim = inMatrices[0]->getDim("Subbands");
	inCursors.push_back(inHolders.back()->getDataMatrix().getCursor(0*inSubbandDim));
	inMatrices.push_back(&(inHolders.back()->getDataMatrix()));
      }

      // Copy every subband to one BG/L core
      for (unsigned subband = 0; subband < itsNSubbandsPerCell; subband++) {
	// ask the round robin selector for the next output
	DH_Subband *outHolder = (DH_Subband*) getDataManager().selectOutHolder();
	RectMatrix<DH_Subband::SampleType>* outMatrix = &outHolder->getSamplesMatrix();
	dimType inSubbandDim = inMatrices[0]->getDim("Subbands");
	dimType outStationDim = outMatrix->getDim("Station");
	outCursor = outMatrix->getCursor( 0 * outStationDim);

	// Copy one subbands from every input
	for (unsigned station = 0; station < itsNStations; station++) {
	  inMatrices[station]->cpy2Matrix(inCursors[station], inSubbandDim, *outMatrix, outCursor, outStationDim, 1);
	  inMatrices[station]->moveCursor(&inCursors[station], inSubbandDim);
	  outMatrix->moveCursor(&outCursor, outStationDim);

	  // copy other information (delayInfo, flags etc)
	  DH_Subband::DelayIntervalType theDelay = {inHolders[station]->getFineDelayAtBegin(), 
						    inHolders[station]->getFineDelayAfterEnd()};
	  outHolder->getDelay(station) = theDelay;
#if defined SPARSE_FLAGS
	  inHolders[station]->getExtraData();
	  outHolder->getFlags(station) = inHolders[station]->getFlags();
#endif
	}
#if defined SPARSE_FLAGS
	outHolder->fillExtraData();
#endif
      }

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
