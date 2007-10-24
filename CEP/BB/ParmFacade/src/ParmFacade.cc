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
#include <BBSKernel/MNS/MeqRequest.h>
#include <BBSKernel/MNS/MeqParmFunklet.h>
#include <BBSKernel/MNS/MeqResult.h>
#include <BBSKernel/MNS/MeqMatrix.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <casa/Utilities/Regex.h>

using namespace std;
using namespace casa;
using namespace LOFAR::BBS;

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
	   ++iter) {
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
	   ++iter) {
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
	  for (iter=itsExprs.begin(); iter!=itsExprs.end(); ++iter) {
	    if (pname.matches (Regex(iter->first + ".*"))) {
	      break;
	    }
	  }
	}
	// A match has been found,
	// so get the range of the expression's children.
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
      return itsPDB.getAllNames(pp);
    }

    // Get the parameter values for the given parameters and domain.
    map<string, vector<double> >
    ParmFacade::getValues (const string& parmNamePattern,
			   double startx, double endx, int nx,
			   double starty, double endy, int ny)
    {
      return record2Map (getValuesRec (parmNamePattern,
				       startx, endx, nx,
				       starty, endy, ny));
    }

    Record ParmFacade::getValuesRec (const string& parmNamePattern,
				     double startx, double endx, int nx,
				     double starty, double endy, int ny)
    {
      // Get all parm names.
      vector<string> names = getNames (parmNamePattern);
      // The output is returned in a record.
      Record out;
      MeqParmGroup parmGroup;
      map<string,MeqExpr> exprs;
      // Loop through all parameters.
      for (vector<string>::const_iterator iter=names.begin();
	   iter != names.end();
	   ++iter) {
	// Create an expression object.
	exprs.insert (std::make_pair (*iter,
				      MeqParmFunklet::create (*iter,
							      parmGroup,
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
      //const vector<MeqParm*>& parmList = parmGroup.getParms();
      //for (vector<MeqParm*>::const_iterator iter = parmList.begin();
      // iter != parmList.end();
      // ++iter) {
      //  (*iter)->fillFunklets (parmValues, domain);
      //}
      for (MeqParmGroup::iterator iter = parmGroup.begin();
	   iter != parmGroup.end(); ++iter) {
	(*iter).second.fillFunklets (parmValues, domain);
      }
      // Define shape of result.
      IPosition shape(2,nx,ny);
      // Evaluate all expressions and store the result in the output map.
      for (map<string,MeqExpr>::iterator iter = exprs.begin();
	   iter != exprs.end();
	   ++iter) {
	// Get result and put in record.
	// Note that putting an array in Record makes a deep copy, so we
	// can take over the MeqMatrix storage.
	MeqResult resVal = iter->second.getResult (req);
	const MeqMatrix& mat = resVal.getValue();
	const double* ptr = mat.doubleStorage();
	Array<double> arr;
	// Store the result in the array.
	// If the result is a scalar, expand it.
	if (mat.nelements() == 1) {
	  arr = *ptr;
	  out.define (iter->first, arr);
	} else {
	  ASSERT (nx*ny == mat.nelements());
	  out.define (iter->first, Array<double>(shape,
						 const_cast<double*>(ptr),
						 SHARE));
	}
      }
      return out;
    }

    // Get the coefficients of parameters of type 'history' matching the
    // parmNamePattern and the specified domain.
    // This is a temporary hack to facillitate viewing the solve
    // history in the SAS GUI.
    map<string, vector<double> >
    ParmFacade::getHistory (const string& parmNamePattern,
			    double startx, double endx,
			    double starty, double endy,
			    double startSolveTime, double endSolveTime)
    {
      return record2Map (getHistoryRec (parmNamePattern,
					startx, endx,
					starty, endy,
					startSolveTime, endSolveTime));
    }

    Record ParmFacade::getHistoryRec (const string& parmNamePattern,
				      double startx, double endx,
				      double starty, double endy,
				      double startSolveTime,
				      double endSolveTime)
    {
      // Get ParmValues of all matching parameters (all timestamps).
      map<string, ParmValueSet> parmValues;
      itsPDB.getValues (parmValues, parmNamePattern,
			ParmDomain(startx, endx, starty, endy), -1);
      // The output is returned in a record.
      Record out;
      // Loop over all parameters.
      // A parameter can have multiple ParmValues, one for each timestamp.
      // Parameters of type 'history' have exactly one coefficient.
      // For each parameter, create an array of coefficients
      // (one coefficient per timestamp), sorted on timestamp.
      vector<double> coeff;
      for (map<string,ParmValueSet>::iterator it = parmValues.begin();
	   it != parmValues.end();
	   ++it) {
	coeff.resize (0);
	vector<ParmValue>& values = it->second.getValues();
	// Sort ParmValues on timestamp.
	sort(values.begin(), values.end(), &pvLess);
        
	for (vector<ParmValue>::const_iterator value_it = values.begin();
	     value_it != values.end();
	     ++value_it) {
	  // Only take ParmValues of type 'history' with a timestamp
	  // within the specified time range.
	  if (value_it->rep().itsType == "history" && 
	      value_it->rep().itsTimeStamp >= startSolveTime &&
	      value_it->rep().itsTimeStamp <= endSolveTime) {
	    ASSERTSTR(value_it->rep().itsCoeff.size() == 1,
		      "parms of type 'history' should have exactly one coefficient.");
	    coeff.push_back(value_it->rep().itsCoeff[0]);
	  }
	}
	out.define (it->first,
		    Array<double> (IPosition(1,coeff.size()),
				   &(coeff[0]), SHARE));
      }        
      return out;
    }

    map<string,vector<double> >
    ParmFacade::record2Map (const Record& rec) const
    {
      map<string, vector<double> > out;
      // Copy all values from the record to the map.
      for (uint i=0; i<rec.nfields(); ++i) {
	// First make empty vecor; thereafter copy values to it.
	vector<double>& vec = out[rec.name(i)];
	ASSERT (vec.size() == 0);
	// Get result and put in map.
	const Array<double>& arr = rec.asArrayDouble(i);
	bool deleteIt;
	const double* ptr = arr.getStorage (deleteIt);
	// Store the result in the vector.
	vec.assign (ptr, ptr+arr.nelements());
      }
      return out;
    }

    bool ParmFacade::pvLess (const ParmValue& left, const ParmValue& right)
    {
      return (left.rep().itsTimeStamp < right.rep().itsTimeStamp);
    }

  } // namespace ParmDB
} // namespace LOFAR
