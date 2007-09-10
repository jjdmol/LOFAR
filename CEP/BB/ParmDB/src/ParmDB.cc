//# ParmDB.cc: Object to hold parameters in a table.
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

#include <lofar_config.h>

#include <ParmDB/ParmDB.h>
#include <ParmDB/ParmDBAIPS.h>
#include <ParmDB/ParmDBPostgres.h>
#include <Common/LofarLogger.h>
#include <casa/Utilities/Regex.h>

#ifdef AIPS_NO_TEMPLATE_SRC
# include <casa/Utilities/GenSort.cc>     // for automatic template
#else
# include <casa/Utilities/GenSort.h>
#endif

using namespace std;
using namespace casa;

namespace LOFAR {
namespace ParmDB {

map<string,int>    ParmDB::theirDBNames;
vector<ParmDBRep*> ParmDB::theirParmDBs;


ParmDBRep::~ParmDBRep()
{}

void ParmDBRep::lock (bool)
{}

void ParmDBRep::unlock()
{}

ParmValue ParmDBRep::getDefValue (const string& parmName)
{
  // Fill the map with default values if not done yet.
  if (!itsDefFilled) {
    fillDefMap (itsDefValues);
    itsDefFilled = true;
  }
  // Try to find the default value.
  // The parameter name consists of parts (separated by colons), so the
  // parameters are categorised in that way.
  // An initial value can be defined for the full name or for a higher
  // category.
  // So look up until found or until no more parts are left.
  string name = parmName;
  while (true) {
    map<string,ParmValue>::const_iterator pos = itsDefValues.find (name);
    if (pos != itsDefValues.end()) {
      return pos->second;
    }
    string::size_type idx = name.rfind (':');
    // Exit loop if no more name parts.
    if (idx == string::npos) {
      break;
    }
    // Remove last part and try again.
    name = name.substr (0, idx);
  }
  // Nothing found; return an empty ParmValue.
  return ParmValue();
}

vector<string> ParmDBRep::getAllNames (const string& pattern,
				       ParmDBRep::TableType tableType)
{
  // First get all names for the standard table.
  vector<string> names = getNames (pattern, tableType);
  // Now append with names from the default table.
  getExprNames (pattern, names);
  // Now sort the result uniquely.
  // Most has already been sorted, so use insertion sort.
  int nr = GenSort<string>::sort (&names[0], names.size(),
				  Sort::Ascending,
				  Sort::InsSort|Sort::NoDuplicates);
  names.resize (nr);
  return names;
}

void ParmDBRep::getExprNames (const string& pattern,
			      vector<string>& names)
{
  // Fill the map with default values if not done yet.
  if (!itsDefFilled) {
    fillDefMap (itsDefValues);
    itsDefFilled = true;
  }
  Regex regex(Regex::fromPattern(pattern));
  for (map<string,ParmValue>::const_iterator iter = itsDefValues.begin();
       iter != itsDefValues.end();
       iter++) {
    if (iter->second.rep().itsType == "parmexpr") {
      String nm (iter->first);
      if (nm.matches (regex)) {
	names.push_back (iter->first);
      }
    }
  }
}



ParmDB::ParmDB (const ParmDBMeta& ptm, bool forceNew)
{
  // Attach to existing one if already opened.
  map<string,int>::iterator pos = theirDBNames.find (ptm.getTableName());
  if (pos != theirDBNames.end()) {
    itsRep = theirParmDBs[pos->second];
    itsRep->link();
    return;
  }
  // Open the correct ParmDB.
  if (ptm.getType() == "aips") {
    itsRep = new ParmDBAIPS (ptm.getTableName(), forceNew);
    ///  } else if (ptm.getType() == "bdb") {
    ///itsRep = new ParmDBBDB (ptm, forceNew);
  } else if (ptm.getType() == "postgres") {
#if defined(HAVE_PGSQL)
    itsRep = new ParmDBPostgres(ptm.getDBName(),
        ptm.getUserName(),
        ptm.getDBPwd(),
        ptm.getHostName(),
        "");
#else
    ASSERTSTR(false, "unsupported parmTableType: "<<ptm.getType());
#endif
  } else {
    ASSERTSTR(false, "unknown parmTableType: "<<ptm.getType());
  }
  itsRep->link();
  itsRep->setParmDBMeta (ptm);
  // Get the sequence number of the ParmDBs opened.
  uint dbnr = theirParmDBs.size();
  if (dbnr == theirDBNames.size()) {
    theirParmDBs.push_back (itsRep);
  } else {
    // Some entry has been deleted; reuse it.
    for (dbnr=0; dbnr<theirParmDBs.size(); ++dbnr) {
      if (theirParmDBs[dbnr] == 0) {
	theirParmDBs[dbnr] = itsRep;
	break;
      }
    }
  }
  itsRep->setParmDBSeqNr (dbnr);
  theirDBNames.insert (make_pair (ptm.getTableName(), dbnr));
}

ParmDB::ParmDB (ParmDBRep* rep)
: itsRep (rep)
{
  itsRep->link();
}

ParmDB::ParmDB (const ParmDB& that)
: itsRep (that.itsRep)
{
  itsRep->link();
}

ParmDB& ParmDB::operator= (const ParmDB& that)
{
  if (this != &that) {
    decrCount();
    itsRep = that.itsRep;
    itsRep->link();
  }
  return *this;
}

void ParmDB::decrCount()
{
  if (itsRep->unlink() == 0) {
    string tabName = itsRep->getParmDBMeta().getTableName();
    map<string,int>::iterator pos = theirDBNames.find (tabName);
    ASSERTSTR (pos != theirDBNames.end(),
	       "~ParmDB " << tabName << " not found in map");
    DBGASSERT (theirParmDBs[pos->second] == itsRep);
    theirParmDBs[pos->second] = 0;
    theirDBNames.erase (pos);
    delete itsRep;
    itsRep = 0;
  }
}

ParmDB ParmDB::getParmDB (uint index)
{
  ASSERTSTR (index < theirParmDBs.size()  &&  theirParmDBs[index] != 0,
	     "ParmDB index " << index << " is unknown");
  return ParmDB(theirParmDBs[index]);
}

vector<string> ParmDB::getExprNames (const string& pattern)
{
  vector<string> names;
  itsRep->getExprNames (pattern, names);
  return names;
}


/*
vector<ParmDBMeta> ParmDB::getAllParmDBMeta()
{
  vector<ParmDBMeta> res;
  res.reserve (theirParmDBs.size());
  for (uint i=0; i<theirParmDBs.size(); i++) {
    if (theirParmDBs[i] == 0) {
      res.push_back (ParmDBMeta());
    } else {
      res.push_back (theirParmDBs[i]->getParmDBMeta());
    }
  }
  return res;
}
*/

} // namespace ParmDB
} // namespace LOFAR
