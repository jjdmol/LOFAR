//# WH_Join.h: A WorkHolder that joins a matrix
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#
//# $Id$

#ifndef WH_JOIN_H
#define WH_JOIN_H

#include <lofar_config.h>

#include <tinyCEP/WorkHolder.h>

namespace LOFAR
{
  /** This class joins its inputs to one outputs
  */

  class WH_Join: public WorkHolder {
  public:
    /// Construct the work holder with its data holders.
    // the matrix sizes are the sizes per input DataHolder
    // the input is 1 dataholder with ninputs*xsize*ysize values
    explicit WH_Join (const string& name = "WH_Join", 
		      int ninputs = 0, 
		      int matrixXsize = 0, 
		      int matrixYsize = 0);

    virtual ~WH_Join();

    /// Static function to create an object.
    static WorkHolder* construct (const string& name = "WH_Join", 
				  int ninputs = 0, 
				  int matrixXsize = 0, 
				  int matrixYsize = 0);

    /// Make a fresh copy of the WH object.
    virtual WH_Join* make (const string& name);

    /// Do a process step.
    void process();

    /// Show the work holder on stdout.
    void dump();

  private:
    /// Forbid copy constructor.
    WH_Join (const WH_Join&);

    /// Forbid assignment.
    WH_Join& operator= (const WH_Join&);

    int itsMatrixXSize;
    int itsMatrixYSize;
    int itsNInputs;
  };
}

#endif
