//# WH_Heat.h: A WorkHolder that performs calculations
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef WH_HEAT_H
#define WH_HEAT_H

#include <lofar_config.h>
#include <complex>
#include <Common/lofar_complex.h>
#include <fftw.h>
#include <tinyCEP/WorkHolder.h>

namespace LOFAR
{
  /** This class performs calculations on a matrix.
      It performs a fft on every column of the matrix.
  */

  class WH_Heat: public WorkHolder {
  public:
    /// Construct the work holder with its data holders.
    // the matrix sizes are the sizes per input DataHolder
    // the output is 1 dataholder with ninputs*xsize*ysize values
    explicit WH_Heat (const string& name = "WH_Heat", 
		      int matrixXsize = 0, 
		      int matrixYsize = 0,
		      bool forwardFFT = true);

    virtual ~WH_Heat();

    /// Static function to create an object.
    static WorkHolder* construct (const string& name = "WH_Heat", 
				  int matrixXsize = 0, 
				  int matrixYsize = 0,
				  bool forwardFFT = true);

    /// Make a fresh copy of the WH object.
    virtual WH_Heat* make (const string& name);

    /// Prepare the run
    void preprocess();
    /// Do a process step.
    void process();
    /// Clean up
    void postprocess();

    /// Show the work holder on stdout.
    void dump();

  private:
    /// Forbid copy constructor.
    WH_Heat (const WH_Heat&);

    /// Forbid assignment.
    WH_Heat& operator= (const WH_Heat&);

    int itsMatrixXSize;
    int itsMatrixYSize;
    fftw_direction itsFFTDirection;
    fftw_plan itsFFTPlan;
  };
}

#endif
