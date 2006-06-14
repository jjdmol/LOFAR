//# ParmFacade.cc: Object access the parameter database
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

#include <ParmFacade/ParmFacade.h>
#include <BBS/MNS/MeqRequest.h>
#include <BBS/MNS/MeqParmFunklet.h>
#include <BBS/MNS/MeqResult.h>
#include <BBS/MNS/MeqMatrix.h>
#include <Common/LofarLogger.h>

using namespace std;

// Create tParmFacade.in_mep with parmdb using:
//   create tablename='tParmFacade.in_mep'
//   add parm1 domain=[1,5,4,10],values=2
//   add parm2 domain=[1,5,4,10],values=[2,0.1],nx=2
//   add parm3 type='expression',expression='parm1*parm2'

namespace LOFAR {
namespace ParmDB {

  ParmFacade::ParmFacade (const string& tableName)
    : itsPDB(ParmDBMeta("aips", tableName))
  {}

  ParmFacade::~ParmFacade()
  {}

  vector<double> ParmFacade::getRange (const string& parmNamePattern) const
  {
    string pp = parmNamePattern;
    if (pp.empty()) {
      pp = "*";
    }
    ParmDomain dom = itsPDB.getRange (pp);
    vector<double> res(4);
    res[0] = dom.getStart()[0];
    res[1] = dom.getEnd()[0];
    res[2] = dom.getStart()[1];
    res[3] = dom.getEnd()[1];
    return res;
  }

  // Get all parameter names in the table.
  vector<string> ParmFacade::getNames (const string& parmNamePattern) const
  {
    string pp = parmNamePattern;
    if (pp.empty()) {
      pp = "*";
    }
    return itsPDB.getAllNames (pp);
  }

  // Get the parameter values for the given parameters and domain.
  map<string, vector<double> >
    ParmFacade::getValues(const string& parmNamePattern,
			  double startx, double endx, int nx,
			  double starty, double endy, int ny)
  {
    // Get all parm names.
    vector<string> names = getNames (parmNamePattern);
    // Get the parm values.
    map<string,ParmValueSet> parmValues;
    itsPDB.getValues (parmValues, names,
		      ParmDomain(startx, endx, starty, endy));
    // The output is returned in a map.
    map<string,vector<double> > out;
    MeqParmGroup parmGroup;
    map<string,MeqExpr> exprs;
    // Loop through all parameters.
    for (vector<string>::const_iterator iter=names.begin();
	 iter != names.end();
	 iter++) {
      // Create an expression object.
      exprs.insert (std::make_pair (*iter,
				    MeqParmFunklet::create (*iter,
							    &parmGroup,
							    &itsPDB)));
    }
    // Make a request object for the given grid.
    MeqDomain domain(startx, endx, starty, endy);
    MeqRequest req(domain, nx, ny);
    // Initialize all parameters.
    const vector<MeqParm*>& parmList = parmGroup.getParms();
    for (vector<MeqParm*>::const_iterator iter = parmList.begin();
	 iter != parmList.end();
	 ++iter) {
      (*iter)->fillFunklets (parmValues, domain);
    }
    // Evaluate all expressions and store the result in the output map.
    for (map<string,MeqExpr>::iterator iter = exprs.begin();
	 iter != exprs.end();
	 ++iter) {
      // Add it to the output map and get a reference to the vector.
      out.insert (std::make_pair (iter->first, vector<double>()));
      map<string,vector<double> >::iterator outiter = out.find (iter->first);
      vector<double>& vec = outiter->second;
      // Get result and put in map.
      MeqResult resVal = iter->second.getResult (req);
      const MeqMatrix& mat = resVal.getValue();
      const double* ptr = mat.doubleStorage();
      // Store the result in the vector.
      // If the result is a scalar, expand it.
      if (mat.nelements() == 1) {
	vec.assign (nx*ny, *ptr);
      } else {
	ASSERT (nx*ny == mat.nelements());
	vec.assign (ptr, ptr+nx*ny);
      }
    }
    return out;
  }

} // namespace ParmDB
} // namespace LOFAR
