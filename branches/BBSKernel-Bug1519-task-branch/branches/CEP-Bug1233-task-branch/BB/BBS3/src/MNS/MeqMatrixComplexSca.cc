//# MeqMatrixComplexSca.cc: Temporary matrix for Mns
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

#undef TIMER

#include <lofar_config.h>
#include <BBS3/MNS/MeqMatrixRealSca.h>
#include <BBS3/MNS/MeqMatrixRealArr.h>
#include <BBS3/MNS/MeqMatrixComplexSca.h>
#include <BBS3/MNS/MeqMatrixComplexArr.h>
#include <Common/LofarLogger.h>

#if defined TIMER
#include <Common/Timer.h>
#endif

namespace LOFAR {

MeqMatrixComplexSca::~MeqMatrixComplexSca()
{}

MeqMatrixRep* MeqMatrixComplexSca::clone() const
{
  return new MeqMatrixComplexSca (itsValue);
}

void MeqMatrixComplexSca::show (ostream& os) const
{
  os << itsValue;
}

MeqMatrixRep* MeqMatrixComplexSca::add (MeqMatrixRep& right, bool rightTmp)
{
  return right.addRep (*this, rightTmp);
}
MeqMatrixRep* MeqMatrixComplexSca::subtract (MeqMatrixRep& right,
					     bool rightTmp)
{
  return right.subRep (*this, rightTmp);
}
MeqMatrixRep* MeqMatrixComplexSca::multiply (MeqMatrixRep& right,
					     bool rightTmp)
{
  return right.mulRep (*this, rightTmp);
}
MeqMatrixRep* MeqMatrixComplexSca::divide (MeqMatrixRep& right, bool rightTmp)
{
  return right.divRep (*this, rightTmp);
}

void MeqMatrixComplexSca::dcomplexStorage(const double *&realPtr, const double *&imagPtr) const
{
  realPtr = (double *) &itsValue;
  imagPtr = realPtr + 1;
}

double MeqMatrixComplexSca::getDouble (int, int) const
{
  ASSERTSTR (imag(itsValue)==0,
	     "MeqMatrix: dcomplex->double conversion not possible");
  return real(itsValue);
}

dcomplex MeqMatrixComplexSca::getDComplex (int, int) const
{
  return itsValue;
}


#define MNSMATRIXCOMPLEXSCA_OP(NAME, OP, OP2) \
MeqMatrixRep* MeqMatrixComplexSca::NAME (MeqMatrixRealSca& left, \
					 bool rightTmp) \
{ \
  MeqMatrixComplexSca* v = this; \
  if (!rightTmp) { \
    v = new MeqMatrixComplexSca (itsValue); \
  } \
  v->itsValue = left.itsValue OP2 v->itsValue; \
  return v; \
} \
MeqMatrixRep* MeqMatrixComplexSca::NAME (MeqMatrixComplexSca& left, \
					 bool) \
{ \
  left.itsValue OP itsValue; \
  return &left; \
}

MNSMATRIXCOMPLEXSCA_OP(addRep,+=,+);
MNSMATRIXCOMPLEXSCA_OP(subRep,-=,-);
MNSMATRIXCOMPLEXSCA_OP(mulRep,*=,*);
MNSMATRIXCOMPLEXSCA_OP(divRep,/=,/);

MeqMatrixRep* MeqMatrixComplexSca::addRep(MeqMatrixRealArr& left, bool)
{
  MeqMatrixComplexArr* v = MeqMatrixComplexArr::allocate (left.nx(), left.ny());
  double re = real(itsValue), im = imag(itsValue);

  for (int i=0; i<left.nelements(); i++) {
    v->itsReal[i] = left.itsValue[i] + re;
    v->itsImag[i] = im;
  }
  return v;
}

MeqMatrixRep* MeqMatrixComplexSca::addRep(MeqMatrixComplexArr& left, bool)
{
#if defined TIMER
  static NSTimer timer("add CS CA", true);
  timer.start();
#endif

  double re = real(itsValue), im = imag(itsValue);

  for (int i=0; i<left.nelements(); i++) {
    left.itsReal[i] += re;
    left.itsImag[i] += im;
  }

#if defined TIMER
  timer.stop();
#endif

  return &left;
}

MeqMatrixRep* MeqMatrixComplexSca::subRep(MeqMatrixRealArr& left, bool)
{
  MeqMatrixComplexArr* v = MeqMatrixComplexArr::allocate (left.nx(), left.ny());
  double re = real(itsValue), im = imag(itsValue);

  for (int i=0; i<left.nelements(); i++) {
    v->itsReal[i] = left.itsValue[i] - re;
    v->itsImag[i] = im;
  }
  return v;
}

MeqMatrixRep* MeqMatrixComplexSca::subRep(MeqMatrixComplexArr& left, bool)
{
#if defined TIMER
  static NSTimer timer("sub CS CA", true);
  timer.start();
#endif

  double re = real(itsValue), im = imag(itsValue);

  for (int i=0; i<left.nelements(); i++) {
    left.itsReal[i] -= re;
    left.itsImag[i] -= im;
  }

#if defined TIMER
  timer.stop();
#endif

  return &left;
}

MeqMatrixRep* MeqMatrixComplexSca::mulRep(MeqMatrixRealArr& left, bool)
{
  MeqMatrixComplexArr* v = MeqMatrixComplexArr::allocate (left.nx(), left.ny());
  double re = real(itsValue), im = imag(itsValue);

  for (int i=0; i<left.nelements(); i++) {
    v->itsReal[i] = left.itsValue[i] * re;
    v->itsImag[i] = left.itsValue[i] * im;
  }
  return v;
}

MeqMatrixRep* MeqMatrixComplexSca::mulRep(MeqMatrixComplexArr& left, bool)
{
#if defined TIMER
  static NSTimer timer("mul CS CA", true);
  timer.start();
#endif

  double re = real(itsValue), im = imag(itsValue);

  for (int i=0; i<left.nelements(); i++) {
    double left_r = left.itsReal[i], left_i = left.itsImag[i];
    left.itsReal[i] = re * left_r - im * left_i;
    left.itsImag[i] = re * left_i + im * left_r;
  }

#if defined TIMER
  timer.stop();
#endif

  return &left;
}

MeqMatrixRep* MeqMatrixComplexSca::divRep(MeqMatrixRealArr& left, bool)
{
  MeqMatrixComplexArr* v = MeqMatrixComplexArr::allocate (left.nx(), left.ny());
  double re = real(itsValue), im = imag(itsValue);
  double tmp = re * re + im * im;

  for (int i=0; i<left.nelements(); i++) {
    double t = left.itsValue[i] / tmp;
    v->itsReal[i] = re * t;
    v->itsImag[i] = - im * t;
  }
  return v;
}

MeqMatrixRep* MeqMatrixComplexSca::divRep(MeqMatrixComplexArr& left, bool)
{
#if defined TIMER
  static NSTimer timer("div CS CA", true);
  timer.start();
#endif

  double re = real(itsValue), im = imag(itsValue);
  double tmp = 1.0 / (re * re + im * im);

  for (int i=0; i<left.nelements(); i++) {
    double left_r = left.itsReal[i], left_i = left.itsImag[i];
    left.itsReal[i] = (left_r * re + left_i * im) * tmp;
    left.itsImag[i] = (left_i * im - left_r * re) * tmp;
  }

#if defined TIMER
  timer.stop();
#endif

  return &left;
}

MeqMatrixRep* MeqMatrixComplexSca::negate()
{
  itsValue = -1. * itsValue;
  return this;
}

MeqMatrixRep* MeqMatrixComplexSca::sin()
{
  itsValue = LOFAR::sin(itsValue);
  return this;
}

MeqMatrixRep* MeqMatrixComplexSca::cos()
{
  itsValue = LOFAR::cos(itsValue);
  return this;
}

MeqMatrixRep* MeqMatrixComplexSca::exp()
{
  itsValue = LOFAR::exp(itsValue);
  return this;
}

MeqMatrixRep* MeqMatrixComplexSca::sqr()
{
  itsValue *= itsValue;
  return this;
}

MeqMatrixRep* MeqMatrixComplexSca::sqrt()
{
  itsValue = LOFAR::sqrt(itsValue);
  return this;
}

MeqMatrixRep* MeqMatrixComplexSca::conj()
{
  itsValue = LOFAR::conj(itsValue);
  return this;
}

MeqMatrixRep* MeqMatrixComplexSca::min()
{
  return this;
}
MeqMatrixRep* MeqMatrixComplexSca::max()
{
  return this;
}
MeqMatrixRep* MeqMatrixComplexSca::mean()
{
  return this;
}
MeqMatrixRep* MeqMatrixComplexSca::sum()
{
  return this;
}

}
