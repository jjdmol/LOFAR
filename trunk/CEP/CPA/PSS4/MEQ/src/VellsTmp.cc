//# VellsTmp.cc: Vells for Mns
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


#include <MEQ/VellsTmp.h>
#include <MEQ/VellsRealArr.h>
#include <MEQ/VellsComplexArr.h>

namespace Meq {

VellsTmp::VellsTmp (double value, int nx, int ny, bool init)
{
  VellsRealArr* v = new VellsRealArr (nx, ny);
  if (init) {
    v->set (value);
  }
  itsRep = v->link();
}

VellsTmp::VellsTmp (complex<double> value, int nx, int ny, bool init)
{
  VellsComplexArr* v = new VellsComplexArr (nx, ny);
  if (init) {
    v->set (value);
  }
  itsRep = v->link();
}

VellsTmp& VellsTmp::operator= (const VellsTmp& that)
{
  if (this != &that) {
    VellsRep::unlink (itsRep);
    itsRep = that.itsRep;
    if (itsRep != 0) {
      itsRep->link();
    }
  }
  return *this;
}

VellsTmp VellsTmp::operator-() const
{
  itsRep->negate();
  return itsRep;
}

VellsTmp posdiff (const VellsTmp& left, const Vells& right)
{
  return left.itsRep->posdiff(*right.rep());
}
VellsTmp posdiff (const VellsTmp& left, const VellsTmp& right)
{
  return left.itsRep->posdiff(*right.rep());
}
VellsTmp tocomplex (const VellsTmp& left, const Vells& right)
{
  return left.itsRep->tocomplex(*right.rep());
}
VellsTmp tocomplex (const VellsTmp& left, const VellsTmp& right)
{
  return left.itsRep->tocomplex(*right.rep());
}
VellsTmp sin (const VellsTmp& arg)
{
  return arg.itsRep->sin();
}
VellsTmp cos (const VellsTmp& arg)
{
  return arg.itsRep->cos();
}
VellsTmp exp (const VellsTmp& arg)
{
  return arg.itsRep->exp();
}
VellsTmp sqr(const VellsTmp& arg)
{
  return arg.itsRep->sqr();
}
VellsTmp sqrt(const VellsTmp& arg)
{
  return arg.itsRep->sqrt();
}
VellsTmp conj (const VellsTmp& arg)
{
  return arg.itsRep->conj();
}
VellsTmp min (const VellsTmp& arg)
{
  return arg.itsRep->min();
}
VellsTmp max (const VellsTmp& arg)
{
  return arg.itsRep->max();
}
VellsTmp mean (const VellsTmp& arg)
{
  return arg.itsRep->mean();
}
VellsTmp sum (const VellsTmp& arg)
{
  return arg.itsRep->sum();
}

} // namespace Meq
