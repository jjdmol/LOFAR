//# MnsMatrixTmp.cc: Matrix for Mns
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


#include <MNS/MnsMatrixTmp.h>


MnsMatrixTmp& MnsMatrixTmp::operator= (const MnsMatrixTmp& that)
{
  if (this != &that) {
    MnsMatrixRep::unlink (itsRep);
    itsRep = that.itsRep;
    if (itsRep != 0) {
      itsRep->link();
    }
  }
  return *this;
}

MnsMatrixTmp MnsMatrixTmp::operator-() const
{
  itsRep->negate();
  return itsRep;
}

MnsMatrixTmp MnsMatrixTmp::sin() const
{
  itsRep->sin();
  return itsRep;
}
MnsMatrixTmp MnsMatrixTmp::cos() const
{
  itsRep->cos();
  return itsRep;
}
MnsMatrixTmp MnsMatrixTmp::exp() const
{
  itsRep->exp();
  return itsRep;
}
MnsMatrixTmp MnsMatrixTmp::conj() const
{
  itsRep->conj();
  return itsRep;
}
MnsMatrixTmp MnsMatrixTmp::min() const
{
  return itsRep->min();
}
MnsMatrixTmp MnsMatrixTmp::max() const
{
  return itsRep->max();
}
MnsMatrixTmp MnsMatrixTmp::mean() const
{
  return itsRep->mean();
}
MnsMatrixTmp MnsMatrixTmp::sum() const
{
  return itsRep->sum();
}
