//  WH_Heat.cc: A WorkHolder that performs ffts on the columns of a matrix
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

#include <AccepTest2/WH_Heat.h>
#include <Common/LofarLogger.h>
#include <AccepTest2/DH_CMatrix.h>
#include <complex>
#include <Common/lofar_complex.h>
#include <fftw3.h>

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

  void WH_Heat::process()
  {
    // use fftwf_ and link with -lfftw3f if valueType is complex<float>
    // use fftw_  and link with -lfftw3  if valueType is complex<double>
    
    fftw_complex* in;
    fftw_complex* out;
    fftw_plan p;

    DH_CMatrix* dh_in_matrix = (DH_CMatrix*)getDataManager().getInHolder(0);
    DH_CMatrix* dh_out_matrix = (DH_CMatrix*)getDataManager().getOutHolder(0);

    for (int xi=0; xi<itsMatrixXSize; xi++) {
      in =  reinterpret_cast<fftw_complex*>(& dh_in_matrix->value(xi, 0));
      out = reinterpret_cast<fftw_complex*>(& dh_out_matrix->value(xi, 0));
      // MEASURE takes a lot of time the first time it is called, but the next time it should be faster
      p = fftw_plan_dft_1d(itsMatrixYSize, in, out, itsFFTDirection, FFTW_MEASURE);
      // we should do this once with FFTW_MEASURE and then use the same plan
      //p = fftw_plan_dft_1d(itsMatrixYSize, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
      fftw_execute(p);
      fftw_destroy_plan(p);
    }
    dh_out_matrix->getXaxis().setBegin(dh_in_matrix->getXaxis().getBegin());
    dh_out_matrix->getXaxis().setEnd  (dh_in_matrix->getXaxis().getEnd());
    dh_out_matrix->getYaxis().setBegin(0);
    dh_out_matrix->getYaxis().setEnd  (itsMatrixYSize / 
				       (dh_in_matrix->getYaxis().getEnd() -
					dh_in_matrix->getYaxis().getBegin()));
  }

  void WH_Heat::dump()
  {
  }
}
