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
#include <Common/StringUtil.h>
#include <casa/Utilities/Regex.h>

using namespace std;
using namespace casa;

// Create tParmFacade.in_mep with parmdb using:
//   create tablename='tParmFacade.in_mep'
//   add parm1 domain=[1,5,4,10],values=2
//   add parm2 domain=[1,5,4,10],values=[2,0.1],nx=2
//   add parm3 type='expression',expression='parm1*parm2'

namespace LOFAR {
namespace ParmDB {

  ParmFacade::ParmFacade (const string& tableName)
    : itsPDB(ParmDBMeta("aips", tableName))
  {
    map<string,ParmValueSet> defValues;
    itsPDB.getDefValues (defValues, "*");
    for (map<string,ParmValueSet>::const_iterator iter = defValues.begin();
	 iter != defValues.end();
	 iter++) {
      const ParmValueRep& pval = iter->second.getValues()[0].rep();
      if (pval.itsType == "parmexpr") {
	itsExprs[iter->first] = set<string>();
	// Add the expression to the map.
	map<string,set<string> >::iterator ch = itsExprs.find(iter->first);
	// Add all children to the set.
	processExpr (defValues, pval.itsExpr, ch->second);
      }
    }
  }

  ParmFacade::~ParmFacade()
  {}

  void ParmFacade::processExpr (const map<string,ParmValueSet>& defValues, 
				const string& expr, set<string>& ch)
  {
    String str(expr);
    // Replace function names by a blank..
    str.gsub (Regex("[a-zA-Z_][a-zA-Z0-9_]* *\\("), " ");
    // Replace parentheses, commas, and operators by a blank.
    str.gsub (Regex("[(),*/+^-]") , " ");
    // Replace multiple blanks by a single one.
    str.gsub (Regex("  *") , " ");
    // Now split the string and append to the children set.
    vector<string> nms = StringUtil::split(str, ' ');
    for (vector<string>::const_iterator iter=nms.begin();
	 iter != nms.end();
	 iter++) {
      if (! iter->empty()) {
	// If the name is an expression, process it recursively.
	bool isExpr = false;
	map<string,ParmValueSet>::const_iterator def = defValues.find (*iter);
	if (def != defValues.end()) {
	  const ParmValueRep& pval = def->second.getValues()[0].rep();
	  if (pval.itsType == "parmexpr") {
	    isExpr = true;
	    processExpr (defValues, pval.itsExpr, ch);
	  }
	}
	if (!isExpr) {
	  // No expression, so add parm name.
	  ch.insert (*iter);
	}
      }
    }
  }

