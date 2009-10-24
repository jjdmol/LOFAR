//# ParmFacadeRep.h: Data access the parameter database
//#
//# Copyright (C) 2006
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

#ifndef LOFAR_PARMDB_PARMFACADEREP_H
#define LOFAR_PARMDB_PARMFACADEREP_H

// \file
// Data access the parameter database.

//# Includes
#include <ParmDB/ParmDB.h>
#include <casa/Containers/Record.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_map.h>
#include <Common/lofar_string.h>
#include <Common/lofar_smartptr.h>


namespace LOFAR { namespace BBS {


  // \ingroup ParmDB
  // @{

  // ParmFacadeRep is the high level interface to the Parameter Data Base.
  // The current version assumes it is an AIPS++ table; with a few extra
  // constructor arguments it can easily be changed to other types of
  // databases.
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

  class ParmFacadeRep
  {
  public:
    // Define a shared_ptr for this class.
    typedef shared_ptr<ParmFacadeRep> ShPtr;

    // The destructor disconnects.
    virtual ~ParmFacadeRep();

    // Get the domain range (as startx,endx,starty,endy) of the given
    // parameters in the table.
    // This is the minimum start value and maximum end value for all parameters.
    // An empty name pattern is the same as * (all parm names).
    virtual vector<double> getRange (const string& parmNamePattern) const = 0;

    // Get parameter names in the table matching the pattern.
    // An empty name pattern is the same as * (all parm names).
    virtual vector<string> getNames (const string& parmNamePattern) const = 0;

    // Get the values of the given parameters on the given regular grid
    // where v1/v2 represents center/width or start/end.
    // The Record contains a map of parameter name to Array<double>.
    virtual casa::Record getValues (const string& parmNamePattern,
                                    double freqv1, double freqv2,
                                    double freqStep,
                                    double timev1, double timev2,
                                    double timeStep,
                                    bool asStartEnd) = 0;

    // Get the values of the given parameters on the given grid where v1/v2
    // represents center/width or start/end.
    // The Record contains a map of parameter name to Array<double>.
    virtual casa::Record getValues (const string& parmNamePattern,
                                    const vector<double>& freqv1,
                                    const vector<double>& freqv2,
                                    const vector<double>& timev1,
                                    const vector<double>& timev2,
                                    bool asStartEnd) = 0;

    // Get the values of the given parameters for the given domain.
    // The Record contains a map of parameter name to Array<value>.
    // Furthermore it contains a subrecord "_grid" containing the grid axes
    // used for each parameters. Their names have the form <parmname>/xx
    // where xx is freqs, freqwidths, times, and timewidths. Their values
    // are the center and width of each cell.
    virtual casa::Record getValuesGrid (const string& parmNamePattern,
                                        double freqv1, double freqv2,
                                        double timev1, double timev2,
                                        bool asStartEnd) = 0;

    // Get coefficients, errors, and domains they belong to.
    virtual casa::Record getCoeff (const string& parmNamePattern,
                                   double freqv1, double freqv2,
                                   double timev1, double timev2,
                                   bool asStartEnd) = 0;
   };

  // @}

}} // namespaces

#endif
