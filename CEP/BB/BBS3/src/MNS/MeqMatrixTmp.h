//# MeqMatrixTmp.h: Temporary matrix for Mns
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

#if !defined(MNS_MEQMATRIXTMP_H)
#define MNS_MEQMATRIXTMP_H


//# Includes
#include <BBS3/MNS/MeqMatrixRep.h>
#include <BBS3/MNS/MeqMatrix.h>
#include <BBS3/MNS/MeqMatrixRealSca.h>
#include <BBS3/MNS/MeqMatrixComplexSca.h>


namespace LOFAR {

class MeqMatrixTmp
{
public:
  // Create a scalar MeqMatrixTmp.
  // <group>
  MeqMatrixTmp (double value)
    : itsRep (new MeqMatrixRealSca(value)) { itsRep->link(); }
  MeqMatrixTmp (complex<double> value)
    : itsRep (new MeqMatrixComplexSca(value)) { itsRep->link(); }
  // <group>

  // Create a MeqMatrixTmp of given size.
  // If the init flag is true, the matrix is initialized to the given value.
  // Otherwise the value only indicates the type of matrix to be created.
  // <group>
  MeqMatrixTmp (double, int nx, int ny, bool init=true);
  MeqMatrixTmp (complex<double>, int nx, int ny, bool init=true);
  // <group>

  // Create a MeqMatrixTmp from a real one (copy semantics).
  MeqMatrixTmp (const MeqMatrix& that)
    : itsRep (that.rep()->clone()->link()) {}

  // Copy constructor (reference semantics).
  MeqMatrixTmp (const MeqMatrixTmp& that)
    : itsRep (that.itsRep->link()) {}
    
  ~MeqMatrixTmp()
    { MeqMatrixRep::unlink (itsRep); }

  // Assignment (reference semantics).
  MeqMatrixTmp& operator= (const MeqMatrixTmp& other);

  // Clone the matrix (copy semantics).
  MeqMatrixTmp clone() const
    { return MeqMatrixTmp (itsRep->clone()); }

  const double* doubleStorage() const
    { return itsRep->doubleStorage(); }

  double* doubleStorage()
    { return (double*)(itsRep->doubleStorage()); }

  const complex<double>* dcomplexStorage() const
    { return (complex<double>*)(itsRep->dcomplexStorage()); }

  complex<double>* dcomplexStorage()
    { return (complex<double>*)(itsRep->dcomplexStorage()); }

  double getDouble (int x, int y) const
    { return itsRep->getDouble (x, y); }

  double getDouble() const
    { return itsRep->getDouble (0, 0); }

  complex<double> getDComplex (int x, int y) const
    { return itsRep->getDComplex (x, y); }

  complex<double> getDComplex() const
    { return itsRep->getDComplex (0, 0); }

  int nx() const
    { return itsRep->nx(); }

  int ny() const
    { return itsRep->ny(); }

  int nelements() const
    { return itsRep->nelements(); }

  int elemLength() const
    { return itsRep->elemLength(); }

  void show (ostream& os) const
    { itsRep->show (os); }

  friend MeqMatrixTmp operator+ (const MeqMatrixTmp& left,
				 const MeqMatrixTmp& right)
    { return left.itsRep->add (*right.itsRep, true); }
  friend MeqMatrixTmp operator+ (const MeqMatrixTmp& left,
				 const MeqMatrix& right)
    { return left.itsRep->add (*right.rep(), false); }

  friend MeqMatrixTmp operator- (const MeqMatrixTmp& left,
				 const MeqMatrixTmp& right)
    { return left.itsRep->subtract (*right.itsRep, true); }
  friend MeqMatrixTmp operator- (const MeqMatrixTmp& left,
				 const MeqMatrix& right)
    { return left.itsRep->subtract (*right.rep(), false); }

  friend MeqMatrixTmp operator* (const MeqMatrixTmp& left,
				 const MeqMatrixTmp& right)
    { return left.itsRep->multiply (*right.itsRep, true); }
  friend MeqMatrixTmp operator* (const MeqMatrixTmp& left,
				 const MeqMatrix& right)
    { return left.itsRep->multiply (*right.rep(), false); }

  friend MeqMatrixTmp operator/ (const MeqMatrixTmp& left,
				 const MeqMatrixTmp& right)
    { return left.itsRep->divide (*right.itsRep, true); }
  friend MeqMatrixTmp operator/ (const MeqMatrixTmp& left,
				 const MeqMatrix& right)
    { return left.itsRep->divide (*right.rep(), false); }

  MeqMatrixTmp operator-() const;

  friend MeqMatrixTmp posdiff (const MeqMatrixTmp&, const MeqMatrix&);
  friend MeqMatrixTmp posdiff (const MeqMatrixTmp&, const MeqMatrixTmp&);
  friend MeqMatrixTmp tocomplex (const MeqMatrixTmp&, const MeqMatrix&);
  friend MeqMatrixTmp tocomplex (const MeqMatrixTmp&, const MeqMatrixTmp&);
  friend MeqMatrixTmp sin (const MeqMatrixTmp&);
  friend MeqMatrixTmp cos (const MeqMatrixTmp&);
  friend MeqMatrixTmp exp (const MeqMatrixTmp&);
  friend MeqMatrixTmp sqr (const MeqMatrixTmp&);
  friend MeqMatrixTmp sqrt(const MeqMatrixTmp&);
  friend MeqMatrixTmp conj(const MeqMatrixTmp&);
  friend MeqMatrixTmp min (const MeqMatrixTmp&);
  friend MeqMatrixTmp max (const MeqMatrixTmp&);
  friend MeqMatrixTmp mean(const MeqMatrixTmp&);
  friend MeqMatrixTmp sum (const MeqMatrixTmp&);


  MeqMatrixRep* rep() const
    { return itsRep; }

  MeqMatrixTmp (MeqMatrixRep* rep)
    { itsRep = rep->link(); }

private:
  MeqMatrixRep* itsRep;
};


inline ostream& operator<< (ostream& os, const MeqMatrixTmp& vec)
  { vec.show (os); return os; }

}

#endif
