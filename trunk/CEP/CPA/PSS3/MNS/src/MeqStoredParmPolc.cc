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

#include <MNS/MeqStoredParmPolc.h>
#include <Common/Debug.h>
#include <aips/Arrays/Matrix.h>
#include <aips/Mathematics/Math.h>

MeqStoredParmPolc::MeqStoredParmPolc (const string& name, ParmTable* table)
: MeqParmPolc (name),
  itsTable    (table)
{}

MeqStoredParmPolc::~MeqStoredParmPolc()
{}

int MeqStoredParmPolc::initDomain (const MeqDomain& domain, int spidIndex)
{
  // Find the polc(s) for the given domain.
  vector<MeqPolc> polcs = itsTable->getPolcs (getName(), domain);
  // If none found, try to get a default value.
  // If no default found, use a 2nd order polynomial with values 1.
  if (polcs.size() == 0) {
    MeqPolc polc = itsTable->getInitCoeff (getName());
    if (polc.getCoeff().isNull()) {
      Matrix<double> defCoeff(3,3);
      defCoeff = 1;
      polc.setCoeff (defCoeff);
    }
    polc.setDomain (domain);
    polcs.push_back (polc);
  } else if (isSolvable()) {
    AssertMsg (polcs.size() == 1, "Solvable parameter " << getName() <<
	       " has multiple matching domains for time "
	       << domain.startX() << ':' << domain.endX() << " and freq "
	       << domain.startY() << ':' << domain.endY());
    const MeqDomain& polDom = polcs[0].domain();
    AssertMsg (near(domain.startX(), polDom.startX())  &&
	       near(domain.endX(), polDom.endX())  &&
	       near(domain.startY(), polDom.startY())  &&
	       near(domain.endY(), polDom.endY()),
	       "Solvable parameter " << getName() <<
	       " has a partially instead of fully matching entry for time "
		 << domain.startX() << ':' << domain.endX() << " and freq "
		 << domain.startY() << ':' << domain.endY());
  }
  setPolcs (polcs);
  return MeqParmPolc::initDomain (domain, spidIndex);
}

void MeqStoredParmPolc::save()
{
  const vector<MeqPolc>& polcs = getPolcs();
  for (unsigned int i=0; i<polcs.size(); i++) {
    itsTable->putCoeff (getName(), polcs[i].domain(), polcs[i].getCoeff());
  }
  MeqParmPolc::save();
}
