//# WH_Transpose.h: A WorkHolder that transposes a matrix
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#
//# $Id$

#ifndef WH_TRANSPOSE_H
#define WH_TRANSPOSE_H

#include <lofar_config.h>

#include <tinyCEP/WorkHolder.h>

namespace LOFAR
{
  /** This class transposes its inputs to 1 output
      The inputs are added in the y direction. If the
      inputs are nx by ny, the output is ny * ninputs by nx
  */

  class WH_Transpose: public WorkHolder {
  public:
    /// Construct the work holder with its data holders.
    // the matrix sizes are the sizes per input DataHolder
    // the output is 1 dataholder with ninputs*xsize*ysize values
    explicit WH_Transpose (const string& name = "WH_Transpose", 
			   int ninputs = 0, 
			   int matrixXsize = 0, 
			   int matrixYsize = 0);

    virtual ~WH_Transpose();

    /// Static function to create an object.
    static WorkHolder* construct (const string& name = "WH_Transpose", 
				  int ninputs = 0, 
				  int matrixXsize = 0, 
				  int matrixYsize = 0);

    /// Make a fresh copy of the WH object.
    virtual WH_Transpose* make (const string& name);

    /// Do a process step.
    void process();

    /// Show the work holder on stdout.
    void dump();

  private:
    /// Forbid copy constructor.
    WH_Transpose (const WH_Transpose&);

    /// Forbid assignment.
    WH_Transpose& operator= (const WH_Transpose&);

    int itsMatrixXSize;
    int itsMatrixYSize;
    int itsNInputs;
  };
}

#endif
