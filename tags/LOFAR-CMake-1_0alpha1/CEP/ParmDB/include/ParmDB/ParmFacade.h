//# ParmFacade.h: Data access the parameter database
//#
//# Copyright (C) 2006
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

#ifndef LOFAR_PARMDB_PARMFACADE_H
#define LOFAR_PARMDB_PARMFACADE_H

// \file
// Data access the parameter database.

//# Includes
#include <ParmDB/ParmFacadeRep.h>
#include <Common/lofar_map.h>


namespace LOFAR { namespace BBS {


  // \ingroup ParmDB
  // @{

  // ParmFacade is the high level interface to the Parameter Data Base.
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

  class ParmFacade
  {
  public:
    // Make a connection to the given ParmTable.
    // The ParmTable can be a single one, but it can also be a distributed
    // ParmTable. The latter should be given by means of the VDS-file as
    // created by the scripts setupparmdb and setupsourcedb.
    ParmFacade (const string& tableName);

    // The destructor closes the parm table.
    ~ParmFacade();

    // Get the version info (tree, top, full or other)
    string version (const string& type) const;

    // Get the domain range (as startx,endx,starty,endy) of the given
    // parameters in the table.
    // This is the minimum start value and maximum end value for all parameters.
    // An empty name pattern is the same as * (all parm names).
    vector<double> getRange (const string& parmNamePattern = "") const
      { return itsRep->getRange (parmNamePattern); }

    // Get parameter names in the table matching the pattern.
    // An empty name pattern is the same as * (all parm names).
    vector<string> getNames (const string& parmNamePattern = "") const
      { return itsRep->getNames (parmNamePattern); }

    // Get the values of the given parameters on the given regular grid
    // where v1/v2 represents center/width or start/end.
    // The vector values in the map are in fact 2-dim arrays with axes freq
    // and time. If freqStep and timeStep are not given (or given as <=0), the
    // default freq and time step from the ParmDB will be used.
    // <group>
    map<string, vector<double> > getValuesMap (const string& parmNamePattern,
                                               double freqv1, double freqv2,
                                               double freqStep,
                                               double timev1, double timev2,
                                               double timeStep,
                                               bool asStartEnd=false);
    map<string, vector<double> > getValuesMap (const string& parmNamePattern,
                                               double freqv1, double freqv2,
                                               double timev1, double timev2,
                                               bool asStartEnd=false)
      { return getValuesMap (parmNamePattern, freqv1, freqv2, 0,
                             timev1, timev2, asStartEnd); }
    // </group>

    // Get the values of the given parameters on the given regular grid
    // where v1/v2 represents center/width or start/end.
    // The Record contains a map of parameter name to Array<double>.
    // If freqStep and timeStep are not given (or given as <=0), the
    // default freq and time step from the ParmDB will be used.
    // Similar to getValuesGrid the resulting record also contains a subrecord
    // "_grid" containing the grid of the parameter values.
    // <group>
    casa::Record getValues (const string& parmNamePattern,
                            double freqv1, double freqv2, double freqStep,
                            double timev1, double timev2, double timeStep,
                            bool asStartEnd=true)
      { return itsRep->getValues (parmNamePattern, freqv1, freqv2, freqStep,
                                  timev1, timev2, timeStep, asStartEnd); }
    casa::Record getValues (const string& parmNamePattern,
                            double freqv1=-1e30, double freqv2=1e30,
                            double timev1=-1e30, double timev2=1e30,
                            bool asStartEnd=true);
    // </group>

    // Get the values of the given parameters on the given grid where v1/v2
    // represents center/width or start/end.
    // The Record contains a map of parameter name to Array<double>.
    // Similar to getValuesGrid the resulting record also contains a subrecord
    // "_grid" containing the grid of the parameter values.
    casa::Record getValues (const string& parmNamePattern,
                            const vector<double>& freqv1,
                            const vector<double>& freqv2,
                            const vector<double>& timev1,
                            const vector<double>& timev2,
                            bool asStartEnd=true)
      { return itsRep->getValues (parmNamePattern, freqv1, freqv2,
                                  timev1, timev2, asStartEnd); }

    // Get the values of the given parameters for the given domain.
    // The Record contains a map of parameter name to Array<value>.
    // Furthermore it contains a subrecord "_grid" containing the grid axes
    // used for each parameters. Their names have the form parmname;xx
    // where xx is freqs, freqwidths, times, and timewidths. Their values
    // are the center and width of each cell.
    casa::Record getValuesGrid (const string& parmNamePattern,
                                double freqv1=-1e30, double freqv2=1e30,
                                double timev1=-1e30, double timev2=1e30,
                                bool asStartEnd=true)
      { return itsRep->getValuesGrid (parmNamePattern, freqv1, freqv2,
                                      timev1, timev2, asStartEnd); }

    // Get the coefficients and possible errors for the given parameters
    // and domains.
    // The Record contains a map of parameter name to a subrecord.
    // The subrecord contains a map of 'v_i' to a subrecord where v_i
    // represents the i-th domain. Each subrecord contains the fields
    // coeff, error, and domain. Each of these fields contain an array of
    // doubles containing the values. The error array is empty if no errors
    // are stored.
    casa::Record getCoeff (const string& parmNamePattern,
                           double freqv1=-1e30, double freqv2=1e30,
                           double timev1=-1e30, double timev2=1e30,
                           bool asStartEnd=true)
      { return itsRep->getCoeff (parmNamePattern, freqv1, freqv2,
                                 timev1, timev2, asStartEnd); }


  private:
    // Convert a record to a map.
    map<string,vector<double> > record2Map (const casa::Record& rec) const;


    //# Data members
    ParmFacadeRep::ShPtr itsRep;
  };

  // @}

}} // namespaces

#endif
