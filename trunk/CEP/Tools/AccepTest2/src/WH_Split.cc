//  WH_Split.cc: A WorkHolder that splits a matrix
//
//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//
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
    // ixi  iterates over the x axis of the input  ixi = outi * xsize + oxi
    // yi   iterates over the y axis of the inputs and y axis of the output;
    // oxi  iterates over the x axis of the outputs;
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
