//  WH_Transpose.cc: A WorkHolder that transposes a matrix
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

#include <AccepTest2/WH_Transpose.h>
#include <Common/LofarLogger.h>
#include <AccepTest2/DH_CMatrix.h>

namespace LOFAR
{

  WH_Transpose::WH_Transpose (const string& name, 
			      int ninputs, 
			      int matrixXsize, 
			      int matrixYsize)
    : WorkHolder (ninputs, 1, name, "WH_Transpose"),
      itsMatrixXSize(matrixXsize),
      itsMatrixYSize(matrixYsize),
      itsNInputs(ninputs)
  {
    char buffer[64];
    for (int i=0; i<ninputs; i++) {
      sprintf(buffer, "transpose_input_%d", i);
      getDataManager().addInDataHolder(i, new DH_CMatrix(buffer, 
							 matrixXsize, 
							 matrixYsize, 
							 "time", 
							 "frequency"));
    }
    getDataManager().addOutDataHolder(0, new DH_CMatrix("transpose_output", 
							matrixYsize * ninputs, 
							matrixXsize, 
							"frequency", 
							"time"));
  }

  WH_Transpose::~WH_Transpose()
  {}

  WorkHolder* WH_Transpose::construct (const string& name, 
				    int ninputs, 
				    int matrixXsize, 
				    int matrixYsize)
  {
    return new WH_Transpose (name, ninputs, matrixXsize, matrixYsize);
  }

  WH_Transpose* WH_Transpose::make (const string& name)
  {
    return new WH_Transpose (name, itsNInputs, itsMatrixXSize, itsMatrixYSize);
  }

  void WH_Transpose::process()
  {
    // this is the reference implementation
    // Tranpose could also be done by just reading in a different way in the next step

    // We could do an assert here to see if the destination DataHolder is large enough

    // this is probably not necessary because al these functions are inline
    // but it delivers cleaner code
    DH_CMatrix* dh_out_matrix = (DH_CMatrix*)getDataManager().getOutHolder(0);
    DH_CMatrix* dh_in_matrix;


    // outi iterates over the inputs
    // iyi  iterates over the y axis of the inputs;
    // xi   iterates over the x axis of the inputs and y of the output;
    // oyi  iterates over the x axis of the output  oyi = outi * ysize + iyi
    int oyi=0;

    for (int outi=0; outi<getDataManager().getInputs(); outi++) {
      // this is probably not necessary because al these functions are inline
      // but it delivers cleaner code
      dh_in_matrix = (DH_CMatrix*)getDataManager().getInHolder(outi);

      // fill matrix
      for (int iyi=0; iyi<itsMatrixYSize; iyi++) {
	for (int xi=0; xi<itsMatrixXSize; xi++) {
	  dh_out_matrix->value(oyi, xi) = dh_in_matrix->value(xi, iyi);	  
	}
	oyi++;
      }
    }
    // set axis properties
    dh_out_matrix->getXaxis().setBegin(((DH_CMatrix*)getDataManager().getInHolder(0))->getYaxis().getBegin());
    dh_out_matrix->getXaxis().setEnd  (dh_in_matrix->getYaxis().getEnd());
    dh_out_matrix->getYaxis().setBegin(dh_in_matrix->getXaxis().getBegin());
    dh_out_matrix->getYaxis().setEnd  (dh_in_matrix->getXaxis().getEnd());
  }

  void WH_Transpose::dump()
  {
  }
}
