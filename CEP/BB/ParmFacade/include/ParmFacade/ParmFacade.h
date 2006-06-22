//# ParmFacade.h: Object access the parameter database
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

#ifndef LOFAR_PARMFACADE_PARMFACADE_H
#define LOFAR_PARMFACADE_PARMFACADE_H

// \file
// Object to hold parameters in a table.

//# Includes
#include <ParmDB/ParmDB.h>
#include <vector>
#include <map>
#include <set>


namespace LOFAR {
namespace ParmDB {


// \ingroup ParmFacade
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
//      of the domains of the parameters.
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
  ParmFacade (const std::string& tableName);

  // The destructor disconnects.
  ~ParmFacade();

  // Get the domain range (as startx,endx,starty,endy) of the given
  // parameters in the table.
  // This is the minimum start value and maximum end value for all parameters.
  // An empty name pattern is the same as * (all parm names).
  std::vector<double> getRange (const std::string& parmNamePattern = "") const;

  // Get parameter names in the table matching the pattern.
  // An empty name pattern is the same as * (all parm names).
  std::vector<std::string> getNames
                               (const std::string& parmNamePattern = "") const;

  // Get the parameter values for the given parameters and domain.
  // The domain is given by the start and end values, while the grid is
  // given by nx and ny.
  std::map<std::string, std::vector<double> >
    getValues (const std::string& parmNamePattern,
	       double startx, double endx, int nx,
	       double starty, double endy, int ny);

  // Get the expressions and the names of their children.
  const std::map<std::string,std::set<std::string> >& getExprs() const
    { return itsExprs; }

private:
  // Process a parmexp and add the children names to the vector.
  // It is done recursively, so a child can be an expression.
  void processExpr (const std::map<std::string,ParmValueSet>& defValues, 
		    const std::string& expr, std::set<std::string>& ch);

  ParmDB itsPDB;
  std::map<std::string,std::set<std::string> > itsExprs;
};

// @}

} // namespace ParmDB
} // namespace LOFAR

#endif
