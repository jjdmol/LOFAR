//# ParmFacadeDistr.h: Data access a distributed parameter database
//#
//# Copyright (C) 2009
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

#ifndef LOFAR_PARMDB_PARMFACADEDISTR_H
#define LOFAR_PARMDB_PARMFACADEDISTR_H

// \file
// Data access the a distributed parameter database.

//# Includes
#include <ParmDB/ParmFacadeRep.h>
#include <MWCommon/SocketConnectionSet.h>
#include <Common/lofar_vector.h>

namespace LOFAR { namespace BBS {


  // \ingroup ParmDB
  // @{

  // ParmFacadeDistr is the high level interface to a distributed Parameter
  // Data Base.
  // It starts the remote processes and connects to them. At the end it
  // sends them a quit message.
  //
  // The class provides a few functions:
  // <ul>
  // <li> getNames returns a vector of the parameter names in the table
  // <li> getRange returns a vector of 4 elements giving the boundary box
  //    of the domains of the parameters.
  // <li> getValues returns the values of the parameters a calculated on
  //      a grid given by the caller.
  // </ul>
  //
  // The parameter names can be given as a pattern. This is the same as a
  // file name pattern that can be given in the UNIX shells (e.g. RA:*).
  // Thus it is not a full regular expression.

  class ParmFacadeDistr : public ParmFacadeRep
  {
  public:
    // Make a connection to the given distributed ParmTable.
    // It starts the remote processes which connect to this object.
    ParmFacadeDistr (const string& tableName);

    // The destructor disconnects and sends the remote processes an
    // end message.
    virtual ~ParmFacadeDistr();

    // Get the domain range (as startx,endx,starty,endy) of the given
    // parameters in the table.
    // This is the minimum start value and maximum end value for all parameters.
    // An empty name pattern is the same as * (all parm names).
    virtual vector<double> getRange (const string& parmNamePattern = "") const;

    // Get parameter names in the table matching the pattern.
    // An empty name pattern is the same as * (all parm names).
    virtual vector<string> getNames (const string& parmNamePattern = "") const;

    // Get the values of the given parameters on the given regular grid
    // where v1/v2 represents center/width or start/end.
    // The Record contains a map of parameter name to Array<double>.
    virtual casa::Record getValues (const string& parmNamePattern,
                                    double freqv1, double freqv2, int nfreq,
                                    double timev1, double timev2, int ntime,
                                    bool asStartEnd=false);

    // Get the values of the given parameters on the given grid where v1/v2
    // represents center/width or start/end.
    // The Record contains a map of parameter name to Array<double>.
    virtual casa::Record getValues (const string& parmNamePattern,
                                    const vector<double>& freqv1,
                                    const vector<double>& freqv2,
                                    const vector<double>& timev1,
                                    const vector<double>& timev2,
                                    bool asStartEnd=false);

    // Get the values of the given parameters for the given domain.
    // The Record contains a map of parameter name to Array<value>.
    // Furthermore it contains a subrecord "_grid" containing the grid axes
    // used for each parameters. Their names have the form <parmname>/xx
    // where xx is freqs, freqwidths, times, and timewidths. Their values
    // are the center and width of each cell.
    virtual casa::Record getValuesGrid (const string& parmNamePattern,
                                        double sfreq=-1e30, double efreq=1e30,
                                        double stime=-1e30, double etime=1e30);

  private:
    // Send all workers a quit message.
    void quit();

    // Get and free a port.
    // <group>
    string getPort();
    void freePort();
    // </group>

    //# Data membe rs
    string                itsPort;      //# declare this before itsConn!!
    mutable LOFAR::CEP::SocketConnectionSet itsConn;
    static int            theirNextPort;
    static vector<string> theirFreePorts;
  };

  // @}

}} // namespaces

#endif
