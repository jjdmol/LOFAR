//# WH_Split.h: A WorkHolder that splits a matrix
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#
//# $Id$

#ifndef WH_SPLIT_H
#define WH_SPLIT_H

#include <lofar_config.h>

#include <tinyCEP/WorkHolder.h>

namespace LOFAR
{
  /** This class splits its input to several outputs
  */

  class WH_Split: public WorkHolder {
  public:
    /// Construct the work holder with its data holders.
    // the matrix sizes are the sizes per output DataHolder
    // the input is 1 dataholder with ninputs*xsize*ysize values
    explicit WH_Split (const string& name = "WH_Split", 
			int noutputs = 0, 
			int matrixXsize = 0, 
			int matrixYsize = 0);

    virtual ~WH_Split();

    /// Static function to create an object.
    static WorkHolder* construct (const string& name = "WH_Split", 
				  int noutputs = 0, 
				  int matrixXsize = 0, 
				  int matrixYsize = 0);

    /// Make a fresh copy of the WH object.
    virtual WH_Split* make (const string& name);

    /// Do a process step.
    void process();

    /// Show the work holder on stdout.
    void dump();

  private:
    /// Forbid copy constructor.
    WH_Split (const WH_Split&);

    /// Forbid assignment.
    WH_Split& operator= (const WH_Split&);

    int itsMatrixXSize;
    int itsMatrixYSize;
    int itsNOutputs;
  };
}

#endif
