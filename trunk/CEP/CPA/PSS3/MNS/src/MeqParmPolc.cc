//# MeqParmPolc.cc: Polynomial coefficients
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

#include <MNS/MeqParmPolc.h>
#include <MNS/MeqRequest.h>
#include <Common/Debug.h>
#include <aips/Arrays/Matrix.h>

MeqParmPolc::MeqParmPolc (const string& name)
: MeqParm       (name),
  itsIsSolvable (false)
{}

MeqParmPolc::~MeqParmPolc()
{}

void MeqParmPolc::setSolvable (bool solvable)
{
  itsIsSolvable = solvable;
}

int MeqParmPolc::initDomain (const MeqDomain&, int spidIndex)
{
  int nr = 0;
  if (itsIsSolvable) {
    for (unsigned int i=0; i<itsPolcs.size(); i++) {
      int nrs = itsPolcs[i].makeSolvable (spidIndex);
      nr += nrs;
      spidIndex += nrs;
    }
  }
  return nr;
}

MeqResult MeqParmPolc::getResult (const MeqRequest& request)
{
  Assert (itsPolcs.size() == 1);
  return itsPolcs[0].getResult (request);
}

void MeqParmPolc::update (const MeqMatrix& value)
{
  for (unsigned int i=0; i<itsPolcs.size(); i++) {
    itsPolcs[i].update (value);
  }
}

void MeqParmPolc::save()
{}
