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

MeqStoredParmPolc::MeqStoredParmPolc (const string& name, ParmTable* table)
: MeqParmPolc (name),
  itsTable    (table)
{}

MeqStoredParmPolc::~MeqStoredParmPolc()
{}

int MeqStoredParmPolc::initDomain (const MeqDomain& domain, int spidIndex)
{
  bool matchDomain;
  MeqMatrix values = itsTable->getValues (matchDomain, getName(), domain);
  if (values.isNull()) {
    values = itsTable->getInitValues (getName());
    if (values.isNull()) {
      Matrix<double> mat(3,1);
      mat(0,0) = 1;
      mat(1,0) = 2000;
      mat(2,0) = 10;
      values = mat;
    }
  } else if (isSolvable()) {
    AssertMsg (matchDomain, "Parameter " << getName() <<
	       " can only be solvable if the domain is matching exactly");
  }
  vector<MeqPolc> polc(1);
  polc[0].setDomain (domain);
  polc[0].setCoeff (values);
  setPolcs (polc);
  return MeqParmPolc::initDomain (domain, spidIndex);
}

void MeqStoredParmPolc::save()
{
  const vector<MeqPolc>& polcs = getPolcs();
  for (unsigned int i=0; i<polcs.size(); i++) {
    itsTable->putValues (getName(), polcs[i].getDomain(), polcs[i].getCoeff());
  }
  MeqParmPolc::save();
}
