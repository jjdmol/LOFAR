//# MnsMatrixTmp.h: Temporary matrix for Mns
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

#if !defined(MNS_MNSMATRIXTMP_H)
#define MNS_MNSMATRIXTMP_H


//# Includes
#include <MNS/MnsMatrixRep.h>
#include <MNS/MnsMatrix.h>
#include <MNS/MnsMatrixRealSca.h>
#include <MNS/MnsMatrixComplexSca.h>


class MnsMatrixTmp
{
friend class MnsMatrix;

public:
  // Create a scalar MnsMatrixTmp.
  // <group>
  MnsMatrixTmp (double value)
    : itsRep (new MnsMatrixRealSca(value)) { itsRep->link(); }
  MnsMatrixTmp (complex<double> value)
    : itsRep (new MnsMatrixComplexSca(value)) { itsRep->link(); }
  // <group>

  // Create a MnsMatrixTmp from a real one (copy semantics).
  MnsMatrixTmp (const MnsMatrix& that)
    : itsRep (that.rep()->clone()->link()) {}

  // Copy constructor (reference semantics).
  MnsMatrixTmp (const MnsMatrixTmp& that)
    : itsRep (that.itsRep->link()) {}
    
  ~MnsMatrixTmp()
    { MnsMatrixRep::unlink (itsRep); }

  // Assignment (reference semantics).
  MnsMatrixTmp& operator= (const MnsMatrixTmp& other);

  // Clone the matrix (copy semantics).
  MnsMatrixTmp clone() const
    { return MnsMatrixTmp (itsRep->clone()); }

  const double* doubleStorage() const
    { return itsRep->doubleStorage(); }

  double getDouble (int x, int y) const
    { return itsRep->getDouble (x, y); }

  complex<double> getDComplex (int x, int y) const
    { return itsRep->getDComplex (x, y); }

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

  MnsMatrixRep* rep() const
    { return itsRep; }

  MnsMatrixTmp operator+ (const MnsMatrixTmp& right)
    { return itsRep->add (*right.itsRep, True); }
  MnsMatrixTmp operator+ (const MnsMatrix& right)
    { return itsRep->add (*right.rep(), False); }

  MnsMatrixTmp operator- (const MnsMatrixTmp& right)
    { return itsRep->subtract (*right.itsRep, True); }
  MnsMatrixTmp operator- (const MnsMatrix& right)
    { return itsRep->subtract (*right.rep(), False); }

  MnsMatrixTmp operator* (const MnsMatrixTmp& right)
    { return itsRep->multiply (*right.itsRep, True); }
  MnsMatrixTmp operator* (const MnsMatrix& right)
    { return itsRep->multiply (*right.rep(), False); }

  MnsMatrixTmp operator/ (const MnsMatrixTmp& right)
    { return itsRep->divide (*right.itsRep, True); }
  MnsMatrixTmp operator/ (const MnsMatrix& right)
    { return itsRep->divide (*right.rep(), False); }

  MnsMatrixTmp operator-() const;

  MnsMatrixTmp sin() const;
  MnsMatrixTmp cos() const;
  MnsMatrixTmp exp() const;
  MnsMatrixTmp conj() const;
  MnsMatrixTmp min() const;
  MnsMatrixTmp max() const;
  MnsMatrixTmp mean() const;
  MnsMatrixTmp sum() const;


private:
  MnsMatrixTmp (MnsMatrixRep* rep)
    { itsRep = rep->link(); }


  MnsMatrixRep* itsRep;
};


inline ostream& operator<< (ostream& os, const MnsMatrixTmp& vec)
  { vec.show (os); return os; }


#endif
