//# ParmPolcStored.cc: Stored parameter with polynomial coefficients
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

#include <MEQ/ParmPolcStored.h>
#include <Common/Debug.h>
#include <Common/Lorrays.h>
#include <aips/Mathematics/Math.h>

namespace Meq {

ParmPolcStored::ParmPolcStored()
: ParmPolc (""),
  itsTable (0)
{}

ParmPolcStored::ParmPolcStored (const string& name, ParmTable* table,
				const Vells& defaultValue)
: ParmPolc   (name),
  itsTable   (table),
  itsDefault (defaultValue)
{}

ParmPolcStored::ParmPolcStored (const string& name, const string& tableName,
				const Vells& defaultValue)
: ParmPolc   (name),
  itsTable   (0),
  itsDefault (defaultValue)
{
  if (! tableName.empty()) {
    itsTable = ParmTable::openTable(tableName);
  }
}

ParmPolcStored::~ParmPolcStored()
{}

int ParmPolcStored::initDomain (const Domain& domain, int spidIndex)
{
  // Find the polc(s) for the given domain.
  vector<Polc> polcs;
  if (itsTable) {
    polcs = itsTable->getPolcs (getName(), domain);
  }
  // If none found, try to get a default value.
  if (polcs.size() == 0) {
    Polc polc;
    if (itsTable) {
      polc = itsTable->getInitCoeff (getName());
    } else {
      polc.setCoeff (itsDefault);
      polc.setSimCoeff (itsDefault);
      polc.setPertSimCoeff (Vells(0.));
      polc.setNormalize (true);
    }
    AssertMsg (!polc.getCoeff().isNull(), "No value found for parameter "
	       << getName());
    polc.setDomain (domain);
    // If needed, normalize the initial values.
    if (polc.isNormalized()) {
      polc.setCoeffOnly (polc.normalize(polc.getCoeff(), domain));
      polc.setSimCoeff  (polc.normalize(polc.getSimCoeff(), domain));
    }
    polcs.push_back (polc);
  } else if (isSolvable()) {
    AssertMsg (polcs.size() == 1, "Solvable parameter " << getName() <<
	       " has multiple matching domains for freq "
	       << domain.startFreq() << ':' << domain.endFreq()
	       << " and time "
	       << domain.startTime() << ':' << domain.endTime());
    const Domain& polDom = polcs[0].domain();
    AssertMsg (near(domain.startFreq(), polDom.startFreq())  &&
	       near(domain.endFreq(), polDom.endFreq())  &&
	       near(domain.startTime(), polDom.startTime())  &&
	       near(domain.endTime(), polDom.endTime()),
	       "Solvable parameter " << getName() <<
	       " has a partially instead of fully matching entry for freq "
	       << domain.startFreq() << ':' << domain.endFreq()
	       << " and time "
	       << domain.startTime() << ':' << domain.endTime());
  } else {
    // Check if the polc domains cover the entire domain and if they
    // do not overlap.
  }
  setPolcs (polcs);
  return ParmPolc::initDomain (domain, spidIndex);
}

void ParmPolcStored::save()
{
  const vector<Polc>& polcs = getPolcs();
  for (unsigned int i=0; i<polcs.size(); i++) {
    if (itsTable) {
      itsTable->putCoeff (getName(), polcs[i]);
    }
  }
  ParmPolc::save();
}

void ParmPolcStored::init (DataRecord::Ref::Xfer& initrec, Forest* frst)
{
  Node::init (initrec, frst);
  // Get default value.
  if (state()[AidDefault].exists()) {
    LoMat_double& val = wstate()[AidDefault].as_wr<LoMat_double>();
    itsDefault = Vells(&val);
  }
  // Get possible ParmTable name and open it.
  string tableName;
  if (state()[AidTablename].exists()) {
    tableName = state()[AidTablename].as<string>();
  }
  if (! tableName.empty()) {
    itsTable = ParmTable::openTable(tableName);
  }
  // Set the name of the parm.
  setName (name());
}

void ParmPolcStored::setState (const DataRecord& rec)
{
}

string ParmPolcStored::sdebug (int detail, const string& prefix,
			       const char* nm) const
{
  string out;
  Debug::appendf(out,"%s(%s)", nm?nm:"Meq::Parm", getName().c_str());
  if (itsTable) {
    Debug::appendf(out,"  parmtable=%s", itsTable->name().c_str());
  }
  return out;
}

} // namespace Meq
