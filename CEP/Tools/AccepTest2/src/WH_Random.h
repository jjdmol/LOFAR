//# WH_Random.h: An template WorkHolder for the Acceptance test
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

#ifndef WH_RANDOM_H
#define WH_RANDOM_H

#include <lofar_config.h>

#include <tinyCEP/WorkHolder.h>

namespace LOFAR
{
  class WH_Random: public WorkHolder {
  public:
    /// Construct the work holder with its data holders.
    // the matrix sizes are the sizes per DataHolder
    // the total number of values is noutputs*xsize*ysize
    explicit WH_Random (const string& name = "WH_Random", 
			int noutputs = 0, 
			int matrixXsize = 0, 
			int matrixYsize = 0);

    virtual ~WH_Random();

    /// Static function to create an object.
    static WorkHolder* construct (const string& name = "WH_Random", 
				  int noutputs = 0, 
				  int matrixXsize = 0, 
				  int matrixYsize = 0);

    /// Make a fresh copy of the WH object.
    virtual WH_Random* make (const string& name);

    /// Do a process step.
    void process();

    /// Show the work holder on stdout.
    void dump();

  private:
    /// Forbid copy constructor.
    WH_Random (const WH_Random&);

    /// Forbid assignment.
    WH_Random& operator= (const WH_Random&);

    int itsMatrixXSize;
    int itsMatrixYSize;
    int itsNOutputs;
    float itsLastTime;

    // incremental fills the matrices with and incrementing number
    // this way it is easy to keep track of the data
#define USE_INCREMENTAL
#ifdef USE_INCREMENTAL
    static int theirBaseNumber;
    int number;
#endif
  };
}

#endif
