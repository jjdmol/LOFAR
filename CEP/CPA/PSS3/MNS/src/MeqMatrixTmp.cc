//# MeqMatrixTmp.cc: Matrix for Mns
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


#include <MNS/MeqMatrixTmp.h>


MeqMatrixTmp& MeqMatrixTmp::operator= (const MeqMatrixTmp& that)
{
  if (this != &that) {
    MeqMatrixRep::unlink (itsRep);
    itsRep = that.itsRep;
    if (itsRep != 0) {
      itsRep->link();
    }
  }
  return *this;
}

MeqMatrixTmp MeqMatrixTmp::operator-() const
{
  itsRep->negate();
  return itsRep;
}

MeqMatrixTmp sin (const MeqMatrixTmp& arg)
{
  return arg.itsRep->sin();
}
MeqMatrixTmp cos (const MeqMatrixTmp& arg)
{
  return arg.itsRep->cos();
}
MeqMatrixTmp exp (const MeqMatrixTmp& arg)
{
  return arg.itsRep->exp();
}
MeqMatrixTmp sqr(const MeqMatrixTmp& arg)
{
  return arg.itsRep->sqr();
}
MeqMatrixTmp sqrt(const MeqMatrixTmp& arg)
{
  return arg.itsRep->sqrt();
}
MeqMatrixTmp conj (const MeqMatrixTmp& arg)
{
  return arg.itsRep->conj();
}
MeqMatrixTmp min (const MeqMatrixTmp& arg)
{
  return arg.itsRep->min();
}
MeqMatrixTmp max (const MeqMatrixTmp& arg)
{
  return arg.itsRep->max();
}
MeqMatrixTmp mean (const MeqMatrixTmp& arg)
{
  return arg.itsRep->mean();
}
MeqMatrixTmp sum (const MeqMatrixTmp& arg)
{
  return arg.itsRep->sum();
}
