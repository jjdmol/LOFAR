//# ParmTableDB.cc: Object to hold parameters in a database table.
//#
//# Copyright (C) 2002
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

#include <MNS/ParmTableDB.h>
#include <MNS/MeqDomain.h>
#include <MNS/TPOParm.h>
#include <PL/Query.h>
#include <PL/Attrib.h>
#include <PL/Collection.h>
#include <Common/Debug.h>
#include <aips/Mathematics/Math.h>

using namespace LOFAR::PL;


ParmTableDB::ParmTableDB (const string& dbType, const string& tableName,
			  const string& userName, const string& passwd)
: itsTableName (tableName)
{
  itsBroker.connect (userName, dbType, passwd);
}

ParmTableDB::~ParmTableDB()
{}

vector<MeqPolc> ParmTableDB::getPolcs (const string& parmName,
				       int, int,
				       const MeqDomain& domain)
{
  vector<MeqPolc> result;
  TPOMParm tpoparm;
  tpoparm.tableName (itsTableName);
  MParmSet set = tpoparm.retrieve (attrib(tpoparm,"name") == parmName);
  MParmSet::const_iterator iter = set.begin();
  for (; iter!=set.end(); iter++) {
    result.push_back (iter->data().getPolc());
  }
  return result;
}

MeqPolc ParmTableDB::getInitCoeff (const string& parmName,
				   int srcnr, int statnr)
{
  // Try to find the default initial values in the InitialValues subtable.
  // The parameter name consists of parts (separated by dots), so the
  // parameters are categorised in that way.
  // An initial value can be defined for the full name or for a higher
  // category.
  // So look up until found or until no more parts are left.
  MeqPolc result;
  string name = parmName;
  while (true) {
    TPOMParmDef tpoparm;
    tpoparm.tableName (itsTableName+"Def");
    MParmDefSet set = tpoparm.retrieve (attrib(tpoparm,"name") == name);
    if (! set.empty()) {
      Assert (set.size() == 1);
      MParmDefSet::const_iterator iter = set.begin();
      return iter->data().getPolc();
    }
    string::size_type idx = name.rfind ('.');
    // Exit loop if no more name parts.
    if (idx == string::npos) {
      break;
    }
    // Remove last part and try again.
    name = name.substr (0, idx);
  }
  return result;
}
				    
void ParmTableDB::putCoeff (const string& parmName,
			    int srcnr, int statnr,
			    const MeqPolc& polc)
{
  const MeqDomain& domain = polc.domain();
  MParmSet set = find (parmName, srcnr, statnr, domain);
  if (! set.empty()) {
    AssertMsg (set.size()==1, "Parameter " << parmName <<
		 " has multiple entries for time "
		 << domain.startX() << ':' << domain.endX() << " and freq "
		 << domain.startY() << ':' << domain.endY());
    TPOMParm tpoparm = *(set.begin());
    MeqParmHolder& parm = tpoparm.data();
    const MeqDomain& pdomain = parm.getPolc().domain();
    AssertMsg (near(domain.startX(), pdomain.startX())  &&
	       near(domain.endX(), pdomain.endX())  &&
	       near(domain.startY(), pdomain.startY())  &&
	       near(domain.endY(), pdomain.endY()),
	       "Parameter " << parmName <<
	       " has a partially instead of fully matching entry for time "
		 << domain.startX() << ':' << domain.endX() << " and freq "
		 << domain.startY() << ':' << domain.endY());
    MeqPolc newPolc = parm.getPolc();
    newPolc.setCoeff (polc.getCoeff());
    parm.setPolc (newPolc);
    tpoparm.update();
  } else {
    MeqParmHolder parm(parmName, srcnr, statnr, polc);
    TPOMParm tpoparm(parm);
    tpoparm.tableName (itsTableName);
    tpoparm.insert();
  }
}

MParmSet ParmTableDB::find (const string& parmName,
			    int srcnr, int statnr,
			    const MeqDomain& domain)
{
  // First see if the parameter name exists at all.
  TPOMParm tpoparm;
  tpoparm.tableName (itsTableName);
  MParmSet set = tpoparm.retrieve (attrib(tpoparm,"name") == parmName);
  if (set.empty()) {
    return set;
  }
  MParmSet set2;
  MParmSet::iterator iter = set.begin();
  for (; iter!=set.end(); iter++) {
    // Find all rows overlapping the requested domain.
    const MeqDomain& pdomain = iter->data().getPolc().domain();
    if (domain.startX() < pdomain.startX() &&
	domain.endX()   > pdomain.endX()   &&
	domain.startY() < pdomain.startY() &&
	domain.endY()   > pdomain.endY()) {
      set2.add (*iter);
    }
  }
  return set2;
}

void ParmTableDB::getSources (vector<string>& nams, vector<int>& srcs)
{
  // Get all parm rows containing RA in the name.
  // Use both tables.
  TPOMParm tpoparm;
  tpoparm.tableName (itsTableName);
  MParmSet set = tpoparm.retrieve (attrib(tpoparm,"name").like ("RA.*"));
  TPOMParmDef tpoparmdef;
  tpoparmdef.tableName (itsTableName+"Def");
  MParmDefSet dset = tpoparmdef.retrieve
    (attrib(tpoparmdef,"name").like ("RA.*"));
  nams.resize(0);
  srcs.resize(0);
  nams.reserve (set.size() + dset.size());
  srcs.reserve (set.size() + dset.size());
  MParmSet::iterator iter = set.begin();
  for (; iter!=set.end(); iter++) {
    nams.push_back (iter->data().getName());
    srcs.push_back (iter->data().getSourceNr());
  }
  MParmDefSet::iterator diter = dset.begin();
  for (; diter!=dset.end(); diter++) {
    nams.push_back (diter->data().getName());
    srcs.push_back (diter->data().getSourceNr());
  }
}

void ParmTableDB::unlock()
{}
