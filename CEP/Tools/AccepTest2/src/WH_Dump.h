//# WH_Dump.h: Dump to output stream
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#
//# $Id$

#ifndef WH_DUMP_H
#define WH_DUMP_H

#include <lofar_config.h>

#include <tinyCEP/WorkHolder.h>
#include <iostream>

namespace LOFAR
{
  using std::ostream;

  class WH_Dump: public WorkHolder {
  public:
    /// Construct the work holder with its data holders.
    // the ostream should be ready be written to
    explicit WH_Dump (const string& name = "WH_Dump", 
		      int matrixXsize = 0, 
		      int matrixYsize = 0,
		      ostream& os = cout);

    virtual ~WH_Dump();

    /// Static function to create an object.
    static WorkHolder* construct (const string& name = "WH_Dump", 
				  int matrixXsize = 0, 
				  int matrixYsize = 0,
				  ostream& os = cout);

    /// Make a fresh copy of the WH object.
    virtual WH_Dump* make (const string& name);

    /// Do a process step.
    void process();

    /// Show the work holder on stdout.
    void dump();

  private:
    /// Forbid copy constructor.
    WH_Dump (const WH_Dump&);

    /// Forbid assignment.
    WH_Dump& operator= (const WH_Dump&);

    int itsMatrixXSize;
    int itsMatrixYSize;
    ostream& itsOs;
  };
}

#endif
