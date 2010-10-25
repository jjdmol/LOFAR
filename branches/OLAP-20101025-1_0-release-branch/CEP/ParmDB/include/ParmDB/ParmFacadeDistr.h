//# ParmFacadeDistr.h: Data access a distributed parameter database
//#
//# Copyright (C) 2009
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
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
#include <Common/lofar_set.h>

//# Forward Declaration.
namespace casa {
  class Record;
  class String;
}

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
    // Define the possible commands.
    enum Command {Quit,
                  GetRange,
                  GetValues,
                  GetValuesVec,
                  GetValuesGrid,
                  GetCoeff};

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
    virtual vector<double> getRange (const string& parmNamePattern) const;

    // Get parameter names in the table matching the pattern.
    // An empty name pattern is the same as * (all parm names).
    virtual vector<string> getNames (const string& parmNamePattern) const;

    // Get default parameter names matching the pattern.
    // An empty name pattern is the same as * (all parm names).
    virtual vector<string> getDefNames (const string& parmNamePattern) const;

    // Get the default values of parameters matching the pattern.
    virtual casa::Record getDefValues (const string& parmNamePattern) const;

    // Get the values of the given parameters on the given regular grid
    // where v1/v2 represents center/width or start/end.
    // The Record contains a map of parameter name to Array<double>.
    virtual casa::Record getValues (const string& parmNamePattern,
                                    double freqv1, double freqv2,
                                    double freqStep,
                                    double timev1, double timev2,
                                    double timeStep,
                                    bool asStartEnd);

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
                                        double freqv1, double freqv2,
                                        double timev1, double timev2,
                                        bool asStartEnd=false);

    // Get coefficients, errors, and domains they belong to.
    virtual casa::Record getCoeff (const string& parmNamePattern,
                                   double freqv1, double freqv2,
                                   double timev1, double timev2,
                                   bool asStartEnd);

  private:
    // Send all workers a quit message.
    void quit();

    // Get and free a port.
    // <group>
    string getPort();
    void freePort();
    // </group>

    // Read a Record from the BlobStream.
    void getRecord (BlobIStream& bis, casa::Record& rec);

    // Check if the names of remote client inx are equal to the first one.
    void checkNames (const vector<string>& firstNames,
                     const vector<string>& names, uint inx) const;

    // Combine the result records from the remote sites.
    casa::Record combineRemote (const vector<casa::Record>& recs) const;

    // Find all parm names in the records and add them to the set.
    void findParmNames (const vector<casa::Record>& recs,
                        set<casa::String>& names) const;

    // Combine the info for the given parm from all records.
    // The info can be the same in some records meaning that a fully or partial
    // global solve is done and distributed to all parmdbs.
    void combineInfo (const casa::String& name,
                      const vector<casa::Record>& recs,
                      casa::Record& result) const;

    //# Data members
    string                itsPort;      //# declare this before itsConn!!
    mutable LOFAR::CEP::SocketConnectionSet itsConn;
    vector<string>        itsPartNames;
    vector<string>        itsParmNames;
    casa::Record          itsDefValues;
    static int            theirNextPort;
    static vector<string> theirFreePorts;
  };

  // @}

}} // namespaces

#endif
