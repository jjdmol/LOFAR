//# MeqStoredParmPolc.cc: Stored parameter with polynomial coefficients
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
#include <BBS3/MNS/MeqStoredParmPolc.h>
#include <Common/LofarLogger.h>
#include <casa/Arrays/Matrix.h>
#include <casa/BasicMath/Math.h>

using namespace casa;

namespace LOFAR {

MeqStoredParmPolc::MeqStoredParmPolc (const string& name, MeqParmGroup* group,
				      ParmDB::ParmDB* table)
: MeqParmPolc (name, group),
  itsTable    (table)
{}

MeqStoredParmPolc::~MeqStoredParmPolc()
{}

ParmDB::ParmDBMeta MeqStoredParmPolc::getParmDBMeta() const
{
  return itsTable->getParmDBMeta();
}

void MeqStoredParmPolc::readPolcs (const MeqDomain& domain)
{
  // Find the polc(s) for the given domain.
  ParmDB::ParmDomain pdomain = domain.toParmDomain();
  ParmDB::ParmValueSet pset = itsTable->getValues (getName(), pdomain);
  vector<ParmDB::ParmValue>& vec = pset.getValues();
  vector<MeqPolc> polcs;
  // If none found, try to get a default value.
  if (vec.size() == 0) {
    ParmDB::ParmValue pval = itsTable->getDefValue (getName());
    ASSERTSTR (!pval.rep().itsType.empty(),
	       "No value found for parameter " << getName());
    ASSERTSTR (pval.rep().itsType=="polc",
	       "No 'polc' funklet found for parameter " << getName());
    ASSERTSTR (pval.rep().itsShape.size()==2,
	       "No 2-dim funklet found for parameter " << getName());
    pval.rep().setDomain (pdomain);
    polcs.push_back (MeqPolc(pval));
  } else {
    // Check if the polc domains cover the entire domain and if they
    // do not overlap.
    // Not implemented yet!!!
    // Convert all to polcs.
    for (uint i=0; i<vec.size(); ++i) {
      ASSERTSTR (vec[i].rep().itsType=="polc",
		 "No 'polc' funklet found for parameter " << getName());
      ASSERTSTR (vec[i].rep().itsShape.size()==2,
		 "No 2-dim funklet found for parameter " << getName());
      polcs.push_back (MeqPolc(vec[i]));
    }
  }
  setPolcs (polcs);
  itsDomain = domain;
}

int MeqStoredParmPolc::initDomain (const MeqDomain& domain, int spidIndex)
{
  ASSERTSTR (domain==itsDomain, "MeqStoredParmPolc::initDomain - "
	     "domain mismatches domain given in last readPolcs");
  if (isSolvable()) {
    const vector<MeqPolc>& polcs = getPolcs();
    ASSERTSTR (polcs.size() == 1, "Solvable parameter " << getName() <<
	       " has multiple matching domains for time "
	       << domain.startX() << ':' << domain.endX() << " and freq "
	       << domain.startY() << ':' << domain.endY());
    const MeqDomain& polDom = polcs[0].domain();
    ASSERTSTR (near(domain.startX(), polDom.startX())  &&
	       near(domain.endX(), polDom.endX())  &&
	       near(domain.startY(), polDom.startY())  &&
	       near(domain.endY(), polDom.endY()),
	       "Solvable parameter " << getName() <<
	       " has a partially instead of fully matching entry for time "
		 << domain.startX() << ':' << domain.endX() << " and freq "
		 << domain.startY() << ':' << domain.endY() << endl
		 << "(" << polDom.startX() << ":" << polDom.endX()
		 << ", " << polDom.startY() << ":" << polDom.endY()) ;


  }
  return MeqParmPolc::initDomain (domain, spidIndex);
}

void MeqStoredParmPolc::updateFromTable()
{
  const vector<MeqPolc>& polcs = getPolcs();
  ASSERT(1 == polcs.size());
  ASSERTSTR (polcs[0].domain()==itsDomain,
	     "MeqStoredParmPolc::updateFromTable - "
	     "domain mismatches domain given in itsPolcs");
  // Find the polc(s) for the current domain.
  ParmDB::ParmDomain pdomain = itsDomain.toParmDomain();
  ParmDB::ParmValueSet pset = itsTable->getValues (getName(), pdomain);
  ASSERT(pset.getValues().size() == 1);
  MeqPolc newpolc (pset.getValues()[0]);
  // Update the coefficients with the new ones.
  update (newpolc.getCoeff());
}

void MeqStoredParmPolc::save()
{
  vector<MeqPolc>& polcs = getPolcs();
  for (unsigned int i=0; i<polcs.size(); i++) {
    ParmDB::ParmValue pval = polcs[i].getParmValue();
    itsTable->putValue (getName(), pval);
  }
  MeqParmPolc::save();
}

}
