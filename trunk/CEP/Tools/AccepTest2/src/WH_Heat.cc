//  WH_Heat.cc: A WorkHolder that performs ffts on the columns of a matrix
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

#include <AccepTest2/WH_Heat.h>
#include <Common/LofarLogger.h>
#include <AccepTest2/DH_CMatrix.h>

namespace LOFAR
{

  WH_Heat::WH_Heat (const string& name, 
		    int matrixXsize, 
		    int matrixYsize,
		    bool forwardFFT)
    : WorkHolder (1, 1, name, "WH_Heat"),
      itsMatrixXSize(matrixXsize),
      itsMatrixYSize(matrixYsize)
  {
    getDataManager().addInDataHolder(0, new DH_CMatrix("heat_input", 
						       matrixXsize, 
						       matrixYsize, 
						       "frequency", 
						       "fft(time)"));
    getDataManager().addOutDataHolder(0, new DH_CMatrix("heat_output", 
							matrixXsize, 
							matrixYsize, 
							"frequency", 
							"time"));
    if (forwardFFT) {
      itsFFTDirection = FFTW_FORWARD;
    } else {
      itsFFTDirection = FFTW_BACKWARD;
    };
  }

  WH_Heat::~WH_Heat()
  {}

  WorkHolder* WH_Heat::construct (const string& name, 
				  int matrixXsize, 
				  int matrixYsize,
				  bool forwardFFT)
  {
    return new WH_Heat (name, matrixXsize, matrixYsize, forwardFFT);
  }

  WH_Heat* WH_Heat::make (const string& name)
  {
    return new WH_Heat (name, itsMatrixXSize, itsMatrixYSize, (itsFFTDirection == FFTW_FORWARD));
  }

  void WH_Heat::preprocess()
  {
    itsFFTPlan = fftw_create_plan(itsMatrixYSize, itsFFTDirection, FFTW_MEASURE);
  }

  void WH_Heat::process()
  {
    fftw_complex* in;
    fftw_complex* out;

    DH_CMatrix* dh_in_matrix = (DH_CMatrix*)getDataManager().getInHolder(0);
    DH_CMatrix* dh_out_matrix = (DH_CMatrix*)getDataManager().getOutHolder(0);

    for (int xi=0; xi<itsMatrixXSize; xi++) {
      in =  reinterpret_cast<fftw_complex*>(& dh_in_matrix->value(xi, 0));
      out = reinterpret_cast<fftw_complex*>(& dh_out_matrix->value(xi, 0));
      fftw_one(itsFFTPlan, in, out);
    }
    dh_out_matrix->getXaxis().setBegin(dh_in_matrix->getXaxis().getBegin());
    dh_out_matrix->getXaxis().setEnd  (dh_in_matrix->getXaxis().getEnd());
    dh_out_matrix->getYaxis().setBegin(0);
    dh_out_matrix->getYaxis().setEnd  (itsMatrixYSize / 
				       (dh_in_matrix->getYaxis().getEnd() -
					dh_in_matrix->getYaxis().getBegin()));
  }

  void WH_Heat::postprocess()
  {
    fftw_destroy_plan(itsFFTPlan);
  }

  void WH_Heat::dump()
  {
  }
}