  vector<double> ParmFacade::getRange (const string& parmNamePattern) const
  {
    string pp = parmNamePattern;
    if (pp.empty()) {
      pp = "*";
    }
    ParmDomain dom = itsPDB.getRange (pp);
    // If no domain found, see if the name matches an expression.
    if (dom.getStart()[0] == 0  &&  dom.getEnd()[0] == 1  &&
	dom.getStart()[1] == 0  &&  dom.getEnd()[1] == 1) {
      map<string,set<string> >::const_iterator iter =
	itsExprs.find (parmNamePattern);
      if (iter == itsExprs.end()) {
	// Not found; see if the name is part of an expression name.
	String pname(parmNamePattern);
	for (iter=itsExprs.begin(); iter!=itsExprs.end(); iter++) {
	  if (pname.matches (Regex(iter->first + ".*"))) {
	    break;
	  }
	}
      }
      // A match has been found, so get the range of the expression's children.
      if (iter != itsExprs.end()) {
	// Copy the set to a vector.
	vector<string> nams(iter->second.begin(), iter->second.end());
	dom = itsPDB.getRange (nams);
      }
    }
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
    ParmFacade::getValues (const string& parmNamePattern,
			   double startx, double endx, int nx,
			   double starty, double endy, int ny)
  {
    // Get all parm names.
    vector<string> names = getNames (parmNamePattern);
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
    // Get the parm values of parameters.
    // All parameters are needed to deal with possible parmexpressions.
    map<string,ParmValueSet> parmValues;
    itsPDB.getValues (parmValues, vector<string>(),
		      ParmDomain(startx, endx, starty, endy));
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

  // Get the parameter coefficients for the given parameters and domain.
  map<string, vector<double> >
    ParmFacade::getHistory (const string& parmNamePattern,
			    double startx, double endx,
			    double starty, double endy,
			    double startSolveTime, double endSolveTime)
  {
    // Get parm values (all timestamps).
    map<string,ParmValueSet> parmValues;
    itsPDB.getValues (parmValues, parmNamePattern,
		      ParmDomain(startx, endx, starty, endy),
		      -1);
    // The output is returned in a map.
    map<string,vector<double> > out;
    // Loop over all parameters and store coefficients in output map.
    vector<ParmValue> vals;
    for (map<string,ParmValueSet>::iterator iter = parmValues.begin();
	 iter != parmValues.end();
	 iter++) {
      // Only take timestamps within time range.
      const vector<ParmValue>& values = iter->second.getValues();
      vals.clear();
      vals.reserve (values.size());
      uint maxNrAxes = 0;
      for (vector<ParmValue>::const_iterator vit = values.begin();
	   vit != values.end();
	   vit++) {
	if (vit->rep().itsTimeStamp >= startSolveTime  &&
	    vit->rep().itsTimeStamp <= endSolveTime) {
	  vals.push_back (*vit);
	  if (vit->rep().itsDomain.getStart().size() > maxNrAxes) {
	    maxNrAxes = vit->rep().itsDomain.getStart().size();
	  }
	}
      }
      if (vals.size() > 0) {
	// Sort the values of a parm in order of domain and timestamp.
	sort (vals.begin(), vals.end(), &pvLess);
	// Collect the coefficients for each domain.
	// First initialize variables for testing on dmain.
	uint nrAxes = maxNrAxes+1;
	vector<double> st;
	vector<double> en;
	st.reserve (maxNrAxes);
	en.reserve (maxNrAxes);
	vector<double> coeff;
	string domainString;
	int nDomain = 0;
	// Loop over all values and test for a new domain.
	for (vector<ParmValue>::const_iterator vit = vals.begin();
	     vit != vals.end();
	     vit++) {
	  const ParmDomain& dom = vit->rep().itsDomain;
	  bool newDomain = false;
	  if (dom.getStart().size() != nrAxes) {
	    newDomain = true;
	  } else {
	    for (uint i=0; i<dom.getStart().size(); ++i) {
	      if (dom.getStart()[i] != st[i]  ||  dom.getEnd()[i] != en[i]) {
		newDomain = true;
		break;
	      }
	    }
	  }
	  if (newDomain) {
	    // A new domain, so add the values to the map (except first time).
	    if (nDomain > 0) {
	      out[iter->first + domainString] = coeff;
	    }
	    // Reset everything.
	    nDomain++;
	    nrAxes = dom.getStart().size();
	    ostringstream os;
	    os << ":domain" << nDomain;
	    domainString = os.str();
	    st = dom.getStart();
	    en = dom.getEnd();
	    coeff.clear();
	    coeff.push_back (vit->rep().itsCoeff.size());
	  }
	  // Append the coefficients to the vector.
	  coeff.insert (coeff.end(),
			vit->rep().itsCoeff.begin(),
			vit->rep().itsCoeff.end());
	}
	// Store last domain.
	out[iter->first + domainString] = coeff;
      }
    }
    return out;
  }

  // Order parms on domain and timestamp.
  bool ParmFacade::pvLess (const ParmValue& left, const ParmValue& right)
  {
    const ParmDomain& ldom = left.rep().itsDomain;
    const ParmDomain& rdom = right.rep().itsDomain;
    if (ldom.getStart().size() < rdom.getStart().size()) return true;
    if (ldom.getStart().size() > rdom.getStart().size()) return false;
    for (uint i=0; i<ldom.getStart().size(); ++i) {
      if (ldom.getStart()[i] < rdom.getStart()[i]) return true;
      if (ldom.getStart()[i] > rdom.getStart()[i]) return false;
      if (ldom.getEnd()[i] < rdom.getEnd()[i]) return true;
      if (ldom.getEnd()[i] > rdom.getEnd()[i]) return false;
    }
    return (left.rep().itsTimeStamp < right.rep().itsTimeStamp);
  }

} // namespace ParmDB
} // namespace LOFAR
