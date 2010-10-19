//# SourceDB.cc: Class to hold source in a table.
//#
//# Copyright (C) 2008
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

#include <SourceDB/SourceDB.h>
#include <SourceDB/SourceDBAIPS.h>
#include <Common/LofarLogger.h>
#include <casa/Utilities/Regex.h>
#include <casa/Utilities/GenSort.h>

using namespace std;
using namespace casa;

namespace LOFAR {
namespace SourceDB {


SourceDBRep::~SourceDBRep()
{}

void SourceDBRep::lock (bool)
{}

void SourceDBRep::unlock()
{}



SourceDB::SourceDB (const SourceDBMeta& ptm, bool forceNew)
{
  // Open the correct SourceDB.
  if (ptm.getType() == "aips") {
    itsRep = new SourceDBAIPS (ptm.getTableName(), forceNew);
  } else if (ptm.getType() == "postgres") {
#if defined(HAVE_PGSQL)
    itsRep = new SourceDBPostgres(ptm.getDBName(),
        ptm.getUserName(),
        ptm.getDBPwd(),
        ptm.getHostName(),
        "");
#else
    ASSERTSTR(false, "unsupported sourceTableType: "<<ptm.getType());
#endif
  } else {
    ASSERTSTR(false, "unknown sourceTableType: "<<ptm.getType());
  }
  itsRep->link();
  itsRep->setSourceDBMeta (ptm);
}

SourceDB::SourceDB (SourceDBRep* rep)
: itsRep (rep)
{
  itsRep->link();
}

SourceDB::SourceDB (const SourceDB& that)
: itsRep (that.itsRep)
{
  itsRep->link();
}

SourceDB& SourceDB::operator= (const SourceDB& that)
{
  if (this != &that) {
    decrCount();
    itsRep = that.itsRep;
    itsRep->link();
  }
  return *this;
}

void SourceDB::decrCount()
{
  if (itsRep->unlink() == 0) {
    delete itsRep;
    itsRep = 0;
  }
}

void SourceDB::addSource (const SourceValue& value)
{
  list<SourceValue> values;
  values.push_back (value);
  addSources (values);
}

void SourceDB::createParmDB (ParmDB::ParmDB& parmdb)
{
  // Get all sources.
  list<SourceValue> sources = getSources(vector<string>(),
					 ParmDB::ParmDomain());
  // Lock the ParmDB for the bulk update.
  parmdb.lock (true);
  // Create the parm object once.
  ParmDB::ParmValue pv;
  pv.setNewParm();
  // Write them into the parameter database.
  // So far, only point sources are handled.
  for (list<SourceValue>::const_iterator iter = sources.begin();
       iter != sources.end();
       ++iter) {
    vector<int> shape;     // empty shape is scalar
    ASSERT (iter->getType() == "point");
    putParm (parmdb, pv, iter->getName(), "RA", iter->getDomain(),
	     iter->getRA(), false); 
    putParm (parmdb, pv, iter->getName(), "DEC", iter->getDomain(),
	     iter->getDEC(), false); 
    putParm (parmdb, pv, iter->getName(), "SpInx", iter->getDomain(),
	     iter->getSpectralIndex(), false); 
    putParm (parmdb, pv, iter->getName(), "StokesI", iter->getDomain(),
	     iter->getFlux()[0], false); 
    putParm (parmdb, pv, iter->getName(), "StokesQ", iter->getDomain(),
	     iter->getFlux()[1], false); 
    putParm (parmdb, pv, iter->getName(), "StokesU", iter->getDomain(),
	     iter->getFlux()[2], false); 
    putParm (parmdb, pv, iter->getName(), "StokesV", iter->getDomain(),
	     iter->getFlux()[3], false); 
  }
  parmdb.unlock();
}

void SourceDB::putParm (ParmDB::ParmDB& parmdb, ParmDB::ParmValue& pv,
			const string& sourceName,
			const string& parm,
			const ParmDB::ParmDomain& domain,
			double value, bool relativePert)
{
  pv.rep().setPerturbation (1e-6, relativePert);
  pv.rep().setCoeff (&value, vector<int>());
  pv.rep().setDomain (domain);
  parmdb.putValue (sourceName+':'+parm, pv);
}

void SourceDB::mergeParmDB (const ParmDB::ParmDB&)
{
  ASSERTSTR (false, "SourceDB::mergeParmDB not implemented yet");
}

} // namespace SourceDB
} // namespace LOFAR
