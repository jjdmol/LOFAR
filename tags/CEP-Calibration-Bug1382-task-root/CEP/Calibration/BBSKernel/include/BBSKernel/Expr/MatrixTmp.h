//# MatrixTmp.h: Temporary matrix for Mns
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

#if !defined(EXPR_MATRIXTMP_H)
#define EXPR_MATRIXTMP_H

// \file
// Temporary matrix for Mns

//# Includes
#include <BBSKernel/Expr/MatrixRep.h>
#include <BBSKernel/Expr/Matrix.h>
#include <BBSKernel/Expr/MatrixRealSca.h>
#include <BBSKernel/Expr/MatrixComplexSca.h>


namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{

class MatrixTmp
{
public:
  // Create a scalar MatrixTmp.
  // <group>
  MatrixTmp (double value)
    : itsRep (new MatrixRealSca(value)) { itsRep->link(); }
  MatrixTmp (dcomplex value)
    : itsRep (new MatrixComplexSca(value)) { itsRep->link(); }
  // <group>

  // Create a MatrixTmp of given size.
  // If the init flag is true, the matrix is initialized to the given value.
  // Otherwise the value only indicates the type of matrix to be created.
  // <group>
  MatrixTmp (double, int nx, int ny, bool init=true);
  MatrixTmp (dcomplex, int nx, int ny, bool init=true);
  // <group>

  // Create a MatrixTmp from a real one (copy semantics).
  MatrixTmp (const Matrix& that)
    : itsRep (that.rep()->clone()->link()) {}

  // Copy constructor (reference semantics).
  MatrixTmp (const MatrixTmp& that)
    : itsRep (that.itsRep->link()) {}

  ~MatrixTmp()
    { MatrixRep::unlink (itsRep); }

  // Assignment (reference semantics).
  MatrixTmp& operator= (const MatrixTmp& other);

  // Clone the matrix (copy semantics).
  MatrixTmp clone() const
    { return MatrixTmp (itsRep->clone()); }

  const double* doubleStorage() const
    { return itsRep->doubleStorage(); }

  double* doubleStorage()
    { return (double*)(itsRep->doubleStorage()); }

#if 0
  const dcomplex* dcomplexStorage() const
    { return (dcomplex*)(itsRep->dcomplexStorage()); }

  dcomplex* dcomplexStorage()
    { return (dcomplex*)(itsRep->dcomplexStorage()); }
#endif

  double getDouble (int x, int y) const
    { return itsRep->getDouble (x, y); }

  double getDouble() const
    { return itsRep->getDouble (0, 0); }

  dcomplex getDComplex (int x, int y) const
    { return itsRep->getDComplex (x, y); }

  dcomplex getDComplex() const
    { return itsRep->getDComplex (0, 0); }

  int nx() const
    { return itsRep->nx(); }

  int ny() const
    { return itsRep->ny(); }

  int nelements() const
    { return itsRep->nelements(); }

  void show (ostream& os) const
    { itsRep->show (os); }

  friend MatrixTmp operator+ (const MatrixTmp& left,
				 const MatrixTmp& right)
    { return left.itsRep->add (*right.itsRep, true); }
  friend MatrixTmp operator+ (const MatrixTmp& left,
				 const Matrix& right)
    { return left.itsRep->add (*right.rep(), false); }

  friend MatrixTmp operator- (const MatrixTmp& left,
				 const MatrixTmp& right)
    { return left.itsRep->subtract (*right.itsRep, true); }
  friend MatrixTmp operator- (const MatrixTmp& left,
				 const Matrix& right)
    { return left.itsRep->subtract (*right.rep(), false); }

  friend MatrixTmp operator* (const MatrixTmp& left,
				 const MatrixTmp& right)
    { return left.itsRep->multiply (*right.itsRep, true); }
  friend MatrixTmp operator* (const MatrixTmp& left,
				 const Matrix& right)
    { return left.itsRep->multiply (*right.rep(), false); }

  friend MatrixTmp operator/ (const MatrixTmp& left,
				 const MatrixTmp& right)
    { return left.itsRep->divide (*right.itsRep, true); }
  friend MatrixTmp operator/ (const MatrixTmp& left,
				 const Matrix& right)
    { return left.itsRep->divide (*right.rep(), false); }

  MatrixTmp operator-() const;

  friend MatrixTmp posdiff (const MatrixTmp&, const Matrix&);
  friend MatrixTmp posdiff (const MatrixTmp&, const MatrixTmp&);
  friend MatrixTmp tocomplex (const MatrixTmp&, const Matrix&);
  friend MatrixTmp tocomplex (const MatrixTmp&, const MatrixTmp&);
  friend MatrixTmp sin (const MatrixTmp&);
  friend MatrixTmp cos (const MatrixTmp&);
  friend MatrixTmp log (const MatrixTmp&);
  friend MatrixTmp exp (const MatrixTmp&);
  friend MatrixTmp sqr (const MatrixTmp&);
  friend MatrixTmp sqrt(const MatrixTmp&);
  friend MatrixTmp conj(const MatrixTmp&);
  friend MatrixTmp min (const MatrixTmp&);
  friend MatrixTmp max (const MatrixTmp&);
  friend MatrixTmp mean(const MatrixTmp&);
  friend MatrixTmp sum (const MatrixTmp&);


  MatrixRep* rep() const
    { return itsRep; }

  MatrixTmp (MatrixRep* rep)
    { itsRep = rep->link(); }

private:
  MatrixRep* itsRep;
};


inline ostream& operator<< (ostream& os, const MatrixTmp& vec)
  { vec.show (os); return os; }

// @}

} // namespace BBS
} // namespace LOFAR

#endif
