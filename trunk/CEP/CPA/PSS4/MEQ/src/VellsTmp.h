//# VellsTmp.h: Temporary vells for Mns
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

#ifndef MEQ_VELLSTMP_H
#define MEQ_VELLSTMP_H


//# Includes
#include <MEQ/VellsRep.h>
#include <MEQ/Vells.h>
#include <MEQ/VellsRealSca.h>
#include <MEQ/VellsComplexSca.h>

namespace MEQ {

class VellsTmp
{
public:
  // Create a scalar VellsTmp.
  // <group>
  VellsTmp (double value)
    : itsRep (new VellsRealSca(value)) { itsRep->link(); }
  VellsTmp (complex<double> value)
    : itsRep (new VellsComplexSca(value)) { itsRep->link(); }
  // <group>

  // Create a VellsTmp of given size.
  // If the init flag is true, the vells is initialized to the given value.
  // Otherwise the value only indicates the type of vells to be created.
  // <group>
  VellsTmp (double, int nx, int ny, bool init=true);
  VellsTmp (complex<double>, int nx, int ny, bool init=true);
  // <group>

  // Create a VellsTmp from a real one (copy semantics).
  VellsTmp (const Vells& that)
    : itsRep (that.rep()->clone()->link()) {}

  // Copy constructor (reference semantics).
  VellsTmp (const VellsTmp& that)
    : itsRep (that.itsRep->link()) {}
    
  ~VellsTmp()
    { VellsRep::unlink (itsRep); }

  // Assignment (reference semantics).
  VellsTmp& operator= (const VellsTmp& other);

  // Clone the values (copy semantics).
  VellsTmp clone() const
    { return VellsTmp (itsRep->clone()); }

  bool isReal() const
    { return itsRep->isReal(); }

  const double* realStorage() const
    { return itsRep->realStorage(); }

  double* realStorage()
    { return itsRep->realStorage(); }

  const complex<double>* complexStorage() const
    { return (complex<double>*)(itsRep->complexStorage()); }

  complex<double>* complexStorage()
    { return itsRep->complexStorage(); }

  int nx() const
    { return itsRep->nx(); }

  int ny() const
    { return itsRep->ny(); }

  int nelements() const
    { return itsRep->nelements(); }

  void show (std::ostream& os) const
    { itsRep->show (os); }

  friend VellsTmp operator+ (const VellsTmp& left, const VellsTmp& right)
    { return left.itsRep->add (*right.itsRep, true); }
  friend VellsTmp operator+ (const VellsTmp& left, const Vells& right)
    { return left.itsRep->add (*right.rep(), false); }

  friend VellsTmp operator- (const VellsTmp& left, const VellsTmp& right)
    { return left.itsRep->subtract (*right.itsRep, true); }
  friend VellsTmp operator- (const VellsTmp& left, const Vells& right)
    { return left.itsRep->subtract (*right.rep(), false); }

  friend VellsTmp operator* (const VellsTmp& left, const VellsTmp& right)
    { return left.itsRep->multiply (*right.itsRep, true); }
  friend VellsTmp operator* (const VellsTmp& left, const Vells& right)
    { return left.itsRep->multiply (*right.rep(), false); }

  friend VellsTmp operator/ (const VellsTmp& left, const VellsTmp& right)
    { return left.itsRep->divide (*right.itsRep, true); }
  friend VellsTmp operator/ (const VellsTmp& left, const Vells& right)
    { return left.itsRep->divide (*right.rep(), false); }

  VellsTmp operator-() const;

  friend VellsTmp posdiff (const VellsTmp&, const Vells&);
  friend VellsTmp posdiff (const VellsTmp&, const VellsTmp&);
  friend VellsTmp tocomplex (const VellsTmp&, const Vells&);
  friend VellsTmp tocomplex (const VellsTmp&, const VellsTmp&);
  friend VellsTmp pow (const VellsTmp& value, const Vells& exponent)
    { return value.itsRep->pow (*exponent.rep(), true); }
  friend VellsTmp pow (const VellsTmp& value, const VellsTmp& exponent)
    { return value.itsRep->pow (*exponent.itsRep, false); }
  friend VellsTmp sin (const VellsTmp&);
  friend VellsTmp cos (const VellsTmp&);
  friend VellsTmp exp (const VellsTmp&);
  friend VellsTmp sqr (const VellsTmp&);
  friend VellsTmp sqrt(const VellsTmp&);
  friend VellsTmp conj(const VellsTmp&);
  friend VellsTmp min (const VellsTmp&);
  friend VellsTmp max (const VellsTmp&);
  friend VellsTmp mean(const VellsTmp&);
  friend VellsTmp sum (const VellsTmp&);


  VellsRep* rep() const
    { return itsRep; }

  VellsTmp (VellsRep* rep)
    { itsRep = rep->link(); }

private:
  VellsRep* itsRep;
};


inline std::ostream& operator<< (std::ostream& os, const VellsTmp& vec)
  { vec.show (os); return os; }


} // namespace MEQ

#endif
