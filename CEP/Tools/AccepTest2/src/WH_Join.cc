//  WH_Join.cc: A WorkHolder that joins matrices
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

#include <AccepTest2/WH_Join.h>
#include <Common/LofarLogger.h>
#include <AccepTest2/DH_CMatrix.h>

namespace LOFAR
{

  WH_Join::WH_Join (const string& name, 
			      int ninputs, 
			      int matrixXsize, 
			      int matrixYsize)
    : WorkHolder (ninputs, 1, name, "WH_Join"),
      itsMatrixXSize(matrixXsize),
      itsMatrixYSize(matrixYsize),
      itsNInputs(ninputs)
  {
    char buffer[64];
    for (int i=0; i<ninputs; i++) {
      sprintf(buffer, "join_input_%d", i);
      getDataManager().addInDataHolder(i, new DH_CMatrix(buffer, 
							 matrixXsize, 
							 matrixYsize, 
							 "frequency", 
							 "time"));
    }
    getDataManager().addOutDataHolder(0, new DH_CMatrix("join_output", 
							matrixXsize * ninputs, 
							matrixYsize, 
							"frequency", 
							"time"));
  }

  WH_Join::~WH_Join()
  {}

  WorkHolder* WH_Join::construct (const string& name, 
				    int ninputs, 
				    int matrixXsize, 
				    int matrixYsize)
  {
    return new WH_Join (name, ninputs, matrixXsize, matrixYsize);
  }

  WH_Join* WH_Join::make (const string& name)
  {
    return new WH_Join (name, itsNInputs, itsMatrixXSize, itsMatrixYSize);
  }

  void WH_Join::process()
  {
    // We could do an assert here to see if the DataHolders sizes match

    // this is probably not necessary because al these functions are inline
    // but it delivers cleaner code
    DH_CMatrix* dh_in_matrix;
    DH_CMatrix* dh_out_matrix = (DH_CMatrix*)getDataManager().getOutHolder(0);

    // ini iterates over the inputs
    // ixi  iterates over the x axis of the inputs
    // yi   iterates over the y axis of the inputs and y axis of the output;
    // oxi  iterates over the x axis of the output  oxi = ini * xsize + oxi
    int oxi=0;

    for (int ini=0; ini<itsNInputs; ini++) {
      dh_in_matrix = (DH_CMatrix*)getDataManager().getInHolder(ini);

      // fill matrix
      for (int ixi=0; ixi<itsMatrixXSize; ixi++) {
	for (int yi=0; yi<itsMatrixYSize; yi++) {
	  dh_out_matrix->value(oxi, yi) = dh_in_matrix->value(ixi, yi);	  
	}
	oxi++;
      }
    }
    // set axis properties
    dh_out_matrix->getXaxis().setBegin(((DH_CMatrix*)getDataManager().getInHolder(0))->getXaxis().getBegin());
    dh_out_matrix->getXaxis().setEnd  (dh_in_matrix->getXaxis().getEnd());
    dh_out_matrix->getYaxis().setBegin(dh_in_matrix->getYaxis().getBegin());
    dh_out_matrix->getYaxis().setEnd  (dh_in_matrix->getYaxis().getEnd());
  }

  void WH_Join::dump()
  {
  }
}
