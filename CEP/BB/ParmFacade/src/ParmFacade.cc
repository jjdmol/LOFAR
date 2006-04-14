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
#include <BBS3/MNS/MeqRequest.h>
#include <BBS3/MNS/MeqStoredParmPolc.h>
#include <BBS3/MNS/MeqResult.h>
#include <BBS3/MNS/MeqMatrix.h>
#include <Common/LofarLogger.h>

using namespace std;

namespace LOFAR {
namespace ParmDB {

  ParmFacade::ParmFacade (const string& tableName)
    : itsPDB(ParmDBMeta("aips", tableName))
  {}

  ParmFacade::~ParmFacade()
  {}

  vector<double> ParmFacade::getRange (const string& parmNamePattern) const
  {
    ParmDomain dom = itsPDB.getRange (parmNamePattern);
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
    return itsPDB.getNames (parmNamePattern);
  }

  // Get the parameter values for the given parameters and domain.
  map<string, vector<double> >
    ParmFacade::getValues(const string& parmNamePattern,
			  double startx, double endx, int nx,
			  double starty, double endy, int ny)
  {
    // Get the parm values.
    map<string,ParmValueSet> parmValues;
    itsPDB.getValues (parmValues, parmNamePattern,
		      ParmDomain(startx, endx, starty, endy));
    // Make a request for the given grid.
    MeqDomain domain(startx, endx, starty, endy);
    MeqRequest req(domain, nx, ny);
    // The output is returned in a map.
    map<string,vector<double> > out;
    MeqParmGroup parmGroup;
    // Loop through all parameters.
    for (map<string,ParmValueSet>::const_iterator iter=parmValues.begin();
	 iter != parmValues.end();
	 iter++) {
      // Add it to the output map and get a reference to the vector.
      out.insert (std::make_pair (iter->first, vector<double>()));
      map<string,vector<double> >::iterator outiter = out.find (iter->first);
      vector<double>& vec = outiter->second;
      // Create a ParmFunklet object and fill it.
      MeqStoredParmPolc* parmp = new MeqStoredParmPolc(iter->first,
						       &parmGroup, &itsPDB);
      parmp->fillPolcs (parmValues, domain);
      MeqExpr expr(parmp);
      // Evaluate it and get the value.
      MeqResult resVal = expr.getResult (req);
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
