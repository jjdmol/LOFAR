//  WH_Split.cc: A WorkHolder that splits a matrix
//
//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//
//////////////////////////////////////////////////////////////////////

#include <AccepTest2/WH_Split.h>
#include <Common/LofarLogger.h>
#include <AccepTest2/DH_CMatrix.h>

namespace LOFAR
{

  WH_Split::WH_Split (const string& name, 
			      int noutputs, 
			      int matrixXsize, 
			      int matrixYsize)
    : WorkHolder (1, noutputs, name, "WH_Split"),
      itsMatrixXSize(matrixXsize),
      itsMatrixYSize(matrixYsize),
      itsNOutputs(noutputs)
  {
    getDataManager().addInDataHolder(0, new DH_CMatrix("split_input", 
						       matrixXsize * noutputs, 
						       matrixYsize, 
						       "frequency", 
						       "fft(time)"));
    char buffer[64];
    for (int i=0; i<noutputs; i++) {
      sprintf(buffer, "split_output_%d", i);
      getDataManager().addOutDataHolder(i, new DH_CMatrix(buffer, 
							  matrixXsize, 
							  matrixYsize, 
							  "frequency", 
							  "fft(time)"));
    }
  }

  WH_Split::~WH_Split()
  {}

  WorkHolder* WH_Split::construct (const string& name, 
				    int noutputs, 
				    int matrixXsize, 
				    int matrixYsize)
  {
    return new WH_Split (name, noutputs, matrixXsize, matrixYsize);
  }

  WH_Split* WH_Split::make (const string& name)
  {
    return new WH_Split (name, itsNOutputs, itsMatrixXSize, itsMatrixYSize);
  }

  void WH_Split::process()
  {
    // We could do an assert here to see if the DataHolders sizes match

    // this is probably not necessary because al these functions are inline
    // but it delivers cleaner code
    DH_CMatrix* dh_in_matrix = (DH_CMatrix*)getDataManager().getInHolder(0);
    DH_CMatrix* dh_out_matrix;

    float freqstep = ( dh_in_matrix->getXaxis().getEnd() 
		       - dh_in_matrix->getXaxis().getBegin() ) 
                       / itsNOutputs;

    // outi iterates over the outputs
    // ixi  iterates over the x axis of the inputs  ixi = outi * xsize + oxi
    // yi   iterates over the y axis of the inputs and y axis of the output;
    // oxi  iterates over the x axis of the output;
    int ixi=0;

    for (int outi=0; outi<itsNOutputs; outi++) {
      // this is probably not necessary because al these functions are inline
      // but it delivers cleaner code
      dh_out_matrix = (DH_CMatrix*)getDataManager().getOutHolder(outi);

      // fill matrix
      for (int oxi=0; oxi<itsMatrixXSize; oxi++) {
	for (int yi=0; yi<itsMatrixYSize; yi++) {
	  dh_out_matrix->value(oxi, yi) = dh_in_matrix->value(ixi, yi);	  
	}
	ixi++;
      }
      // set axis properties
      dh_out_matrix->getXaxis().setBegin(dh_in_matrix->getXaxis().getBegin() + freqstep * outi);
      dh_out_matrix->getXaxis().setEnd  (dh_in_matrix->getXaxis().getEnd() + freqstep * (outi + 1));
      dh_out_matrix->getYaxis().setBegin(dh_in_matrix->getYaxis().getBegin());
      dh_out_matrix->getYaxis().setEnd  (dh_in_matrix->getYaxis().getEnd());
    }
  }

  void WH_Split::dump()
  {
  }
}
